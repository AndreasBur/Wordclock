// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief RP2350 WS2812 driver: the buffer, turned into frames for WS2812Pio's PIO state machine.
 *
 * A PIO state machine generates the WS2812 pulses in hardware and DMA feeds it the frame, so
 * nothing here has to hit a timing with instruction cycles - which is what the xmega driver spent
 * most of its 800 lines on, and what only an oscilloscope can debug.
 */

#include "Pixels.h"

#include "Arduino.h"
#include "WS2812Pio.h"

namespace {
/*
 * One word per LED rather than one byte per colour, because that is the unit the state
 * machine pulls: the three bytes sit in bits 31..8 and are shifted out from the top. The
 * eight low bits are never reached - autopull refills after 24 - so what stands in them
 * does not matter.
 */
constexpr size_t kFrameSize{PIXELS_NUMBER_OF_LEDS};
constexpr uint32_t kGreenShift{24u};
constexpr uint32_t kRedShift{16u};
constexpr uint32_t kBlueShift{8u};

/*
 * The frame DMA reads from while it transmits. Separate from the firmware's pixel buffer
 * because the transmission outlives the call: the next tick may already be writing new
 * pixels while DMA is still fetching the old frame, and sharing one buffer would put half
 * of each on the strip.
 */
WS2812Pio::WordType frame_buffer[kFrameSize]{};

}  // namespace

void Pixels::Init(byte pin) {
  pin_ = pin;

  WS2812Pio::GetInstance().Init(pin_);
}

StdReturnType Pixels::Render() {
  /*
   * Before the dirty check, and the buffer is left marked on purpose: while the strip has
   * no supply nothing may reach DIN, and the supply coming back has to find a frame still
   * owed rather than a buffer that looks up to date.
   */
  if (output_suspended_) {
    return kEOk;
  }

  if (!dirty_) {
    return kEOk;
  }
  if (!WS2812Pio::GetInstance().IsReady()) {
    return kENotOk;
  }

  StdReturnType const result = TransmitFrame();
  // Cleared even when the transmission failed, so a strip that does not answer cannot
  // turn every following tick into another failing attempt.
  dirty_ = false;
  return result;
}

/*
 * An instance method, though it reads no member: it is part of the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
bool Pixels::IsFrameOnTheWire() const { return WS2812Pio::GetInstance().IsBusy(); }
// NOLINTEND(readability-convert-member-functions-to-static)

StdReturnType Pixels::GetPixel(byte index, PixelType& pixel) const {
  if (!IsIndexValid(index)) {
    return kENotOk;
  }

  pixel = pixel_buffer_[ToRow(index)][ToColumn(index)];
  return kEOk;
}

Pixels::PixelType Pixels::GetPixel(byte index) const {
  // An index that is not on the strip has no pixel, and an unlit one is the answer no
  // caller can mistake for an LED that is on.
  if (!IsIndexValid(index)) {
    return PixelType{};
  }

  return pixel_buffer_[ToRow(index)][ToColumn(index)];
}

StdReturnType Pixels::SetPixel(byte index, PixelType pixel) {
  if (!IsIndexValid(index)) {
    return kENotOk;
  }

  pixel_buffer_[ToRow(index)][ToColumn(index)] = pixel;
  dirty_ = true;
  return kEOk;
}

StdReturnType Pixels::SetPixel(byte index, byte red, byte green, byte blue) {
  return SetPixel(index, PixelType(red, green, blue));
}

void Pixels::SetPixels(PixelType pixel) {
  for (auto& row : pixel_buffer_) {
    for (PixelType& slot : row) {
      slot = pixel;
    }
  }
  dirty_ = true;
}

void Pixels::SetBrightness(byte brightness, bool gamma_correction) {
  /*
   * Master output on and off, driven by EnablePixels() and DisablePixels(). The buffer
   * is left alone and the level is applied on the way out, so switching back on
   * restores the picture without the firmware having to redraw it.
   */
  brightness_ = brightness;
  dirty_ = true;

  UNUSED(gamma_correction);
}

/*
 * Copies the pixel buffer into the frame the peripheral reads. Pixel already stores its
 * colours in the WS2812 order (green, red, blue - see the offsets in Pixel.h), so this is a
 * scale and not a reordering. The accessors are used anyway, so that changing those offsets
 * cannot silently swap the colours on the strip.
 */
void Pixels::FillFrameBuffer() const {
  for (byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
    PixelType const pixel = GetOutputPixel(index);

    /*
     * Left-aligned into the word rather than packed at the bottom: the state machine
     * shifts out from bit 31, so green has to start there for the strip to receive
     * the three bytes in its own order.
     */
    frame_buffer[index] = (static_cast<WS2812Pio::WordType>(pixel.GetGreen()) << kGreenShift) |
                          (static_cast<WS2812Pio::WordType>(pixel.GetRed()) << kRedShift) |
                          (static_cast<WS2812Pio::WordType>(pixel.GetBlue()) << kBlueShift);
  }
}

/*
 * Hands one frame to the PIO driver; kEOk if the frame was handed over. Refuses while the
 * previous frame is still going out, because DMA is still reading frame_buffer until it is
 * done. Where the ESP32 backend waits for its peripheral with a timeout, this one returns: a
 * frame takes 3.3 ms against a 10 ms tick, so finding DMA still busy means something is wrong
 * rather than that the strip needs a moment.
 */
StdReturnType Pixels::TransmitFrame() {
  WS2812Pio& driver = WS2812Pio::GetInstance();

  if (driver.IsBusy()) {
    return kENotOk;
  }

  FillFrameBuffer();

  return driver.Transmit(frame_buffer, kFrameSize);
}
