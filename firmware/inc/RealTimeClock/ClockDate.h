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

/*!
 * \brief A calendar date: year, month and day, with its weekday.
 *
 * A new date is 1 January 2000. Each setter takes a value in range only, a year from 2000 to
 * 2099, a month from 1 to 12 and a day from 1 to 31, and otherwise leaves its field as it was.
 * The day is not checked against the month, so 31 February is taken.
 */
class ClockDate
{
  public:
    /*! \brief A day of the week, from Sunday as 0 to Saturday as 6. */
    enum class WeekdayType {
        kSunday = 0u, /*!< Sunday. */
        kMonday, /*!< Monday. */
        kTuesday, /*!< Tuesday. */
        kWednesday, /*!< Wednesday. */
        kThursday, /*!< Thursday. */
        kFriday, /*!< Friday. */
        kSaturday, /*!< Saturday. */
    };

    /*! \brief A month of the year, from January as 1 to December as 12. */
    enum class MonthEnumType {
        kJanuary = 1u, /*!< January. */
        kFebruary, /*!< February. */
        kMarch, /*!< March. */
        kApril, /*!< April. */
        kMay, /*!< May. */
        kJune, /*!< June. */
        kJuly, /*!< July. */
        kAugust, /*!< August. */
        kSeptember, /*!< September. */
        kOctober, /*!< October. */
        kNovember, /*!< November. */
        kDezember /*!< December. */
    };

    /*! \brief A year, in full, 2000 to 2099. */
    using YearType = uint16_t;
    /*! \brief A month, 1 for January to 12 for December. */
    using MonthType = byte;
    /*! \brief A day of the month, 1 to 31. */
    using DayType = byte;
    /*! \brief A day in the Rata Die count, in which 1 January of the year 1 is day 1. */
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

    /*
     * Keith and Craver's formula, as it is usually written:
     * (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4 - y/100 + y/400) % 7.
     * Here the day and the year are added in a uint16_t. Added to the day itself, a byte, the
     * sum wrapped, and from 31 January 2017 on most dates came out on the wrong weekday.
     */
    static constexpr byte GetWeekday(YearType year, MonthType month, DayType day) {
        const bool is_before_march{month < 3u};
        const uint16_t day_and_year{static_cast<uint16_t>(day + (is_before_march ? year : year - 2u))};
        const YearType counted_year{is_before_march ? static_cast<YearType>(year - 1u) : year};

        return static_cast<byte>((23u * month / 9u + day_and_year + 4u + counted_year / 4u
                                  - counted_year / 100u + counted_year / 400u) % 7u);
    }

  public:
    /*! \brief Makes the date 1 January 2000. */
    constexpr ClockDate() { }

    /*!
     * \brief Makes the date `day` of `month` of `year`.
     *
     * A value out of range leaves its field as a new date has it; the others are taken all the
     * same.
     *
     * \param[in] year The year, 2000 to 2099.
     * \param[in] month The month, 1 to 12.
     * \param[in] day The day of the month, 1 to 31.
     */
    constexpr ClockDate(YearType year, MonthType month, DayType day)
    {
        SetYear(year);
        SetMonth(month);
        SetDay(day);
    }

    /*! \brief Destroys the date. */
    ~ClockDate() = default;

    /*! \brief Returns the year, 2000 to 2099. */
    YearType GetYear() const { return year_; }

    /*! \brief Returns the month, 1 to 12. */
    MonthType GetMonth() const { return month_; }

    /*! \brief Returns the month as a MonthEnumType. */
    MonthEnumType GetMonthEnum() const { return static_cast<MonthEnumType>(month_); }

    /*! \brief Returns the day of the month, 1 to 31. */
    DayType GetDay() const { return day_; }

    /*!
     * \brief Returns the weekday of the date.
     *
     * \return The weekday.
     */
    WeekdayType GetWeekday() const {
        return static_cast<WeekdayType>(GetWeekday(year_, month_, day_));
    }

