// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The overlay that shows the date.
 */

#ifndef WORDCLOCK_SRC_OVERLAY_OVERLAYDATE_H_
#define WORDCLOCK_SRC_OVERLAY_OVERLAYDATE_H_

#include "Arduino.h"
#include "Overlay.h"
#include "StandardTypes.h"

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
class OverlayDate : public Overlay<OverlayDate> {
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
  static void ResetOwnToDefaults() {}

  void SetText() { Text::GetInstance().SetTextWithShift(date_string_, GetFont()); }

  void SetDateString(ClockDate current_date) {
    char* tmp = date_string_;
    itoa(current_date.GetDay(), tmp, 10u);
    tmp = CheckDigitsAndAppendDot(current_date.GetDay(), tmp);
    itoa(current_date.GetMonth(), tmp, 10u);
    tmp = CheckDigitsAndAppendDot(current_date.GetMonth(), tmp);
    itoa(static_cast<int>(current_date.GetYear()), tmp, 10u);
  }

  static char* AppendDot(char* string) {
    string[0u] = '.';
    return &string[1u];
  }
  static char* CheckDigitsAndAppendDot(uint16_t value, char* string) {
    return AppendDot(&string[DigitsOfNumber(value)]);
  }

 public:
  /*! \brief Makes the date overlay, switched off. */
  constexpr OverlayDate() {}

  /*! \brief Destroys the date overlay. */
  ~OverlayDate() = default;
};

#endif  // WORDCLOCK_SRC_OVERLAY_OVERLAYDATE_H_
