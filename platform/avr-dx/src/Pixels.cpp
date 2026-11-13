// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The LED matrix as the firmware core sees it, see Pixels.h.
 */

#include "Arduino.h"
#include "Pixels.h"
#include "WS2812.h"

namespace {
constexpr uint16_t kFrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::GetNumberOfColors()};

/*
 * The frame the interrupt reads from while it transmits. Separate from the pixel buffer
 * because the transmission outlives the call: the next tick may already be writing new
 * pixels while the previous frame is still going out, and sharing one buffer would put
 * half of each on the strip.
 */
byte frame_buffer[kFrameSize]{};

}  // namespace

/*
 * Lays the buffer out the way the strip reads it: green, red, blue - the WS2812's own
 * order, which is not the order the colour is written or spoken in. Spelled out per
 * component rather than copied wholesale, so a change to the pixel's internal layout cannot
 * silently swap two colours on the wire.
 */
void Pixels::FillFrameBuffer() const {
  byte* target = frame_buffer;

  for (byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
    PixelType const pixel = GetOutputPixel(index);

    *target++ = pixel.GetGreen();
    *target++ = pixel.GetRed();
    *target++ = pixel.GetBlue();
  }
}

StdReturnType Pixels::TransmitFrame() {
  FillFrameBuffer();

  return WS2812::GetInstance().Send(frame_buffer, kFrameSize);
}

void Pixels::Init(byte pin) {
  pin_ = pin;

  WS2812::GetInstance().Init();
  initialised_ = true;
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
  if (!initialised_) {
    return kENotOk;
  }
  /*
   * Not waited for, and the mark kept: a frame still on the wire means the previous tick's
   * is barely done, and this one goes out on the next tick. Blocking here would hold up the
   * clock; dropping it, as this did, left the strip on the frame before until something
   * else was drawn - the last step of an animation, say, until the next word change.
   */
  if (WS2812::GetInstance().IsBusy()) {
    return kEOk;
  }

  StdReturnType const result = TransmitFrame();
  // Cleared even when the transmission failed, so a strip that does not take the frame
  // cannot turn every following tick into another failing attempt.
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
bool Pixels::IsFrameOnTheWire() const { return WS2812::GetInstance().IsBusy(); }
// NOLINTEND(readability-convert-member-functions-to-static)

StdReturnType Pixels::GetPixel(byte index, PixelType& pixel) const {
  if (!IsIndexValid(index)) {
    return kENotOk;
  }

  pixel = GetPixel(index);
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
  for (byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
    SetPixel(index, pixel);
  }
}

void Pixels::SetBrightness(byte brightness, bool) {
  brightness_ = brightness;
  dirty_ = true;
}
