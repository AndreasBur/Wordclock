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
constexpr size_t kFrameSize{PIXELS_NUMBER_OF_LEDS};
constexpr uint32_t kGreenShift{24u};
constexpr uint32_t kRedShift{16u};
constexpr uint32_t kBlueShift{8u};

/* The frame DMA reads from while it transmits. Separate from the firmware's pixel buffer
   because the transmission outlives the call: the next tick may already be writing new
   pixels while DMA is still fetching the old frame, and sharing one buffer would put half
   of each on the strip. */
WS2812Pio::WordType frame_buffer[kFrameSize]{};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
/*! \brief          Hands the strip's data pin to the PIO driver
 *  \details        Failures are reported and left behind rather than thrown: the
 *                  contract's Init() returns nothing, and a clock whose serial console
 *                  still answers is more useful for finding a miswired pin than one
 *                  that reboots. render() then does nothing until the driver is ready.
******************************************************************************************************************************************************/
void Pixels::Init(byte pin)
{
    pin_ = pin;

    WS2812Pio::GetInstance().Init(pin_);
} /* Init */


/******************************************************************************************************************************************************
  Render()
******************************************************************************************************************************************************/
/*! \brief          Puts the buffer on the strip, if the firmware asked for it
 *  \details        The application calls this once per tick, after the firmware's pass -
 *                  never Show() itself. Two reasons, and the second is the one that
 *                  bites: several modules call Display::Show() within a single tick
 *                  (DisplayManager, Animations, Text, Clock and the command parsers all
 *                  do), so transmitting from Show() would put frames back to back on the
 *                  wire. WS2812 needs the line low for longer than 280 us between
 *                  frames, and losing that gap does not blank the display - it shifts
 *                  the next frame by a pixel, which reads as a wiring fault.
 *                  Rendering once per 10 ms tick leaves milliseconds of idle line.
 *
 *  \return         kEOk if a frame went out or none was due
******************************************************************************************************************************************************/
StdReturnType Pixels::Render()
{
    /* Before the dirty check, and the buffer is left marked on purpose: while the strip has
       no supply nothing may reach DIN, and the supply coming back has to find a frame still
       owed rather than a buffer that looks up to date. */
    if(output_suspended_) { return kEOk; }

    if(!dirty_) { return kEOk; }
    if(!WS2812Pio::GetInstance().IsReady()) { return kENotOk; }

    const StdReturnType result = TransmitFrame();
    /* Cleared even when the transmission failed, so a strip that does not answer cannot
       turn every following tick into another failing attempt. */
    dirty_ = false;
    return result;
} /* Render */


/* An instance method, though it reads no member: it is part of the interface the
   firmware core calls through GetInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  IsFrameOnTheWire()
******************************************************************************************************************************************************/
/*! \brief          Whether the previous frame is still being transmitted
 *  \details        The driver's own answer rather than a flag kept here, and the same one
 *                  TransmitFrame() refuses on.
 *
 *  \return         true while a frame is still going out
******************************************************************************************************************************************************/
bool Pixels::IsFrameOnTheWire() const
{
    return WS2812Pio::GetInstance().IsBusy();
} /* IsFrameOnTheWire */
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
  GetPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::GetPixel(byte index, PixelType& pixel) const
{
    if(!IsIndexValid(index)) { return kENotOk; }

    pixel = pixel_buffer_[ToRow(index)][ToColumn(index)];
    return kEOk;
} /* GetPixel */


/******************************************************************************************************************************************************
  GetPixel()
******************************************************************************************************************************************************/
Pixels::PixelType Pixels::GetPixel(byte index) const
{
    /* An index that is not on the strip has no pixel, and an unlit one is the answer no
       caller can mistake for an LED that is on. */
    if(!IsIndexValid(index)) { return PixelType{}; }

    return pixel_buffer_[ToRow(index)][ToColumn(index)];
} /* GetPixel */


/******************************************************************************************************************************************************
  SetPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::SetPixel(byte index, PixelType pixel)
{
    if(!IsIndexValid(index)) { return kENotOk; }

    pixel_buffer_[ToRow(index)][ToColumn(index)] = pixel;
    dirty_ = true;
    return kEOk;
} /* SetPixel */


/******************************************************************************************************************************************************
  SetPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::SetPixel(byte index, byte red, byte green, byte blue)
{
    return SetPixel(index, PixelType(red, green, blue));
} /* SetPixel */


/******************************************************************************************************************************************************
  SetPixels()
******************************************************************************************************************************************************/
void Pixels::SetPixels(PixelType pixel)
{
    for(unsigned int row = 0; row < PIXELS_DISPLAY_NUMBER_OF_ROWS; row++) {
        for(unsigned int column = 0; column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; column++) {
            pixel_buffer_[row][column] = pixel;
        }
    }
    dirty_ = true;
} /* SetPixels */


/******************************************************************************************************************************************************
  SetBrightness()
******************************************************************************************************************************************************/
void Pixels::SetBrightness(byte brightness, bool gamma_correction)
{
    /* Master output on and off, driven by EnablePixels() and DisablePixels(). The buffer
       is left alone and the level is applied on the way out, so switching back on
       restores the picture without the firmware having to redraw it. */
    brightness_ = brightness;
    dirty_ = true;

    UNUSED(gamma_correction);
} /* SetBrightness */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  FillFrameBuffer()
******************************************************************************************************************************************************/
/*! \brief          Copies the pixel buffer into the frame the peripheral reads
 *  \details        Pixel already stores its colours in the WS2812 order (green, red,
 *                  blue - see the offsets in Pixel.h), so this is a scale and not a
 *                  reordering. The accessors are used anyway, so that changing those
 *                  offsets cannot silently swap the colours on the strip.
******************************************************************************************************************************************************/
void Pixels::FillFrameBuffer() const
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
        const PixelType pixel = GetOutputPixel(index);

        /* Left-aligned into the word rather than packed at the bottom: the state machine
           shifts out from bit 31, so green has to start there for the strip to receive
           the three bytes in its own order. */
        frame_buffer[index] = (static_cast<WS2812Pio::WordType>(pixel.GetGreen()) << kGreenShift)
                           | (static_cast<WS2812Pio::WordType>(pixel.GetRed())   << kRedShift)
                           | (static_cast<WS2812Pio::WordType>(pixel.GetBlue())  << kBlueShift);
    }
} /* FillFrameBuffer */


/******************************************************************************************************************************************************
  TransmitFrame()
******************************************************************************************************************************************************/
/*! \brief          Hands one frame to the PIO driver
 *  \details        Refuses while the previous frame is still going out, because DMA is
 *                  still reading frame_buffer until it is done. Where the ESP32 backend
 *                  waits for its peripheral with a timeout, this one returns: a frame
 *                  takes 3.3 ms against a 10 ms tick, so finding DMA still busy means
 *                  something is wrong rather than that the strip needs a moment.
 *
 *  \return         kEOk if the frame was handed over
******************************************************************************************************************************************************/
StdReturnType Pixels::TransmitFrame()
{
    WS2812Pio& driver = WS2812Pio::GetInstance();

    if(driver.IsBusy()) { return kENotOk; }

    FillFrameBuffer();

    return driver.Transmit(frame_buffer, kFrameSize);
} /* TransmitFrame */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
