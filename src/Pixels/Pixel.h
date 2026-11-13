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

#define PIXEL_COLOR_OFFSET_GREEN                   0u /*!< The index of green in a pixel. */
#define PIXEL_COLOR_OFFSET_RED                     1u /*!< The index of red in a pixel. */
#define PIXEL_COLOR_OFFSET_BLUE                    2u /*!< The index of blue in a pixel. */

/*!
 * \brief One pixel: its red, green and blue values, stored in the order the LED strip wants.
 *
 * The PIXEL_COLOR_OFFSET macros give the order, green first for the WS2812.
 */
class Pixel
{
  public:
    /*! \brief The value of one colour, from 0 for off to 255 for full. */
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

    static constexpr ColorType DimmColor(ColorType color, byte brightness) {
        return static_cast<ColorType>((static_cast<uint16_t>(color) * (brightness + 1u)) >> 8u);
    }

  public:
    /*! \brief Makes a pixel that is off. */
    constexpr Pixel() { }

    /*!
     * \brief Makes a pixel of this colour.
     *
     * \param[in] color_red The red value.
     * \param[in] color_green The green value.
     * \param[in] color_blue The blue value.
     */
    constexpr Pixel(ColorType color_red, ColorType color_green, ColorType color_blue) {
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
        SetPixel(color_red, color_green, color_blue);
    }

    /*! \brief Destroys the pixel. */
    ~Pixel() = default;

    /*! \brief Returns the number of colours of a pixel, three. */
    static constexpr byte GetNumberOfColors() { return kNumberOfColors; }

    /*! \brief Returns the red value. */
    ColorType GetRed() const { return pixel_raw_[kColorOffsetRed]; }

    /*! \brief Returns the blue value. */
    ColorType GetBlue() const { return pixel_raw_[kColorOffsetBlue]; }

    /*! \brief Returns the green value. */
    ColorType GetGreen() const { return pixel_raw_[kColorOffsetGreen]; }

    /*!
     * \brief Sets the red value.
     *
     * \param[in] value The red value.
     */
    void SetRed(ColorType value) { pixel_raw_[kColorOffsetRed] = value; }

    /*!
     * \brief Sets the blue value.
     *
     * \param[in] value The blue value.
     */
    void SetBlue(ColorType value) { pixel_raw_[kColorOffsetBlue] = value; }

    /*!
     * \brief Sets the green value.
     *
     * \param[in] value The green value.
     */
    void SetGreen(ColorType value) { pixel_raw_[kColorOffsetGreen] = value; }

    /*!
     * \brief Sets all three colours.
     *
     * \param[in] red The red value.
     * \param[in] green The green value.
     * \param[in] blue The blue value.
     */
    constexpr void SetPixel(ColorType red, ColorType green, ColorType blue) {
         // constexpr so the constructor can use it, and thereby be the only place in the
         // class that maps a colour name onto a raw index.
         pixel_raw_[kColorOffsetRed] = red;
         pixel_raw_[kColorOffsetBlue] = blue;
         pixel_raw_[kColorOffsetGreen] = green;
    }

    /*! \brief Turns the pixel off: all three colours to 0. */
    void ClearPixel() { SetPixel(0u, 0u, 0u); }

    /*!
     * \brief Dims all three colours by `brightness`.
     *
     * It dims the colour as a whole, so the hue is preserved and only the brightness changes. Each
     * colour is multiplied by `brightness` + 1 and divided by 256, rounding down.
     *
     * \param[in] brightness 255 keeps the pixel unchanged, 0 turns it off.
     */
    void DimmPixel(byte brightness) {
        SetPixel(DimmColor(GetRed(), brightness), DimmColor(GetGreen(), brightness), DimmColor(GetBlue(), brightness));
    }

    /*! \brief Raises the red value by one, unless it is 255 already. */
    void IncrementRed() { if(pixel_raw_[kColorOffsetRed] < kColorMaxValue) pixel_raw_[kColorOffsetRed]++; }

    /*! \brief Raises the green value by one, unless it is 255 already. */
    void IncrementGreen() { if(pixel_raw_[kColorOffsetGreen] < kColorMaxValue) pixel_raw_[kColorOffsetGreen]++; }

    /*! \brief Raises the blue value by one, unless it is 255 already. */
    void IncrementBlue() { if(pixel_raw_[kColorOffsetBlue] < kColorMaxValue) pixel_raw_[kColorOffsetBlue]++; }

    /*! \brief Lowers the red value by one, unless it is 0 already. */
    void DecrementRed() { if(pixel_raw_[kColorOffsetRed] > 0u) pixel_raw_[kColorOffsetRed]--; }

    /*! \brief Lowers the green value by one, unless it is 0 already. */
    void DecrementGreen() { if(pixel_raw_[kColorOffsetGreen] > 0u) pixel_raw_[kColorOffsetGreen]--; }

    /*! \brief Lowers the blue value by one, unless it is 0 already. */
    void DecrementBlue() { if(pixel_raw_[kColorOffsetBlue] > 0u) pixel_raw_[kColorOffsetBlue]--; }
};

#endif
