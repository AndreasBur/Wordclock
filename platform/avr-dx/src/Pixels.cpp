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
 *      \brief      The LED matrix as the firmware core sees it, see Pixels.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"
#include "Pixels.h"
#include "WS2812.h"

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

constexpr uint16_t FrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::getNumberOfColors()};

/* The frame the interrupt reads from while it transmits. Separate from the pixel buffer
   because the transmission outlives the call: the next tick may already be writing new
   pixels while the previous frame is still going out, and sharing one buffer would put
   half of each on the strip. */
byte FrameBuffer[FrameSize]{};

} // namespace

/******************************************************************************************************************************************************
 *  P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  fillFrameBuffer()
******************************************************************************************************************************************************/
/*! \brief          Lays the buffer out the way the strip reads it
 *  \details        Green, red, blue - the WS2812's own order, which is not the order the
 *                  colour is written or spoken in. Spelled out per component rather than
 *                  copied wholesale, so a change to the pixel's internal layout cannot
 *                  silently swap two colours on the wire.
******************************************************************************************************************************************************/
void Pixels::fillFrameBuffer() const
{
    byte* Target = FrameBuffer;

    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        const PixelType Pixel = getOutputPixelFast(Index);

        *Target++ = Pixel.getGreen();
        *Target++ = Pixel.getRed();
        *Target++ = Pixel.getBlue();
    }
} /* fillFrameBuffer */


/******************************************************************************************************************************************************
  transmitFrame()
******************************************************************************************************************************************************/
StdReturnType Pixels::transmitFrame()
{
    /* Not waited for: a frame still on the wire means the previous tick's is barely done,
       and the display is redrawn on the next one anyway. Blocking here would hold up the
       clock for a frame nobody would have seen. */
    if(WS2812::getInstance().isBusy()) { return E_NOT_OK; }

    fillFrameBuffer();

    return WS2812::getInstance().send(FrameBuffer, FrameSize);
} /* transmitFrame */


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Claims the strip's data line
 *  \details        The pin the core passes is not used: the line is driven by the look-up
 *                  table's own output, which is fixed to one of two pads and selected in
 *                  WS2812.h. Taken as a parameter regardless, because that is the seam every
 *                  backend implements.
******************************************************************************************************************************************************/
void Pixels::init(byte sPin)
{
    Pin = sPin;

    WS2812::getInstance().init();
    Initialised = true;
} /* init */


/******************************************************************************************************************************************************
  render()
******************************************************************************************************************************************************/
/*! \brief          Puts the buffer on the strip, if the firmware asked for it
 *  \details        Called once per tick by the application, never by show(). Several modules
 *                  ask for a frame within a single tick, and transmitting from each of them
 *                  would put frames back to back on the wire; the strip needs the line idle
 *                  between them or it latches a frame shifted by a pixel, which reads as a
 *                  wiring fault rather than as a software one.
 *
 *  \return         E_OK if a frame went out or none was due
******************************************************************************************************************************************************/
StdReturnType Pixels::render()
{
    if(!Dirty) { return E_OK; }
    if(!Initialised) { return E_NOT_OK; }

    const StdReturnType Result = transmitFrame();
    /* Cleared even when the transmission failed, so a strip that does not take the frame
       cannot turn every following tick into another failing attempt. */
    Dirty = false;

    return Result;
} /* render */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::getPixel(byte Index, PixelType& Pixel) const
{
    if(!isIndexValid(Index)) { return E_NOT_OK; }

    Pixel = getPixelFast(Index);
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
    if(!isIndexValid(Index)) { return E_NOT_OK; }

    setPixelFast(Index, Pixel);
    return E_OK;
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    if(!isIndexValid(Index)) { return E_NOT_OK; }

    setPixelFast(Index, Red, Green, Blue);
    return E_OK;
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Pixels::setPixelFast(byte Index, PixelType Pixel)
{
    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Pixels::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    PixelBuffer[toRow(Index)][toColumn(Index)].setPixel(Red, Green, Blue);
    Dirty = true;
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixels()
******************************************************************************************************************************************************/
void Pixels::setPixels(PixelType Pixel)
{
    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        setPixelFast(Index, Pixel);
    }
} /* setPixels */


/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
/*! \brief          Sets what the buffer is dimmed by on its way out
 *  \details        The buffer itself is left alone, so the colour a command set can still be
 *                  read back after the light sensor has dimmed the display.
******************************************************************************************************************************************************/
void Pixels::setBrightness(byte sBrightness, bool)
{
    Brightness = sBrightness;
    Dirty = true;
} /* setBrightness */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
