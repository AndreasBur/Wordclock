// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief One pixel: its red, green and blue values, stored in the order the LED strip wants.
 */

#ifndef _PIXEL_H_
#define _PIXEL_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include <array>

/* Pixel configuration parameter */
#define PIXEL_COLOR_OFFSET_GREEN                   0u
#define PIXEL_COLOR_OFFSET_RED                     1u
#define PIXEL_COLOR_OFFSET_BLUE                    2u

/* Pixel parameter */

class Pixel
{
  public:
    using ColorType = byte;

  private:
    static constexpr byte kNumberOfColors{3u};
    using PixelRawType = std::array<byte, kNumberOfColors>;

  private:
    PixelRawType pixel_raw_{0u, 0u, 0u};

    static constexpr byte kColorMaxValue{255u};
    static constexpr byte kColorOffsetRed{PIXEL_COLOR_OFFSET_RED};
    static constexpr byte kColorOffsetGreen{PIXEL_COLOR_OFFSET_GREEN};
    static constexpr byte kColorOffsetBlue{PIXEL_COLOR_OFFSET_BLUE};

    // functions
    static constexpr ColorType DimmColor(ColorType color, byte brightness) {
        return static_cast<ColorType>((static_cast<uint16_t>(color) * (brightness + 1u)) >> 8u);
    }

  public:
    constexpr Pixel() { }
    /*
     * Goes through SetPixel() rather than initialising the array in argument order,
     * which is what it used to do: the raw array is ordered by the ColorOffset
     * constants (green first, for the WS2812 wire order), so a positional initialiser
     * put red where GetRed() does not look for it. Nothing visible came of it while
     * every colour built this way was grey or black, and the simulator could not have
     * shown it either - it renders brightness, not hue. On the strip it swaps red and
     * green. Keeping SetPixel() as the only place that knows the layout is what stops
     * that from coming back.
     */
    constexpr Pixel(ColorType color_red, ColorType color_green, ColorType color_blue) {
        SetPixel(color_red, color_green, color_blue);
    }
    ~Pixel() { }

    // get methods
    static constexpr byte GetNumberOfColors() { return kNumberOfColors; }
    ColorType GetRed() const { return pixel_raw_[kColorOffsetRed]; }
    ColorType GetBlue() const { return pixel_raw_[kColorOffsetBlue]; }
    ColorType GetGreen() const { return pixel_raw_[kColorOffsetGreen]; }

    // set methods
    void SetRed(ColorType value) { pixel_raw_[kColorOffsetRed] = value; }
    void SetBlue(ColorType value) { pixel_raw_[kColorOffsetBlue] = value; }
    void SetGreen(ColorType value) { pixel_raw_[kColorOffsetGreen] = value; }

    // constexpr so the constructor can use it, and thereby be the only place in the
    // class that maps a colour name onto a raw index.
    constexpr void SetPixel(ColorType red, ColorType green, ColorType blue) {
         pixel_raw_[kColorOffsetRed] = red;
         pixel_raw_[kColorOffsetBlue] = blue;
         pixel_raw_[kColorOffsetGreen] = green;
    }

    // methods
    void ClearPixel() { SetPixel(0u, 0u, 0u); }

    // dimms all three colors, kColorMaxValue keeps the pixel unchanged. Dimms the
    // color as a whole, so the hue is preserved and only the brightness changes.
    void DimmPixel(byte brightness) {
        SetPixel(DimmColor(GetRed(), brightness), DimmColor(GetGreen(), brightness), DimmColor(GetBlue(), brightness));
    }

    void IncrementRed() { if(pixel_raw_[kColorOffsetRed] < kColorMaxValue) pixel_raw_[kColorOffsetRed]++; }
    void IncrementGreen() { if(pixel_raw_[kColorOffsetGreen] < kColorMaxValue) pixel_raw_[kColorOffsetGreen]++; }
    void IncrementBlue() { if(pixel_raw_[kColorOffsetBlue] < kColorMaxValue) pixel_raw_[kColorOffsetBlue]++; }

    void DecrementRed() { if(pixel_raw_[kColorOffsetRed] > 0u) pixel_raw_[kColorOffsetRed]--; }
    void DecrementGreen() { if(pixel_raw_[kColorOffsetGreen] > 0u) pixel_raw_[kColorOffsetGreen]--; }
    void DecrementBlue() { if(pixel_raw_[kColorOffsetBlue] > 0u) pixel_raw_[kColorOffsetBlue]--; }
};

#endif
