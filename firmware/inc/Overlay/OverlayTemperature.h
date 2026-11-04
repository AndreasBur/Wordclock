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

/*!
 * \brief The temperature overlay: scrolls the last reading of the DS3231 over the clock.
 *
 * The reading is written as Temperature::GetTemperatureStringToShow() writes it: 21.5 degrees as
 * "21.5", the degree sign and "C". It is taken at each start and kept while the overlay shows.
 *
 * While there is no reading, as until the chip has answered once, CanShow() is false: the overlay
 * does not start, by its period or by the "show now" procedure. A clock built without the chip
 * never shows it. The overlay has no settings of its own beyond those of Overlay.
 */
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
    /*! \brief Makes the temperature overlay, switched off. */
    constexpr OverlayTemperature() { }

    /*! \brief Destroys the temperature overlay. */
    ~OverlayTemperature() { }

    /*!
     * \brief Returns the reading the overlay took at its last start.
     *
     * \return The reading, ended by a null character, with the degree sign as the Latin-1 byte
     *         0xB0; an empty string before the first start, and after a start without a reading.
     */
    const char* GetTemperatureString() const { return temperature_string_; }

    /*!
     * \brief Takes the overlay one second further, as Overlay::Task() does, and starts the reading
     *        again each time it has scrolled through.
     *
     * While the overlay shows, it first starts the reading anew if the text output is idle, which
     * it is once the previous run has come to its end; then it runs Overlay::Task(). The reading
     * scrolled anew is the one taken at the start.
     *
     * \param[in] show_timer_in_seconds The show timer, in seconds.
     * \param[in] current_date The date now.
     * \param[in] current_time The time now.
     * \return The new show timer, in seconds, as Overlay::Task() returns it.
     */
    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == StateType::kShow) { ShowTask(); }
        return Overlay::Task(show_timer_in_seconds, current_date, current_time);
    }
};

#endif
