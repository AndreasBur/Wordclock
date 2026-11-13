// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The real-time clock of both Arduino backends, backed by the system clock.
 *
 * The system clock's conversions go through the C library's local time, so the zone and the
 * daylight-saving rule the backend installed - configTzTime() on the ESP32,
 * wordclock_time::ApplyTimeZone() on the RP2350 - apply in both directions. The chip's go through
 * UTC and leave the rule out.
 */

#include "RealTimeClock.h"

#include <sys/time.h>
#include <time.h>

namespace {
/* tm counts years from 1900 and months from zero, ClockDate counts neither. */
constexpr int kTmYearOffset{1900};
constexpr int kTmMonthOffset{1};

void ToDateTime(const struct tm& local, ClockDateTime& date_time) {
  date_time.SetDateYear(static_cast<ClockDateTime::YearType>(local.tm_year + kTmYearOffset));
  date_time.SetDateMonth(static_cast<ClockDateTime::MonthType>(local.tm_mon + kTmMonthOffset));
  date_time.SetDateDay(static_cast<ClockDateTime::DayType>(local.tm_mday));
  date_time.SetTimeHour(static_cast<ClockDateTime::HourType>(local.tm_hour));
  date_time.SetTimeMinute(static_cast<ClockDateTime::MinuteType>(local.tm_min));
  date_time.SetTimeSecond(static_cast<ClockDateTime::SecondType>(local.tm_sec));
}

// The chip keeps UTC, so the two conversions for it leave the zone rule out: gmtime_r and
// ToSecondsUtc() are what localtime_r and mktime are for the display's local time.
void ToDateTimeUtc(time_t const& seconds, ClockDateTime& date_time) {
  struct tm utc{};

  gmtime_r(&seconds, &utc);
  ToDateTime(utc, date_time);
}

constexpr int32_t kSecondsPerMinute{60};
constexpr int32_t kSecondsPerHour{60 * kSecondsPerMinute};
constexpr int32_t kSecondsPerDay{24 * kSecondsPerHour};

/*
 * Days from 1970-01-01 to a civil date, by Howard Hinnant's days_from_civil: the year is
 * shifted so that it begins in March, which puts the leap day at the end of it and makes
 * the month lengths a repeating pattern rather than a table.
 *
 * Written out rather than left to timegm(), which mktime's counterpart would be: newlib
 * declares it on neither of the two cores this firmware is built against, so writing it
 * out is what keeps one source serving both. The other way round - a local time to
 * seconds - keeps using mktime, which is there and which has to apply the zone rule
 * anyway.
 */
constexpr int32_t DaysFromCivil(int32_t year, int32_t month, int32_t day) {
  year -= (month <= 2) ? 1 : 0;

  int32_t const era = ((year >= 0) ? year : (year - 399)) / 400;
  int32_t const year_of_era = year - (era * 400);
  int32_t const day_of_year = (((153 * (month + ((month > 2) ? -3 : 9))) + 2) / 5) + day - 1;
  int32_t const day_of_era =
      (year_of_era * 365) + (year_of_era / 4) - (year_of_era / 100) + day_of_year;

  return (era * 146097) + day_of_era - 719468;
}

// The epoch itself, the leap day that the shifted year is built around, and both ends of
// what ClockDate covers.
static_assert(DaysFromCivil(1970, 1, 1) == 0, "the epoch must be day zero");
static_assert(DaysFromCivil(2000, 3, 1) == 11017, "the day after a leap day must line up");
static_assert(DaysFromCivil(2026, 8, 14) == 20679, "an ordinary day must line up");
static_assert(DaysFromCivil(2099, 12, 31) == 47481, "the last day ClockDate accepts must line up");

// Widened before the multiplication, not after it: in 32 bit the days alone overflow on
// 2038-01-19, and ClockDate runs to 2099.
constexpr time_t ToSecondsSinceEpoch(int32_t days, int32_t hour, int32_t minute, int32_t second) {
  return (static_cast<time_t>(days) * kSecondsPerDay) +
         (static_cast<time_t>(hour) * kSecondsPerHour) +
         (static_cast<time_t>(minute) * kSecondsPerMinute) + second;
}

static_assert(ToSecondsSinceEpoch(DaysFromCivil(2099, 12, 31), 23, 59, 59) == 4102444799,
              "the last second ClockDate accepts must not overflow");

time_t ToSecondsUtc(ClockDateTime const& date_time) {
  int32_t const days = DaysFromCivil(static_cast<int32_t>(date_time.GetDateYear()),
                                     static_cast<int32_t>(date_time.GetDateMonth()),
                                     static_cast<int32_t>(date_time.GetDateDay()));

  return ToSecondsSinceEpoch(days, static_cast<int32_t>(date_time.GetTimeHour()),
                             static_cast<int32_t>(date_time.GetTimeMinute()),
                             static_cast<int32_t>(date_time.GetTimeSecond()));
}

void ToTm(ClockDateTime const& date_time, struct tm& local) {
  local = {};
  local.tm_year = static_cast<int>(date_time.GetDateYear()) - kTmYearOffset;
  local.tm_mon = static_cast<int>(date_time.GetDateMonth()) - kTmMonthOffset;
  local.tm_mday = static_cast<int>(date_time.GetDateDay());
  local.tm_hour = static_cast<int>(date_time.GetTimeHour());
  local.tm_min = static_cast<int>(date_time.GetTimeMinute());
  local.tm_sec = static_cast<int>(date_time.GetTimeSecond());
  /*
   * Unknown rather than off, so mktime() decides from the zone rule whether the wall
   * clock time it was handed is in summer or winter time. Hardcoding zero would move
   * the clock by an hour for half the year.
   */
  local.tm_isdst = -1;
}

}  // namespace

