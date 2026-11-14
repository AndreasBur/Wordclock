// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The pixel buffer of the simulator, in place of the LED strip.
 */

#include "sim/Pixels.h"

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
   * Master output on and off, driven by EnablePixels() and DisablePixels(). The
   * buffer is left alone, so switching back on restores the picture without having to
   * copy it away first.
   */
  brightness_ = brightness;
  dirty_ = true;

  UNUSED(gamma_correction);
}
