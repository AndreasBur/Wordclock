// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The colour of the display, and what it is dimmed to.
 */

#ifndef _DISPLAY_COLOR_H_
#define _DISPLAY_COLOR_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixel.h"

/*!
 * \brief Whether DisplayColor keeps a dimmed copy of the colour, STD_ON, or not, STD_OFF.
 *
 * With STD_ON the display dims by writing its pixels in that copy. With STD_OFF the strip's own
 * master brightness dims them instead, as DISPLAY_USE_PIXELS_DIMMING in Display.h then selects.
 */
#define DISPLAY_COLOR_SUPPORT_DIMMING                   STD_ON

/*!
 * \brief The colour of the display: the colour set, the colour cycle's colour above it, and, with
 *        DISPLAY_COLOR_SUPPORT_DIMMING, the dimmed copy the pixels are written in.
 *
 * A new clock's colour is white, 255 on all three channels. While the colour cycle runs, its
 * colour is shown instead of the colour set, and the colour set stays as it was underneath it.
 *
 * Every channel runs from 0 to 255. The dimmed copy follows neither colour by itself: DimmColors()
 * derives it from the colour to show, and until the next call it keeps what that call made.
 */
class DisplayColor
{
  public:

  private:
    // White, and named rather than written into the member below alone: it is also what
    // the reset puts back, and two literals could drift apart.
    static constexpr byte kColorInitValue{255u};

    Pixel color_{kColorInitValue, kColorInitValue, kColorInitValue};

    /*
     * What the colour cycle is currently on, and whether it is running at all. A level
     * above the setting rather than a write into it: the setting is what command 2 answers
     * and what Persistence stores, and a cycle that wrote it would save a colour nobody
     * chose - and leave nothing to put back when it stopped. The same arrangement the night
     * brightness has above the brightness.
     */
    bool cycle_active_{false};
    Pixel cycle_color_{color_};

#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    Pixel color_dimmed_{color_};
#endif

#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    /*
     * Never rounds a channel the colour has down to nothing: a lit word whose channels all
     * reached 0 would read as unlit at a low brightness. A channel the colour does not have
     * is the other way round - lifting it to 1 like the rest put a trace of green and blue
     * into a pure red, which is a different colour rather than a dimmer one.
     */
    static constexpr byte DimmColor(byte color, byte brightness) {
        if(color == 0u) { return 0u; }

        // One operand widened before the multiply: two bytes promote to a signed 16-bit
        // int on the AVR, and 255 * 255 does not fit in one.
        const byte dimmed_color = (static_cast<uint16_t>(color) * brightness) >> 8u;
        if(dimmed_color == 0u) { return 1u; }
        else { return dimmed_color; }
    }
#endif

  public:
    /*! \brief Makes the colour of a new clock: white, with the colour cycle stopped. */
    DisplayColor() { }

    /*! \brief Destroys the colour. */
    ~DisplayColor() = default;

    /*! \brief Returns the colour set, also while the colour cycle shows another. */
    Pixel GetColor() const { return color_; }

    /*!
     * \brief Returns the colour that is to reach the strip: the colour cycle's while it runs, the
     *        colour set otherwise.
     *
     * What is to reach the strip is not always the setting, and this is the one place that
     * difference is decided, so every writer of a pixel and the current limit with them see the
     * cycle without having to know there is one.
     */
    Pixel GetColorToShow() const { return cycle_active_ ? cycle_color_ : color_; }

    /*! \brief Returns the red channel of the colour set. */
    byte GetColorRed() const { return color_.GetRed(); }

    /*! \brief Returns the green channel of the colour set. */
    byte GetColorGreen() const { return color_.GetGreen(); }

    /*! \brief Returns the blue channel of the colour set. */
    byte GetColorBlue() const { return color_.GetBlue(); }

#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    /*! \brief Returns the dimmed copy, as it was last dimmed. */
    Pixel GetColorDimmed() const { return color_dimmed_; }

    /*! \brief Returns the red channel of the dimmed copy. */
    byte GetColorRedDimmed() const { return color_dimmed_.GetRed(); }

    /*! \brief Returns the green channel of the dimmed copy. */
    byte GetColorGreenDimmed() const { return color_dimmed_.GetGreen(); }

    /*! \brief Returns the blue channel of the dimmed copy. */
    byte GetColorBlueDimmed() const { return color_dimmed_.GetBlue(); }
#endif

    /*!
     * \brief Sets the colour.
     *
     * \param[in] color The colour.
     */
    void SetColor(Pixel color) { color_ = color; }

