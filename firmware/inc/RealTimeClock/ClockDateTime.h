// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A date and a time of day together.
 */

#ifndef _CLOCK_DATE_TIME_H_
#define _CLOCK_DATE_TIME_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDate.h"
#include "ClockTime.h"

/* ClockDateTime configuration parameter */

/* ClockDateTime parameter */

class ClockDateTime
{
  public:
    using HourType = ClockTime::HourType;
    using MinuteType = ClockTime::MinuteType;
    using SecondType = ClockTime::SecondType;

    using YearType = ClockDate::YearType;
    using MonthType = ClockDate::MonthType;
    using DayType = ClockDate::DayType;
    using WeekdayType = ClockDate::WeekdayType;

  private:
    ClockTime time_;
    ClockDate date_;

    // functions

  public:
    constexpr ClockDateTime() {}
    ~ClockDateTime() {}

	// get methods
    ClockTime GetTime() const { return time_; }
    HourType GetTimeHour() const { return time_.GetHour(); }
    MinuteType GetTimeMinute() const { return time_.GetMinute(); }
    SecondType GetTimeSecond() const { return time_.GetSecond(); }

    ClockDate GetDate() const { return date_; }
    YearType GetDateYear() const { return date_.GetYear(); }
    MonthType GetDateMonth() const { return date_.GetMonth(); }
    DayType GetDateDay() const { return date_.GetDay(); }
    WeekdayType GetDateWeekday() const { return date_.GetWeekday(); }

    // set methods
    void SetTime(ClockTime time) { time_ = time; }
    StdReturnType SetTimeHour(HourType hour) { return time_.SetHour(hour); }
    StdReturnType SetTimeMinute(MinuteType minute) { return time_.SetMinute(minute); }
    StdReturnType SetTimeSecond(SecondType second) { return time_.SetSecond(second); }

    void SetDate(ClockDate date) { date_ = date; }
    StdReturnType SetDateYear(YearType year) { return date_.SetYear(year); }
    StdReturnType SetDateMonth(MonthType month) { return date_.SetMonth(month); }
    StdReturnType SetDateDay(DayType day) { return date_.SetDay(day); }

	// methods
    static DayType GetDaysOfMonth(MonthType month, YearType year) { return ClockDate::GetDaysOfMonth(month, year); }
};

#endif