bool RealTimeClock::IsSystemTimeSet() {
  ClockDateTime unused;
  return ReadSystemDateTime(unused);
}

void RealTimeClock::SetDateTime(ClockDateTime date_time) {
  date_time_ = date_time;
  WriteSystemDateTime(date_time_);
  /*
   * A time set by hand is worth keeping over the next power cut as much as one from the
   * network, so the chip is written again on the next task rather than at the turn of
   * the hour it would otherwise wait for.
   */
  hour_written_to_chip_ = kNoHourWritten;
}

void RealTimeClock::SetTime(ClockTime time) {
  ClockDateTime value = date_time_;
  value.SetTime(time);
  SetDateTime(value);
}

void RealTimeClock::SetDate(ClockDate date) {
  ClockDateTime value = date_time_;
  value.SetDate(date);
  SetDateTime(value);
}

void RealTimeClock::Task() {
  ClockDateTime current;

  /*
   * An unset clock is left alone rather than shown. Before the first SNTP answer the
   * system clock sits in 1970, which ClockDate rejects outright - so the display would
   * otherwise keep whatever the failed setters left behind.
   */
  if (!ReadSystemDateTime(current)) {
    ReadChip();
    return;
  }

  date_time_ = current;
  WriteChip();
}

// Reads the system clock as local date and time. True if it holds a year ClockDate accepts;
// `value` is then written, and otherwise left as it was.
bool RealTimeClock::ReadSystemDateTime(ClockDateTime& value) {
  time_t const now = time(nullptr);
  struct tm local{};

  if (localtime_r(&now, &local) == nullptr) {
    return false;
  }
  if (!ClockDate::IsYearValid(static_cast<ClockDate::YearType>(local.tm_year + kTmYearOffset))) {
    return false;
  }

  ToDateTime(local, value);
  return true;
}

/*
 * Puts what the clock chip kept into the system clock, and answers kEOk if the chip had a time
 * and the system clock took it.
 *
 * Only ever called while the system clock holds nothing, so there is nothing here that could
 * overwrite a synchronised time. Retried once a second until it works, which is what picks up
 * a chip that answers late - and what keeps the retry off a bus the light sensor shares.
 */
StdReturnType RealTimeClock::ReadChip() {
  if (chip_read_countdown_ > 0u) {
    chip_read_countdown_--;
    return kENotOk;
  }

  chip_read_countdown_ = kChipReadInterval;

  ClockDateTime utc;

  if (chip_.GetDateTime(utc) == kENotOk) {
    return kENotOk;
  }

  const struct timeval time{ToSecondsUtc(utc), 0};
  if (settimeofday(&time, nullptr) != 0) {
    return kENotOk;
  }

  // Not written back at once: what was just read is what the chip already holds, and the
  // hour it belongs to is what the next write is measured against.
  hour_written_to_chip_ = utc.GetTimeHour();
  return kEOk;
}

/*
 * Keeps the clock chip in step with the system clock: once an hour, and once more whenever the
 * time was set by hand. Once an hour because the chip is the more accurate of the two and needs
 * no correcting - what the write is for is the case where SNTP has moved the system clock and
 * the chip would otherwise keep an old time for the next power cut. The hour is remembered
 * rather than a countdown kept, so a clock that was off for a while writes on its first hour
 * and not a whole period later.
 *
 * A failed write is not retried within the hour: the next one comes anyway, and a chip that is
 * not answering would otherwise be talked to on every single tick.
 */
void RealTimeClock::WriteChip() {
  if (date_time_.GetTimeHour() == hour_written_to_chip_) {
    return;
  }

  ClockDateTime utc;
  ToDateTimeUtc(time(nullptr), utc);

  chip_.SetDateTime(utc);
  hour_written_to_chip_ = date_time_.GetTimeHour();
}

// Puts a date and time into the system clock, as local time; kEOk if the system clock took
// the value.
StdReturnType RealTimeClock::WriteSystemDateTime(ClockDateTime const& value) {
  struct tm local{};
  ToTm(value, local);

  time_t const seconds = mktime(&local);
  if (seconds == static_cast<time_t>(-1)) {
    return kENotOk;
  }

  const struct timeval time{seconds, 0};
  if (settimeofday(&time, nullptr) != 0) {
    return kENotOk;
  }

  return kEOk;
}
