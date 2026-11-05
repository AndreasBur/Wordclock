// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A date and a time of day together.
 */

#ifndef _CLOCK_DATE_TIME_H_
#define _CLOCK_DATE_TIME_H_

#include "StandardTypes.h"
#include "ClockDate.h"
#include "ClockTime.h"

/*!
 * \brief A date and a time of day together.
 *
 * A new one is 1 January 2000, 0:00:00. It keeps a ClockTime and a ClockDate, and each setter
 * checks its field as they do.
 */
class ClockDateTime
{
  public:
    /*! \brief An hour of the day, as ClockTime counts it. */
    using HourType = ClockTime::HourType;
    /*! \brief A minute of the hour, as ClockTime counts it. */
    using MinuteType = ClockTime::MinuteType;
    /*! \brief A second of the minute, as ClockTime counts it. */
    using SecondType = ClockTime::SecondType;

    /*! \brief A year, as ClockDate counts it. */
    using YearType = ClockDate::YearType;
    /*! \brief A month, as ClockDate counts it. */
    using MonthType = ClockDate::MonthType;
    /*! \brief A day of the month, as ClockDate counts it. */
    using DayType = ClockDate::DayType;
    /*! \brief A day of the week, as ClockDate counts it. */
    using WeekdayType = ClockDate::WeekdayType;

  private:
    ClockTime time_;
    ClockDate date_;

  public:
    /*! \brief Makes the date and time 1 January 2000, 0:00:00. */
    constexpr ClockDateTime() {}

    /*! \brief Destroys the date and time. */
    ~ClockDateTime() {}

    /*! \brief Returns the time of day. */
    ClockTime GetTime() const { return time_; }

    /*! \copydoc ClockTime::GetHour() */
    HourType GetTimeHour() const { return time_.GetHour(); }

    /*! \copydoc ClockTime::GetMinute() */
    MinuteType GetTimeMinute() const { return time_.GetMinute(); }

    /*! \copydoc ClockTime::GetSecond() */
    SecondType GetTimeSecond() const { return time_.GetSecond(); }

    /*! \brief Returns the date. */
    ClockDate GetDate() const { return date_; }

    /*! \copydoc ClockDate::GetYear() */
    YearType GetDateYear() const { return date_.GetYear(); }

    /*! \copydoc ClockDate::GetMonth() */
    MonthType GetDateMonth() const { return date_.GetMonth(); }

    /*! \copydoc ClockDate::GetDay() */
    DayType GetDateDay() const { return date_.GetDay(); }

    /*! \copydoc ClockDate::GetWeekday() const */
    WeekdayType GetDateWeekday() const { return date_.GetWeekday(); }

    /*!
     * \brief Sets the time of day.
     *
     * \param[in] time The time.
     */
    void SetTime(ClockTime time) { time_ = time; }

    /*! \copydoc ClockTime::SetHour() */
    StdReturnType SetTimeHour(HourType hour) { return time_.SetHour(hour); }

    /*! \copydoc ClockTime::SetMinute() */
    StdReturnType SetTimeMinute(MinuteType minute) { return time_.SetMinute(minute); }

    /*! \copydoc ClockTime::SetSecond() */
    StdReturnType SetTimeSecond(SecondType second) { return time_.SetSecond(second); }

    /*!
     * \brief Sets the date.
     *
     * \param[in] date The date.
     */
    void SetDate(ClockDate date) { date_ = date; }

    /*! \copydoc ClockDate::SetYear() */
    StdReturnType SetDateYear(YearType year) { return date_.SetYear(year); }

    /*! \copydoc ClockDate::SetMonth() */
    StdReturnType SetDateMonth(MonthType month) { return date_.SetMonth(month); }

    /*! \copydoc ClockDate::SetDay() */
    StdReturnType SetDateDay(DayType day) { return date_.SetDay(day); }

    /*! \copydoc ClockDate::GetDaysOfMonth() */
    static DayType GetDaysOfMonth(MonthType month, YearType year) { return ClockDate::GetDaysOfMonth(month, year); }
};

#endif
