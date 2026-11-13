// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the colour cycle, and the hue each of them is.
 */

#include "ColorCycle.h"
#include "Display.h"

namespace {
Display& GetDisplay() { return Display::GetInstance(); }
}  // namespace

/*
 * One hue as a colour, at full saturation and full value - the outside edge of the
 * wheel. Anything inside it is a colour the strip shows as a paler version of white,
 * and a cycle whose hue cannot be seen is not what this is for.
 *
 * Integer throughout, six segments of 256/6 each: within a segment one channel rises
 * while the third stays dark, and which channel does what is the segment number. The
 * multiplication by six before the shift is what keeps that division exact without a
 * remainder to carry, which a hue split into 43-wide segments would have had.
 *
 * Plain static rather than constexpr. It could be one since Pixel's destructor is defaulted,
 * but a constexpr function has to be in the header for its callers to see, and nothing needs
 * this one before the clock runs.
 */
Pixel ColorCycle::ToColour(HueType hue) {
  uint16_t const scaled = static_cast<uint16_t>(hue) * kNumberOfSegments;
  byte const segment = static_cast<byte>(scaled >> 8u);
  byte const rising = static_cast<byte>(scaled & 0xFFu);
  byte const falling = static_cast<byte>(kFullChannel - rising);

  switch (segment) {
    case 0u:
      return Pixel(kFullChannel, rising, 0u);
    case 1u:
      return Pixel(falling, kFullChannel, 0u);
    case 2u:
      return Pixel(0u, kFullChannel, rising);
    case 3u:
      return Pixel(0u, falling, kFullChannel);
    case 4u:
      return Pixel(rising, 0u, kFullChannel);
    default:
      return Pixel(kFullChannel, 0u, falling);
  }
}

/*
 * A display that is off has nothing to cycle, and stepping it anyway would cost more
 * than the arithmetic: every step marks the pixel buffer, and the supply switch waits
 * for a buffer that has settled before it drops the rail. A cycle running through the
 * night would hold that wait open and spend the current the switch exists to save.
 */
bool ColorCycle::IsDisplayShowingAnything() { return GetDisplay().IsEnabled(); }

void ColorCycle::SetIsActive(bool is_active) {
  is_active_ = is_active;

  if (is_active_) {
    GetDisplay().SetColorCycle(ToColour(hue_));
  } else {
    GetDisplay().ClearColorCycle();
  }
}

void ColorCycle::ResetToDefaults() {
  SetTaskCycle(kTaskCycleInitValue);
  hue_ = kHueInitValue;
  SetIsActive(kIsActiveInitValue);
}

void ColorCycle::Task() {
  if (!is_active_) {
    return;
  }
  if (!IsDisplayShowingAnything()) {
    return;
  }

  hue_++;
  GetDisplay().SetColorCycle(ToColour(hue_));
}
