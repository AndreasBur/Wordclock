/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       Pixels.cpp
 *      \brief      ESP32 WS2812 driver, driven by the RMT peripheral
 *
 *      \details    The RMT peripheral generates the WS2812 pulses in hardware and
 *                  fetches the bytes by DMA, so nothing here has to hit a timing with
 *                  instruction cycles - which is what the xmega driver spent most of
 *                  its 800 lines on, and what only an oscilloscope can debug.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "WS2812Pio.h"

#include "Arduino.h"
#include "Pixels.h"

#include <cassert>

/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* One word per LED rather than one byte per colour, because that is the unit the state
   machine pulls: the three bytes sit in bits 31..8 and are shifted out from the top. The
   eight low bits are never reached - autopull refills after 24 - so what stands in them
   does not matter. */
constexpr size_t FrameSize{PIXELS_NUMBER_OF_LEDS};
constexpr uint32_t GreenShift{24u};
constexpr uint32_t RedShift{16u};
constexpr uint32_t BlueShift{8u};

/* The frame DMA reads from while it transmits. Separate from the firmware's pixel buffer
   because the transmission outlives the call: the next tick may already be writing new
   pixels while DMA is still fetching the old frame, and sharing one buffer would put half
   of each on the strip. */
WS2812Pio::WordType FrameBuffer[FrameSize]{};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Hands the strip's data pin to the PIO driver
 *  \details        Failures are reported and left behind rather than thrown: the
 *                  contract's init() returns nothing, and a clock whose serial console
 *                  still answers is more useful for finding a miswired pin than one
 *                  that reboots. render() then does nothing until the driver is ready.
******************************************************************************************************************************************************/
void Pixels::init(byte sPin)
{
    Pin = sPin;

    WS2812Pio::getInstance().init(Pin);
} /* init */


/******************************************************************************************************************************************************
  render()
******************************************************************************************************************************************************/
/*! \brief          Puts the buffer on the strip, if the firmware asked for it
 *  \details        The application calls this once per tick, after the firmware's pass -
 *                  never show() itself. Two reasons, and the second is the one that
 *                  bites: several modules call Display::show() within a single tick
 *                  (DisplayManager, Animations, Text, Clock and the command parsers all
 *                  do), so transmitting from show() would put frames back to back on the
 *                  wire. WS2812 needs the line low for longer than 280 us between
 *                  frames, and losing that gap does not blank the display - it shifts
 *                  the next frame by a pixel, which reads as a wiring fault.
 *                  Rendering once per 10 ms tick leaves milliseconds of idle line.
 *
 *  \return         E_OK if a frame went out or none was due
******************************************************************************************************************************************************/
StdReturnType Pixels::render()
{
    if(!Dirty) { return E_OK; }
    if(!WS2812Pio::getInstance().isReady()) { return E_NOT_OK; }

    const StdReturnType Result = transmitFrame();
    /* Cleared even when the transmission failed, so a strip that does not answer cannot
       turn every following tick into another failing attempt. */
    Dirty = false;
    return Result;
} /* render */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::getPixel(byte Index, PixelType& Pixel) const
{
    if(Index >= PIXELS_NUMBER_OF_LEDS) { return E_NOT_OK; }

    Pixel = PixelBuffer[toRow(Index)][toColumn(Index)];
    return E_OK;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
Pixels::PixelType Pixels::getPixelFast(byte Index) const
{
    return PixelBuffer[toRow(Index)][toColumn(Index)];
} /* getPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::setPixel(byte Index, PixelType Pixel)
{
    if(Index >= PIXELS_NUMBER_OF_LEDS) { return E_NOT_OK; }

    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
    return E_OK;
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    return setPixel(Index, PixelType(Red, Green, Blue));
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
/* assert rather than a silent clamp, matching the simulator: an index past the end is a
   firmware bug, and this is the build where it can still be caught. */
void Pixels::setPixelFast(byte Index, PixelType Pixel)
{
    assert(Index < PIXELS_NUMBER_OF_LEDS);

    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Pixels::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    setPixelFast(Index, PixelType(Red, Green, Blue));
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixels()
******************************************************************************************************************************************************/
void Pixels::setPixels(PixelType Pixel)
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            PixelBuffer[Row][Column] = Pixel;
        }
    }
    Dirty = true;
} /* setPixels */


/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
void Pixels::setBrightness(byte sBrightness, bool GammaCorrection)
{
    /* Master output on and off, driven by enablePixels() and disablePixels(). The buffer
       is left alone and the level is applied on the way out, so switching back on
       restores the picture without the firmware having to redraw it. */
    Brightness = sBrightness;
    Dirty = true;

    UNUSED(GammaCorrection);
} /* setBrightness */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  fillFrameBuffer()
******************************************************************************************************************************************************/
/*! \brief          Copies the pixel buffer into the frame the peripheral reads
 *  \details        Pixel already stores its colours in the WS2812 order (green, red,
 *                  blue - see the offsets in Pixel.h), so this is a scale and not a
 *                  reordering. The accessors are used anyway, so that changing those
 *                  offsets cannot silently swap the colours on the strip.
******************************************************************************************************************************************************/
void Pixels::fillFrameBuffer() const
{
    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        const PixelType Pixel = getOutputPixelFast(Index);

        /* Left-aligned into the word rather than packed at the bottom: the state machine
           shifts out from bit 31, so green has to start there for the strip to receive
           the three bytes in its own order. */
        FrameBuffer[Index] = (static_cast<WS2812Pio::WordType>(Pixel.getGreen()) << GreenShift)
                           | (static_cast<WS2812Pio::WordType>(Pixel.getRed())   << RedShift)
                           | (static_cast<WS2812Pio::WordType>(Pixel.getBlue())  << BlueShift);
    }
} /* fillFrameBuffer */


/******************************************************************************************************************************************************
  transmitFrame()
******************************************************************************************************************************************************/
/*! \brief          Hands one frame to the PIO driver
 *  \details        Refuses while the previous frame is still going out, because DMA is
 *                  still reading FrameBuffer until it is done. Where the ESP32 backend
 *                  waits for its peripheral with a timeout, this one returns: a frame
 *                  takes 3.3 ms against a 10 ms tick, so finding DMA still busy means
 *                  something is wrong rather than that the strip needs a moment.
 *
 *  \return         E_OK if the frame was handed over
******************************************************************************************************************************************************/
StdReturnType Pixels::transmitFrame()
{
    WS2812Pio& Driver = WS2812Pio::getInstance();

    if(Driver.isBusy()) { return E_NOT_OK; }

    fillFrameBuffer();

    return Driver.transmit(FrameBuffer, FrameSize);
} /* transmitFrame */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
