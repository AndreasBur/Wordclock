// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The overlay that shows the date.
 */

#ifndef _OVERLAY_DATE_H_
#define _OVERLAY_DATE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"

/*!
 * \brief The date overlay: scrolls the current date over the clock.
 *
 * The date is the day, the month and the year in full, with a dot after the day and the month and
 * no leading zeros, as "4.11.2026". It is written from the current date at each start and kept
 * while the overlay shows.
 *
 * The date always has something to show, so CanShow() is always true. The overlay has no settings
 * of its own beyond those of Overlay.
 */
class OverlayDate : public Overlay<OverlayDate>
{
  public:

  private:
    friend class Overlay;
    static constexpr byte kDateStringLength{10u + 1u};
    char date_string_[kDateStringLength]{0u};

    void SetStateToShow(ClockDate current_date, ClockTime current_time) {
        SetDateString(current_date);
        SetText();
        UNUSED(current_time);
    }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) {
        Text::GetInstance().Stop();
        UNUSED(current_date);
        UNUSED(current_time);
    }

    /*
     * The date is always there to be shown; only the temperature has to wait for a
     * reading. Answered here rather than defaulted in the base, so that adding an overlay
     * is a decision about this and not a silent inheritance of it.
     */
    static bool IsReady() { return true; }

    // Nothing of its own: the string it shows is built from the current date on every
    // start, so there is no configuration here for a reset to undo.
    static void ResetOwnToDefaults() { }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::StateType::kIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(date_string_, GetFont()); }

    void SetDateString(ClockDate current_date) {
        char* tmp = date_string_;
        itoa(current_date.GetDay(), tmp, 10u);
        tmp = CheckDigitsAndAppendDot(current_date.GetDay(), tmp);
        itoa(current_date.GetMonth(), tmp, 10u);
        tmp = CheckDigitsAndAppendDot(current_date.GetMonth(), tmp);
        itoa(current_date.GetYear(), tmp, 10u);
    }

    static char* AppendDot(char* string) { string[0u] = '.'; return &string[1u]; }
    static char* CheckDigitsAndAppendDot(uint16_t value, char* string) { return AppendDot(&string[DigitsOfNumber(value)]); }

  public:
    /*! \brief Makes the date overlay, switched off. */
    constexpr OverlayDate() { }

    /*! \brief Destroys the date overlay. */
    ~OverlayDate() = default;

    /*!
     * \brief Takes the overlay one second further, as Overlay::Task() does, and starts the date
     *        again each time it has scrolled through.
     *
     * While the overlay shows, it first starts the date anew if the text output is idle, which it
     * is once the previous run has come to its end; then it runs Overlay::Task().
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
