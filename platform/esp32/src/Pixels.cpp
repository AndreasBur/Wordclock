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
/* The driver headers first, ahead of the Arduino.h that binds Serial to a macro. */
#include <driver/rmt_tx.h>
#include <esp_err.h>
/* For SOC_RMT_MEM_WORDS_PER_CHANNEL. rmt_tx.h already pulls it in, but the memory block
   below is sized from it, and a transitive include is not a promise. */
#include <soc/soc_caps.h>

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

/* 0.1 us per tick, which makes every WS2812 duration below a small whole number. */
constexpr uint32_t RmtResolutionHz{10u * 1000u * 1000u};

/* The datasheet gives T0H 0.4 us and T1H 0.8 us with a tolerance of +-150 ns. The
   ESP-IDF led_strip example uses 0.3/0.9 instead, which also works but sits at the edge
   of the window on one side; centring on the nominal values keeps the margin symmetric,
   and that is what a long strip with a slow edge eats into. */
constexpr uint16_t Bit0HighTicks{4u};
constexpr uint16_t Bit0LowTicks{8u};
constexpr uint16_t Bit1HighTicks{8u};
constexpr uint16_t Bit1LowTicks{4u};

/* Two blocks, and deliberately rather than by rounding: the frame is streamed through this
   memory and not held in it, so without DMA its depth is the deadline the driver's refill
   interrupt has to meet. 110 LEDs are 2640 symbols and one block holds 48 of them, about
   58 us of output at 1.2 us a bit - halving that to save a channel would be trading the
   margin that keeps a frame whole on a part that is also running WiFi.

   Derived rather than written as a number because a block is not the same size everywhere:
   48 words on this part, 64 on a classic ESP32. The 64 that stood here was one block on the
   chip that number came from and two on the S3, which is the right amount for the wrong
   stated reason. On a part with only two transmit channels - C3, C6 - this is both of them,
   which is affordable exactly as long as nothing else wants RMT. */
constexpr size_t RmtMemoryBlockSymbols{2u * SOC_RMT_MEM_WORDS_PER_CHANNEL};

/* Only ever one frame in flight - see render(), which waits for the previous one. A
   deeper queue would let two frames go out back to back and lose the reset gap. */
constexpr size_t RmtQueueDepth{1u};

/* Long enough that it can only be reached if the peripheral stopped answering: a full
   frame takes 110 * 24 * 1.2 us, so about 3.2 ms. */
constexpr int TransmitTimeoutMs{50};

constexpr size_t FrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::getNumberOfColors()};

/* The frame the peripheral reads from while it transmits. Separate from the firmware's
   pixel buffer because the transmission outlives the call: the next tick may already be
   writing new pixels while the DMA is still fetching the old frame, and sharing one
   buffer would put half of each on the strip. */
byte FrameBuffer[FrameSize]{};

rmt_channel_handle_t RmtChannel{nullptr};
rmt_encoder_handle_t RmtEncoder{nullptr};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Claims an RMT channel for the strip's data pin
 *  \details        Failures are reported and left behind rather than thrown: the
 *                  contract's init() returns nothing, and a clock whose serial console
 *                  still answers is more useful for finding a miswired pin than one
 *                  that reboots. render() then does nothing until a channel exists.
******************************************************************************************************************************************************/
void Pixels::init(byte sPin)
{
    Pin = sPin;

    rmt_tx_channel_config_t ChannelConfig{};
    ChannelConfig.gpio_num = static_cast<gpio_num_t>(Pin);
    ChannelConfig.clk_src = RMT_CLK_SRC_DEFAULT;
    ChannelConfig.resolution_hz = RmtResolutionHz;
    ChannelConfig.mem_block_symbols = RmtMemoryBlockSymbols;
    ChannelConfig.trans_queue_depth = RmtQueueDepth;

    esp_err_t Result = rmt_new_tx_channel(&ChannelConfig, &RmtChannel);
    if(Result != ESP_OK) {
        Serial.print(F("Pixels: RMT channel failed, error "));
        Serial.println(static_cast<int>(Result));
        RmtChannel = nullptr;
        return;
    }

    rmt_bytes_encoder_config_t EncoderConfig{};
    EncoderConfig.bit0.level0 = 1u;
    EncoderConfig.bit0.duration0 = Bit0HighTicks;
    EncoderConfig.bit0.level1 = 0u;
    EncoderConfig.bit0.duration1 = Bit0LowTicks;
    EncoderConfig.bit1.level0 = 1u;
    EncoderConfig.bit1.duration0 = Bit1HighTicks;
    EncoderConfig.bit1.level1 = 0u;
    EncoderConfig.bit1.duration1 = Bit1LowTicks;
    EncoderConfig.flags.msb_first = 1u;

    Result = rmt_new_bytes_encoder(&EncoderConfig, &RmtEncoder);
    if(Result != ESP_OK) {
        Serial.print(F("Pixels: RMT encoder failed, error "));
        Serial.println(static_cast<int>(Result));
        rmt_del_channel(RmtChannel);
        RmtChannel = nullptr;
        return;
    }

    Result = rmt_enable(RmtChannel);
    if(Result != ESP_OK) {
        Serial.print(F("Pixels: RMT enable failed, error "));
        Serial.println(static_cast<int>(Result));
        RmtChannel = nullptr;
    }
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
    /* Before the dirty check, and the buffer is left marked on purpose: while the strip has
       no supply nothing may reach DIN, and the supply coming back has to find a frame still
       owed rather than a buffer that looks up to date. */
    if(OutputSuspended) { return E_OK; }

    if(!Dirty) { return E_OK; }
    if(RmtChannel == nullptr) { return E_NOT_OK; }

    const StdReturnType Result = transmitFrame();
    /* Cleared even when the transmission failed, so a strip that does not answer cannot
       turn every following tick into another failing attempt. */
    Dirty = false;
    return Result;
} /* render */


/******************************************************************************************************************************************************
  isFrameOnTheWire()
******************************************************************************************************************************************************/
/*! \brief          Whether the previous frame is still being transmitted
 *  \details        The same call transmitFrame() waits on, with a timeout of nothing: this is
 *                  a question and not a wait, and it is asked from a task that has a clock to
 *                  keep. ESP_ERR_TIMEOUT is therefore the ordinary "still busy" answer rather
 *                  than a fault.
 *
 *                  A channel that was never opened has nothing going out, which is not the
 *                  same as what the driver would answer for a null handle - and reporting a
 *                  frame that cannot exist would stall the switch-off for good.
 *
 *  \return         true while a frame is still going out
******************************************************************************************************************************************************/
bool Pixels::isFrameOnTheWire() const
{
    if(RmtChannel == nullptr) { return false; }

    return rmt_tx_wait_all_done(RmtChannel, 0) != ESP_OK;
} /* isFrameOnTheWire */


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
/*! \brief          Hands one frame to the RMT peripheral
 *  \details        Waits for the previous frame first, because the peripheral is still
 *                  reading FrameBuffer until it is done.
 *
 *  \return         E_OK if the frame was queued
******************************************************************************************************************************************************/
StdReturnType Pixels::transmitFrame()
{
    if(rmt_tx_wait_all_done(RmtChannel, TransmitTimeoutMs) != ESP_OK) { return E_NOT_OK; }

    fillFrameBuffer();

    rmt_transmit_config_t TransmitConfig{};
    TransmitConfig.loop_count = 0;

    if(rmt_transmit(RmtChannel, RmtEncoder, FrameBuffer, sizeof(FrameBuffer), &TransmitConfig) != ESP_OK) { return E_NOT_OK; }

    return E_OK;
} /* transmitFrame */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
