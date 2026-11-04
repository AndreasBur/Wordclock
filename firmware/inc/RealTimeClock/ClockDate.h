// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A calendar date: year, month and day, with its weekday.
 */

#ifndef _CLOCK_DATE_H_
#define _CLOCK_DATE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* ClockDate configuration parameter */

/* ClockDate parameter */

class ClockDate
{
  public:
    enum class WeekdayType {
        kSunday = 0u,
        kMonday,
        kTuesday,
        kWednesday,
        kThursday,
        kFriday,
        kSaturday,
    };

    enum class MonthEnumType {
        kJanuary = 1u,
        kFebruary,
        kMarch,
        kApril,
        kMay,
        kJune,
        kJuly,
        kAugust,
        kSeptember,
        kOctober,
        kNovember,
        kDezember
    };

    using YearType = uint16_t;
    using MonthType = byte;
    using DayType = byte;
    using RataDieDayType = uint32_t;

  private:
    YearType year_{kYearMinValue};
    MonthType month_{kMonthMinValue};
    DayType day_{kDayMinValue};

    static constexpr YearType kYearMinValue{2000u};
    static constexpr YearType kYearMaxValue{2099u};
    static constexpr MonthType kMonthMinValue{1u};
    static constexpr MonthType kMonthMaxValue{12u};
    static constexpr DayType kDayMinValue{1u};
    static constexpr DayType kDayMaxValue{31u};

    // functions
    static constexpr DayType GetWeekday(YearType year, MonthType month, DayType day) {
        return (day+=month < 3 ? year-- : year-2, 23*month/9+day+4+year/4-year/100+year/400)%7;
    }

  public:
    constexpr ClockDate() { }
    constexpr ClockDate(YearType year, MonthType month, DayType day)
    {
        SetYear(year);
        SetMonth(month);
        SetDay(day);
    }
    ~ClockDate() {}

	// get methods
    YearType GetYear() const { return year_; }
    MonthType GetMonth() const { return month_; }
    MonthEnumType GetMonthEnum() const { return static_cast<MonthEnumType>(month_); }
    DayType GetDay() const { return day_; }
    WeekdayType GetWeekday() const {
        return static_cast<WeekdayType>(GetWeekday(year_, month_, day_));
    }

	// set methods
    constexpr StdReturnType SetYear(YearType year)
    {
        if(IsYearValid(year)) {
            year_ = year;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    constexpr StdReturnType SetMonth(MonthType month)
    {
        if(IsMonthValid(month)) {
            month_ = month;
            return kEOk;
        } else {
            return kENotOk; }
    }
    constexpr StdReturnType SetDay(DayType day)
    {
        if(IsDayValid(day)) {
            day_ = day;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

	// methods
    static constexpr bool IsYearValid(YearType year) { return (year >= kYearMinValue) && (year <= kYearMaxValue); }
    static constexpr bool IsMonthValid(MonthType month) { return (month >= kMonthMinValue) && (month <= kMonthMaxValue); }
    static constexpr bool IsDayValid(DayType day) { return (day >= kDayMinValue) && (day <= kDayMaxValue); }

    static RataDieDayType GetRataDieDay(YearType year, MonthType month, DayType day) {
        if (month < 3u) { year--, month += 12u; }
        return static_cast<RataDieDayType>(365u)*year+year/4u-year/100u+year/400u+(153u*month-457u)/5u+day-306u;
    }

    RataDieDayType GetPassedDays(YearType year, MonthType month, DayType day) const {
        const RataDieDayType requested_day = GetRataDieDay(year, month, day);
        const RataDieDayType current_day = GetRataDieDay(year_, month_, day_);
        return requested_day >= current_day ? requested_day - current_day : current_day - requested_day;
    }

    static DayType GetDaysOfMonth(MonthType month, YearType year) {
        switch (month) {
            case 4u:
            case 6u:
            case 9u:
            case 11u:
                return 30u;
            case 2u:
                return (year % 4u == 0u) ? 29u : 28u;
            case 1u:
            case 3u:
            case 5u:
            case 7u:
            case 8u:
            case 10u:
            case 12u:
                return 31u;
            default:
                return 0u;
        }
    }
};

#endif