    /*!
     * \brief Sets the year.
     *
     * \param[in] year The year, 2000 to 2099.
     * \return kEOk, or kENotOk if `year` lies below 2000 or above 2099; the year is then left as it
     *         was.
     */
    constexpr StdReturnType SetYear(YearType year)
    {
        if(IsYearValid(year)) {
            year_ = year;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Sets the month.
     *
     * \param[in] month The month, 1 to 12.
     * \return kEOk, or kENotOk if `month` is 0 or above 12; the month is then left as it was.
     */
    constexpr StdReturnType SetMonth(MonthType month)
    {
        if(IsMonthValid(month)) {
            month_ = month;
            return kEOk;
        } else {
            return kENotOk; }
    }

    /*!
     * \brief Sets the day of the month.
     *
     * The day is checked against 31, not against the days of the month.
     *
     * \param[in] day The day, 1 to 31.
     * \return kEOk, or kENotOk if `day` is 0 or above 31; the day is then left as it was.
     */
    constexpr StdReturnType SetDay(DayType day)
    {
        if(IsDayValid(day)) {
            day_ = day;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Returns whether `year` is a year the date takes.
     *
     * \param[in] year The year to check.
     * \return True if `year` lies from 2000 to 2099.
     */
    static constexpr bool IsYearValid(YearType year) { return (year >= kYearMinValue) && (year <= kYearMaxValue); }

    /*!
     * \brief Returns whether `month` is a month of the year.
     *
     * \param[in] month The month to check.
     * \return True if `month` lies from 1 to 12.
     */
    static constexpr bool IsMonthValid(MonthType month) { return (month >= kMonthMinValue) && (month <= kMonthMaxValue); }

    /*!
     * \brief Returns whether `day` is a day the date takes.
     *
     * \param[in] day The day to check.
     * \return True if `day` lies from 1 to 31, whatever the month.
     */
    static constexpr bool IsDayValid(DayType day) { return (day >= kDayMinValue) && (day <= kDayMaxValue); }

    /*!
     * \brief Returns the day of `year`, `month` and `day` in the Rata Die count.
     *
     * The difference of two such days is the number of days between the dates. The date is not
     * checked.
     *
     * \param[in] year The year, in full.
     * \param[in] month The month, 1 to 12.
     * \param[in] day The day of the month.
     * \return The day in the Rata Die count, 1 for 1 January of the year 1.
     */
    static constexpr RataDieDayType GetRataDieDay(YearType year, MonthType month, DayType day) {
        if (month < 3u) { year--, month += 12u; }
        return static_cast<RataDieDayType>(365u)*year+year/4u-year/100u+year/400u+(153u*month-457u)/5u+day-306u;
    }

    /*!
     * \brief Returns the number of days between this date and `day` of `month` of `year`.
     *
     * It counts in either direction: a date before this one gives as many days as one after it.
     * The given date is not checked.
     *
     * \param[in] year The year, in full.
     * \param[in] month The month, 1 to 12.
     * \param[in] day The day of the month.
     * \return The days between the two dates, 0 for the same date.
     */
    RataDieDayType GetPassedDays(YearType year, MonthType month, DayType day) const {
        const RataDieDayType requested_day = GetRataDieDay(year, month, day);
        const RataDieDayType current_day = GetRataDieDay(year_, month_, day_);
        return requested_day >= current_day ? requested_day - current_day : current_day - requested_day;
    }

    /*!
     * \brief Returns the number of days of `month` in `year`.
     *
     * Every year divisible by 4 counts as a leap year, which is right for the years from 2000 to
     * 2099.
     *
     * \param[in] month The month, 1 to 12.
     * \param[in] year The year, in full.
     * \return 28 to 31, or 0 if `month` is not a month from 1 to 12.
     */
    static constexpr DayType GetDaysOfMonth(MonthType month, YearType year) {
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
