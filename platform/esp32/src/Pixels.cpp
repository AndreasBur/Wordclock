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


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* 0.1 us per tick, which makes every WS2812 duration below a small whole number. */
constexpr uint32_t kRmtResolutionHz{10u * 1000u * 1000u};

/* The datasheet gives T0H 0.4 us and T1H 0.8 us with a tolerance of +-150 ns. The
   ESP-IDF led_strip example uses 0.3/0.9 instead, which also works but sits at the edge
   of the window on one side; centring on the nominal values keeps the margin symmetric,
   and that is what a long strip with a slow edge eats into. */
constexpr uint16_t kBit0HighTicks{4u};
constexpr uint16_t kBit0LowTicks{8u};
constexpr uint16_t kBit1HighTicks{8u};
constexpr uint16_t kBit1LowTicks{4u};

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
constexpr size_t kRmtMemoryBlockSymbols{2u * SOC_RMT_MEM_WORDS_PER_CHANNEL};

/* Only ever one frame in flight - see Render(), which waits for the previous one. A
   deeper queue would let two frames go out back to back and lose the reset gap. */
constexpr size_t kRmtQueueDepth{1u};

/* Long enough that it can only be reached if the peripheral stopped answering: a full
   frame takes 110 * 24 * 1.2 us, so about 3.2 ms. */
constexpr int kTransmitTimeoutMs{50};

constexpr size_t kFrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::GetNumberOfColors()};

/* The frame the peripheral reads from while it transmits. Separate from the firmware's
   pixel buffer because the transmission outlives the call: the next tick may already be
   writing new pixels while the DMA is still fetching the old frame, and sharing one
   buffer would put half of each on the strip. */
byte frame_buffer[kFrameSize]{};

rmt_channel_handle_t rmt_channel{nullptr};
rmt_encoder_handle_t rmt_encoder{nullptr};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
/*! \brief          Claims an RMT channel for the strip's data pin
 *  \details        Failures are reported and left behind rather than thrown: the
 *                  contract's Init() returns nothing, and a clock whose serial console
 *                  still answers is more useful for finding a miswired pin than one
 *                  that reboots. render() then does nothing until a channel exists.
******************************************************************************************************************************************************/
void Pixels::Init(byte pin)
{
    pin_ = pin;

    rmt_tx_channel_config_t channel_config{};
    channel_config.gpio_num = static_cast<gpio_num_t>(pin_);
    channel_config.clk_src = RMT_CLK_SRC_DEFAULT;
    channel_config.resolution_hz = kRmtResolutionHz;
    channel_config.mem_block_symbols = kRmtMemoryBlockSymbols;
    channel_config.trans_queue_depth = kRmtQueueDepth;

    esp_err_t result = rmt_new_tx_channel(&channel_config, &rmt_channel);
    if(result != ESP_OK) {
        Serial.print(F("Pixels: RMT channel failed, error "));
        Serial.println(static_cast<int>(result));
        rmt_channel = nullptr;
        return;
    }

    rmt_bytes_encoder_config_t encoder_config{};
    encoder_config.bit0.level0 = 1u;
    encoder_config.bit0.duration0 = kBit0HighTicks;
    encoder_config.bit0.level1 = 0u;
    encoder_config.bit0.duration1 = kBit0LowTicks;
    encoder_config.bit1.level0 = 1u;
    encoder_config.bit1.duration0 = kBit1HighTicks;
    encoder_config.bit1.level1 = 0u;
    encoder_config.bit1.duration1 = kBit1LowTicks;
    encoder_config.flags.msb_first = 1u;

    result = rmt_new_bytes_encoder(&encoder_config, &rmt_encoder);
    if(result != ESP_OK) {
        Serial.print(F("Pixels: RMT encoder failed, error "));
        Serial.println(static_cast<int>(result));
        rmt_del_channel(rmt_channel);
        rmt_channel = nullptr;
        return;
    }

    result = rmt_enable(rmt_channel);
    if(result != ESP_OK) {
        Serial.print(F("Pixels: RMT enable failed, error "));
        Serial.println(static_cast<int>(result));
        rmt_channel = nullptr;
    }
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
    if(rmt_channel == nullptr) { return kENotOk; }

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
 *  \details        The same call TransmitFrame() waits on, with a timeout of nothing: this is
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
bool Pixels::IsFrameOnTheWire() const
{
    if(rmt_channel == nullptr) { return false; }

    return rmt_tx_wait_all_done(rmt_channel, 0) != ESP_OK;
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
    byte* target = frame_buffer;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
        const PixelType pixel = GetOutputPixel(index);

        *target++ = pixel.GetGreen();
        *target++ = pixel.GetRed();
        *target++ = pixel.GetBlue();
    }
} /* FillFrameBuffer */


/******************************************************************************************************************************************************
  TransmitFrame()
******************************************************************************************************************************************************/
/*! \brief          Hands one frame to the RMT peripheral
 *  \details        Waits for the previous frame first, because the peripheral is still
 *                  reading frame_buffer until it is done.
 *
 *  \return         kEOk if the frame was queued
******************************************************************************************************************************************************/
StdReturnType Pixels::TransmitFrame()
{
    if(rmt_tx_wait_all_done(rmt_channel, kTransmitTimeoutMs) != ESP_OK) { return kENotOk; }

    FillFrameBuffer();

    rmt_transmit_config_t transmit_config{};
    transmit_config.loop_count = 0;

    if(rmt_transmit(rmt_channel, rmt_encoder, frame_buffer, sizeof(frame_buffer), &transmit_config) != ESP_OK) { return kENotOk; }

    return kEOk;
} /* TransmitFrame */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
