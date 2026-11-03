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

constexpr uint16_t kFrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::GetNumberOfColors()};

/* The frame the interrupt reads from while it transmits. Separate from the pixel buffer
   because the transmission outlives the call: the next tick may already be writing new
   pixels while the previous frame is still going out, and sharing one buffer would put
   half of each on the strip. */
byte frame_buffer[kFrameSize]{};

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
void Pixels::FillFrameBuffer() const
{
    byte* target = frame_buffer;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
        const PixelType pixel = GetOutputPixel(index);

        *target++ = pixel.GetGreen();
        *target++ = pixel.GetRed();
        *target++ = pixel.GetBlue();
    }
} /* fillFrameBuffer */


/******************************************************************************************************************************************************
  transmitFrame()
******************************************************************************************************************************************************/
StdReturnType Pixels::TransmitFrame()
{
    /* Not waited for: a frame still on the wire means the previous tick's is barely done,
       and the display is redrawn on the next one anyway. Blocking here would hold up the
       clock for a frame nobody would have seen. */
    if(WS2812::GetInstance().IsBusy()) { return kENotOk; }

    FillFrameBuffer();

    return WS2812::GetInstance().Send(frame_buffer, kFrameSize);
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
void Pixels::Init(byte pin)
{
    pin_ = pin;

    WS2812::GetInstance().Init();
    initialised_ = true;
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
StdReturnType Pixels::Render()
{
    /* Before the dirty check, and the buffer is left marked on purpose: while the strip has
       no supply nothing may reach DIN, and the supply coming back has to find a frame still
       owed rather than a buffer that looks up to date. */
    if(output_suspended_) { return kEOk; }

    if(!dirty_) { return kEOk; }
    if(!initialised_) { return kENotOk; }

    const StdReturnType result = TransmitFrame();
    /* Cleared even when the transmission failed, so a strip that does not take the frame
       cannot turn every following tick into another failing attempt. */
    dirty_ = false;

    return result;
} /* render */


/* An instance method, though it reads no member: it is part of the interface the
   firmware core calls through getInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  isFrameOnTheWire()
******************************************************************************************************************************************************/
/*! \brief          Whether the previous frame is still being transmitted
 *  \details        The driver's own answer rather than a flag kept here: it knows both halves
 *                  of it, the queue and the byte still sitting in the shift register.
 *
 *  \return         true while a frame is still going out
******************************************************************************************************************************************************/
bool Pixels::IsFrameOnTheWire() const
{
    return WS2812::GetInstance().IsBusy();
} /* isFrameOnTheWire */
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::GetPixel(byte index, PixelType& pixel) const
{
    if(!IsIndexValid(index)) { return kENotOk; }

    pixel = GetPixel(index);
    return kEOk;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
Pixels::PixelType Pixels::GetPixel(byte index) const
{
    /* An index that is not on the strip has no pixel, and an unlit one is the answer no
       caller can mistake for an LED that is on. */
    if(!IsIndexValid(index)) { return PixelType{}; }

    return pixel_buffer_[ToRow(index)][ToColumn(index)];
} /* getPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::SetPixel(byte index, PixelType pixel)
{
    if(!IsIndexValid(index)) { return kENotOk; }

    SetPixel(index, pixel);
    return kEOk;
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Pixels::SetPixel(byte index, byte red, byte green, byte blue)
{
    if(!IsIndexValid(index)) { return kENotOk; }

    SetPixel(index, red, green, blue);
    return kEOk;
} /* setPixel */


/******************************************************************************************************************************************************
  setPixels()
******************************************************************************************************************************************************/
void Pixels::SetPixels(PixelType pixel)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
        SetPixel(index, pixel);
    }
} /* setPixels */


/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
/*! \brief          Sets what the buffer is dimmed by on its way out
 *  \details        The buffer itself is left alone, so the colour a command set can still be
 *                  read back after the light sensor has dimmed the display.
******************************************************************************************************************************************************/
void Pixels::SetBrightness(byte brightness, bool)
{
    brightness_ = brightness;
    dirty_ = true;
} /* setBrightness */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