    /*!
     * \brief Sets the red channel of the colour.
     *
     * \param[in] red The red channel, 0 to 255.
     */
    void SetColorRed(byte red) { color_.SetRed(red); }

    /*!
     * \brief Sets the green channel of the colour.
     *
     * \param[in] green The green channel, 0 to 255.
     */
    void SetColorGreen(byte green) { color_.SetGreen(green); }

    /*!
     * \brief Sets the blue channel of the colour.
     *
     * \param[in] blue The blue channel, 0 to 255.
     */
    void SetColorBlue(byte blue) { color_.SetBlue(blue); }

    /*!
     * \brief Returns whether the colour cycle runs, so that its colour is the one shown.
     *
     * The cycle's own pair, with GetCycleColor(). Reading it back is what lets a test say which of
     * the two levels the strip is on without going through the strip.
     */
    bool IsCycleActive() const { return cycle_active_; }

    /*! \brief Returns the colour the colour cycle is on, kept after ClearCycle() too. */
    Pixel GetCycleColor() const { return cycle_color_; }

    /*!
     * \brief Sets the colour the colour cycle is on, and shows it instead of the colour set.
     *
     * \param[in] cycle_color The colour the cycle is on.
     */
    void SetCycleColor(Pixel cycle_color) { cycle_color_ = cycle_color; cycle_active_ = true; }

    /*! \brief Stops the colour cycle, so that the colour set is shown again. */
    void ClearCycle() { cycle_active_ = false; }

    /*!
     * \brief Puts the colour set back to white.
     *
     * The cycle is not reset here. It belongs to ColorCycle, which has its own defaults and is
     * asked for them by the same reset - and a colour reset that stopped the cycle would be
     * command 2's reset quietly switching off command 15. The dimmed copy is left as it was, too.
     */
    void ResetToDefaults() { SetColor(Pixel(kColorInitValue, kColorInitValue, kColorInitValue)); }

    /*! \brief Raises the red channel of the colour set by one, unless it is 255 already. */
    void IncrementColorRed() { color_.IncrementRed(); }

    /*! \brief Raises the green channel of the colour set by one, unless it is 255 already. */
    void IncrementColorGreen() { color_.IncrementGreen(); }

    /*! \brief Raises the blue channel of the colour set by one, unless it is 255 already. */
    void IncrementColorBlue() { color_.IncrementBlue(); }

    /*! \brief Lowers the red channel of the colour set by one, unless it is 0 already. */
    void DecrementColorRed() { color_.DecrementRed(); }

    /*! \brief Lowers the green channel of the colour set by one, unless it is 0 already. */
    void DecrementColorGreen() { color_.DecrementGreen(); }

    /*! \brief Lowers the blue channel of the colour set by one, unless it is 0 already. */
    void DecrementColorBlue() { color_.DecrementBlue(); }

#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    /*!
     * \brief Writes the red channel of the colour to show, dimmed by `brightness`, into the dimmed
     *        copy.
     *
     * The channel is dimmed as DimmColors() describes.
     *
     * \param[in] brightness The brightness, 0 to 255.
     */
    void DimmColorRed(byte brightness) { color_dimmed_.SetRed(DimmColor(GetColorToShow().GetRed(), brightness)); }

    /*!
     * \brief Writes the green channel of the colour to show, dimmed by `brightness`, into the
     *        dimmed copy.
     *
     * The channel is dimmed as DimmColors() describes.
     *
     * \param[in] brightness The brightness, 0 to 255.
     */
    void DimmColorGreen(byte brightness) { color_dimmed_.SetGreen(DimmColor(GetColorToShow().GetGreen(), brightness)); }

    /*!
     * \brief Writes the blue channel of the colour to show, dimmed by `brightness`, into the dimmed
     *        copy.
     *
     * The channel is dimmed as DimmColors() describes.
     *
     * \param[in] brightness The brightness, 0 to 255.
     */
    void DimmColorBlue(byte brightness) { color_dimmed_.SetBlue(DimmColor(GetColorToShow().GetBlue(), brightness)); }

    /*!
     * \brief Makes the dimmed copy: the colour to show, dimmed by `brightness`.
     *
     * Each channel is multiplied by `brightness` and divided by 256, rounding down. A channel of 0
     * stays 0, and any other channel never drops below 1, so that a lit word still reads as lit.
     * The colour set and the colour cycle's colour are left as they were.
     *
     * \param[in] brightness The brightness, 0 to 255.
     * \note A brightness of 0 leaves every channel the colour has at 1, not at 0.
     */
    void DimmColors(byte brightness) {
        DimmColorRed(brightness);
        DimmColorGreen(brightness);
        DimmColorBlue(brightness);
    }
#endif
};

#endif
