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

/* DisplayColor configuration parameter */
#define DISPLAY_COLOR_SUPPORT_DIMMING                   STD_ON

/* DisplayColor parameter */

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

    // functions
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
    DisplayColor() { }
    ~DisplayColor() { }

	// get methods
    Pixel GetColor() const { return color_; }

    /*
     * What is to reach the strip, which is not always the setting - the one place that
     * difference is decided, so every writer of a pixel and the current limit with them see
     * the cycle without having to know there is one.
     */
    Pixel GetColorToShow() const { return cycle_active_ ? cycle_color_ : color_; }
    byte GetColorRed() const { return color_.GetRed(); }
    byte GetColorGreen() const { return color_.GetGreen(); }
    byte GetColorBlue() const { return color_.GetBlue(); }
        
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    Pixel GetColorDimmed() const { return color_dimmed_; }
    byte GetColorRedDimmed() const { return color_dimmed_.GetRed(); }
    byte GetColorGreenDimmed() const { return color_dimmed_.GetGreen(); }
    byte GetColorBlueDimmed() const { return color_dimmed_.GetBlue(); }
#endif

	// set methods
    void SetColor(Pixel color) { color_ = color; }
    void SetColorRed(byte red) { color_.SetRed(red); }
    void SetColorGreen(byte green) { color_.SetGreen(green); }
    void SetColorBlue(byte blue) { color_.SetBlue(blue); }

    // The cycle's own pair. Reading it back is what lets a test say which of the two levels
    // the strip is on without going through the strip.
    bool IsCycleActive() const { return cycle_active_; }
    Pixel GetCycleColor() const { return cycle_color_; }
    void SetCycleColor(Pixel cycle_color) { cycle_color_ = cycle_color; cycle_active_ = true; }
    void ClearCycle() { cycle_active_ = false; }

	// methods
    /*
     * The cycle is not reset here. It belongs to ColorCycle, which has its own defaults and
     * is asked for them by the same reset - and a colour reset that stopped the cycle would
     * be command 2's reset quietly switching off command 15.
     */
    void ResetToDefaults() { SetColor(Pixel(kColorInitValue, kColorInitValue, kColorInitValue)); }

    void IncrementColorRed() { color_.IncrementRed(); }
    void IncrementColorGreen() { color_.IncrementGreen(); }
    void IncrementColorBlue() { color_.IncrementBlue(); }
    
    void DecrementColorRed() { color_.DecrementRed(); }
    void DecrementColorGreen() { color_.DecrementGreen(); }
    void DecrementColorBlue() { color_.DecrementBlue(); }    
    
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    void DimmColorRed(byte brightness) { color_dimmed_.SetRed(DimmColor(GetColorToShow().GetRed(), brightness)); }
    void DimmColorGreen(byte brightness) { color_dimmed_.SetGreen(DimmColor(GetColorToShow().GetGreen(), brightness)); }
    void DimmColorBlue(byte brightness) { color_dimmed_.SetBlue(DimmColor(GetColorToShow().GetBlue(), brightness)); }
    
    void DimmColors(byte brightness) {
        DimmColorRed(brightness);
        DimmColorGreen(brightness);
        DimmColorBlue(brightness);
    }
#endif
};

#endif
