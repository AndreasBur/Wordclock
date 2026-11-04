// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The overlay that shows the temperature.
 */

#ifndef _OVERLAY_TEMPERATURE_H_
#define _OVERLAY_TEMPERATURE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"
#include "Temperature.h"

/* OverlayTemperature configuration parameter */

/* OverlayTemperature parameter */

class OverlayTemperature : public Overlay<OverlayTemperature>
{
  public:

  private:
    friend class Overlay;

    /*
     * Written when the overlay starts and left alone while it shows: the text scrolls out
     * of this buffer, and a reading that changed halfway through would swap a digit under
     * the letters that are already on their way across.
     */
    char temperature_string_[Temperature::kStringToShowLength]{0u};

    // functions
    // Only ever entered with a reading in hand: IsReady() is what the overlay is started
    // through, and it is what says whether there is one.
    void SetStateToShow(ClockDate current_date, ClockTime current_time) {
        SetTemperatureString();
        SetText();
        UNUSED(current_date);
        UNUSED(current_time);
    }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) {
        Text::GetInstance().Stop();
        UNUSED(current_date);
        UNUSED(current_time);
    }

    /*
     * No reading, no overlay: a clock without the chip - and every clock until it has
     * answered once - would otherwise hold the display for the whole endurance to show
     * nothing, or show a zero that reads like a measurement.
     */
    static bool IsReady() { return Temperature::GetInstance().IsTemperatureAvailable(); }

    // Nothing of its own: the string it shows comes from the chip on every start, so
    // there is no configuration here for a reset to undo.
    static void ResetOwnToDefaults() { }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::StateType::kIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(temperature_string_, GetFont()); }

    void SetTemperatureString() { Temperature::GetInstance().GetTemperatureStringToShow(temperature_string_); }

  public:
    constexpr OverlayTemperature() { }
    ~OverlayTemperature() { }

	// get methods
    const char* GetTemperatureString() const { return temperature_string_; }

	// set methods

	// methods
    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == StateType::kShow) { ShowTask(); }
        return Overlay::Task(show_timer_in_seconds, current_date, current_time);
    }
};

#endif
