// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

// The weekday of every date the clock can hold.

#include "ClockDate.h"
#include "test/cases.h"
#include "test/check.h"

/*
 * Every date from 1 January 2000 to 31 December 2099, against a count that knows nothing of the
 * formula: 1 January 2000 was a Saturday, and every day after it is the next weekday. Within
 * these years every fourth year is a leap year, 2000 included.
 *
 * All of them rather than a few, because the formula used to add the year to the day in a byte:
 * the sum wrapped, and two dates out of three came out wrong - but not 14 August 2026, the one
 * date the DS3231 test writes, which is how it went unnoticed.
 */
void TestWeekdayOfEveryDate() {
  constexpr byte kDaysOfMonth[]{31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u};
  byte weekday{static_cast<byte>(ToUnderlying(ClockDate::WeekdayType::kSaturday))};
  uint16_t wrong{0u};

  for (uint16_t year = 2000u; year <= 2099u; year++) {
    for (byte month = 1u; month <= 12u; month++) {
      bool const is_leap_february{(month == 2u) && (year % 4u == 0u)};
      byte const days{static_cast<byte>(kDaysOfMonth[month - 1u] + (is_leap_february ? 1u : 0u))};

      for (byte day = 1u; day <= days; day++) {
        ClockDate const date(year, month, day);
        if (ToUnderlying(date.GetWeekday()) != weekday) {
          wrong++;
        }
        weekday = static_cast<byte>((weekday + 1u) % 7u);
      }
    }
  }
  Expect(wrong == 0u, "every date from 2000 to 2099 falls on its weekday");
}
