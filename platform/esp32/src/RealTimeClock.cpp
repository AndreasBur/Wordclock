// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief ESP32 real-time clock, backed by the system clock.
 *
 * All conversions go through the C library's local time, so the zone and the daylight-saving rule
 * that the application installed with configTzTime() apply in both directions.
 */

#include "RealTimeClock.h"

#include <sys/time.h>
#include <time.h>

namespace {
/* tm counts years from 1900 and months from zero, ClockDate counts neither. */
constexpr int kTmYearOffset{1900};
constexpr int kTmMonthOffset{1};

void ToDateTime(const struct tm& local, ClockDateTime& date_time)
{
    date_time.SetDateYear(static_cast<ClockDateTime::YearType>(local.tm_year + kTmYearOffset));
    date_time.SetDateMonth(static_cast<ClockDateTime::MonthType>(local.tm_mon + kTmMonthOffset));
    date_time.SetDateDay(static_cast<ClockDateTime::DayType>(local.tm_mday));
    date_time.SetTimeHour(static_cast<ClockDateTime::HourType>(local.tm_hour));
    date_time.SetTimeMinute(static_cast<ClockDateTime::MinuteType>(local.tm_min));
    date_time.SetTimeSecond(static_cast<ClockDateTime::SecondType>(local.tm_sec));
}

// The chip keeps UTC, so the two conversions for it leave the zone rule out: gmtime_r and
// timegm are what localtime_r and mktime are for the display's local time.
void ToDateTimeUtc(const time_t& seconds, ClockDateTime& date_time)
{
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
 * does not declare it on this target, so a build against the ESP32 core does not compile
 * with it. The other way round - a local time to seconds - keeps using mktime, which is
 * there and which has to apply the zone rule anyway.
 */
constexpr int32_t DaysFromCivil(int32_t year, int32_t month, int32_t day)
{
    year -= (month <= 2) ? 1 : 0;

    const int32_t era = ((year >= 0) ? year : (year - 399)) / 400;
    const int32_t year_of_era = year - (era * 400);
    const int32_t day_of_year = (((153 * (month + ((month > 2) ? -3 : 9))) + 2) / 5) + day - 1;
    const int32_t day_of_era = (year_of_era * 365) + (year_of_era / 4) - (year_of_era / 100) + day_of_year;

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
constexpr time_t ToSecondsSinceEpoch(int32_t days, int32_t hour, int32_t minute, int32_t second)
{
    return (static_cast<time_t>(days) * kSecondsPerDay) + (static_cast<time_t>(hour) * kSecondsPerHour) +
           (static_cast<time_t>(minute) * kSecondsPerMinute) + second;
}

static_assert(ToSecondsSinceEpoch(DaysFromCivil(2099, 12, 31), 23, 59, 59) == 4102444799,
              "the last second ClockDate accepts must not overflow");

time_t ToSecondsUtc(const ClockDateTime& date_time)
{
    const int32_t days = DaysFromCivil(static_cast<int32_t>(date_time.GetDateYear()),
                                       static_cast<int32_t>(date_time.GetDateMonth()),
                                       static_cast<int32_t>(date_time.GetDateDay()));

    return ToSecondsSinceEpoch(days,
                               static_cast<int32_t>(date_time.GetTimeHour()),
                               static_cast<int32_t>(date_time.GetTimeMinute()),
                               static_cast<int32_t>(date_time.GetTimeSecond()));
}

void ToTm(const ClockDateTime& date_time, struct tm& local)
{
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

} // namespace

/*! \brief          Tells whether the system clock holds a real date yet
 *  \details        Asked as "is the year one ClockDate accepts" rather than by querying
 *                  SNTP: that answers the question the firmware actually has, and it
 *                  stays true for a time that arrived from a command instead of the net.
 *
 *  \return         true once the system clock has been set
 */
bool RealTimeClock::IsSystemTimeSet()
{
    ClockDateTime unused;
    return ReadSystemDateTime(unused);
}

void RealTimeClock::SetDateTime(ClockDateTime date_time)
{
    date_time_ = date_time;
    WriteSystemDateTime(date_time_);
    /*
     * A time set by hand is worth keeping over the next power cut as much as one from the
     * network, so the chip is written again on the next task rather than at the turn of
     * the hour it would otherwise wait for.
     */
    hour_written_to_chip_ = kNoHourWritten;
}

void RealTimeClock::SetTime(ClockTime time)
{
    ClockDateTime value = date_time_;
    value.SetTime(time);
    SetDateTime(value);
}

void RealTimeClock::SetDate(ClockDate date)
{
    ClockDateTime value = date_time_;
    value.SetDate(date);
    SetDateTime(value);
}

/*! \brief          Takes the current local time from the system clock
 *  \details        Nothing is counted forward here, unlike on the simulator: SNTP keeps
 *                  the system clock right on its own, so counting would only add a
 *                  second source of truth that drifts away from it.
 *
 *                  The clock chip is the source of last resort rather than a second one:
 *                  it is read only while the system clock holds nothing, and written back
 *                  from the system clock afterwards. Which way round matters - the chip
 *                  drifts a few seconds a month and SNTP does not, so a chip that could
 *                  overwrite a synchronised clock would make the clock worse.
 */
void RealTimeClock::Task()
{
    ClockDateTime current;

    /*
     * An unset clock is left alone rather than shown. Before the first SNTP answer the
     * system clock sits in 1970, which ClockDate rejects outright - so the display would
     * otherwise keep whatever the failed setters left behind.
     */
    if(!ReadSystemDateTime(current)) {
        ReadChip();
        return;
    }

    date_time_ = current;
    WriteChip();
}

/*! \brief          Reads the system clock as local date and time
 *
 *  \return         true if the system clock holds a year ClockDate accepts
 */
bool RealTimeClock::ReadSystemDateTime(ClockDateTime& value)
{
    const time_t now = time(nullptr);
    struct tm local{};

    if(localtime_r(&now, &local) == nullptr) { return false; }
    if(!ClockDate::IsYearValid(static_cast<ClockDate::YearType>(local.tm_year + kTmYearOffset))) { return false; }

    ToDateTime(local, value);
    return true;
}

/*! \brief          Puts what the clock chip kept into the system clock
 *  \details        Only ever called while the system clock holds nothing, so there is
 *                  nothing here that could overwrite a synchronised time. Retried once a
 *                  second until it works, which is what picks up a chip that answers late -
 *                  and what keeps the retry off a bus the light sensor shares.
 *
 *  \return         kEOk if the chip had a time and the system clock took it
 */
StdReturnType RealTimeClock::ReadChip()
{
    if(chip_read_countdown_ > 0u) { chip_read_countdown_--; return kENotOk; }

    chip_read_countdown_ = kChipReadInterval;

    ClockDateTime utc;

    if(chip_.GetDateTime(utc) == kENotOk) { return kENotOk; }

    const struct timeval time{ToSecondsUtc(utc), 0};
    if(settimeofday(&time, nullptr) != 0) { return kENotOk; }

    // Not written back at once: what was just read is what the chip already holds, and the
    // hour it belongs to is what the next write is measured against.
    hour_written_to_chip_ = utc.GetTimeHour();
    return kEOk;
}

/*! \brief          Keeps the clock chip in step with the system clock
 *  \details        Once an hour, and once more whenever the time was set by hand. Once an
 *                  hour because the chip is the more accurate of the two and needs no
 *                  correcting - what the write is for is the case where SNTP has moved the
 *                  system clock and the chip would otherwise keep an old time for the next
 *                  power cut. The hour is remembered rather than a countdown kept, so a
 *                  clock that was off for a while writes on its first hour and not a
 *                  whole period later.
 *
 *                  A failed write is not retried within the hour: the next one comes
 *                  anyway, and a chip that is not answering would otherwise be talked to
 *                  on every single tick.
 */
void RealTimeClock::WriteChip()
{
    if(date_time_.GetTimeHour() == hour_written_to_chip_) { return; }

    ClockDateTime utc;
    ToDateTimeUtc(time(nullptr), utc);

    chip_.SetDateTime(utc);
    hour_written_to_chip_ = date_time_.GetTimeHour();
}

/*! \brief          Puts a date and time into the system clock
 *
 *  \return         kEOk if the system clock took the value
 */
StdReturnType RealTimeClock::WriteSystemDateTime(const ClockDateTime& value)
{
    struct tm local{};
    ToTm(value, local);

    const time_t seconds = mktime(&local);
    if(seconds == static_cast<time_t>(-1)) { return kENotOk; }

    const struct timeval time{seconds, 0};
    if(settimeofday(&time, nullptr) != 0) { return kENotOk; }

    return kEOk;
}
