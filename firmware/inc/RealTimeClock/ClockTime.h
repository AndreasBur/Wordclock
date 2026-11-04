// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A time of day: hour, minute and second.
 */

#ifndef _CLOCK_TIME_H_
#define _CLOCK_TIME_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief A time of day: hour, minute and second.
 *
 * A new time is 0:00:00. Each setter takes a value in range only, an hour from 0 to 23 and a
 * minute or a second from 0 to 59, and otherwise leaves its field as it was.
 */
class ClockTime
{
  public:
    /*! \brief An hour of the day, 0 to 23. */
    using HourType = byte;
    /*! \brief A minute of the hour, 0 to 59. */
    using MinuteType = byte;
    /*! \brief A second of the minute, 0 to 59. */
    using SecondType = byte;

  private:
    HourType hour_{0u};
    MinuteType minute_{0u};
    SecondType second_{0u};

    static constexpr HourType kHourMinValue{0u};
    static constexpr HourType kHourMaxValue{23u};
    static constexpr MinuteType kMinuteMinValue{0u};
    static constexpr MinuteType kMinuteMaxValue{59u};
    static constexpr SecondType kSecondMinValue{0u};
    static constexpr SecondType kSecondMaxValue{59u};

  public:
    /*! \brief Makes the time 0:00:00. */
    constexpr ClockTime() { }

    /*!
     * \brief Makes the time `hour`:`minute`:`second`.
     *
     * A value out of range leaves its field as a new time has it; the others are taken all the
     * same.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \param[in] second The second, 0 to 59.
     */
    constexpr ClockTime(HourType hour, MinuteType minute, SecondType second)
    {
        SetHour(hour);
        SetMinute(minute);
        SetSecond(second);
    }

    /*! \brief Destroys the time. */
    ~ClockTime() {}

    /*! \brief Returns the hour, 0 to 23. */
    HourType GetHour() const { return hour_; }

    /*! \brief Returns the minute, 0 to 59. */
    MinuteType GetMinute() const { return minute_; }

    /*! \brief Returns the second, 0 to 59. */
    SecondType GetSecond() const { return second_; }

    /*!
     * \brief Sets the hour.
     *
     * \param[in] hour The hour, 0 to 23.
     * \return kEOk, or kENotOk if `hour` is above 23; the hour is then left as it was.
     */
    constexpr StdReturnType SetHour(HourType hour)
    {
        if(IsHourValid(hour)) {
            hour_ = hour;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Sets the minute.
     *
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if `minute` is above 59; the minute is then left as it was.
     */
    constexpr StdReturnType SetMinute(MinuteType minute)
    {
        if(IsMinuteValid(minute)) {
            minute_ = minute;
            return kEOk;
        } else {
            return kENotOk; }
    }

    /*!
     * \brief Sets the second.
     *
     * \param[in] second The second, 0 to 59.
     * \return kEOk, or kENotOk if `second` is above 59; the second is then left as it was.
     */
    constexpr StdReturnType SetSecond(SecondType second)
    {
        if(IsSecondValid(second)) {
            second_ = second;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Returns whether `hour` is an hour of the day.
     *
     * \param[in] hour The hour to check.
     * \return True if `hour` lies from 0 to 23.
     */
    static constexpr bool IsHourValid(HourType hour) { return (hour >= kHourMinValue) && (hour <= kHourMaxValue); }

    /*!
     * \brief Returns whether `minute` is a minute of the hour.
     *
     * \param[in] minute The minute to check.
     * \return True if `minute` lies from 0 to 59.
     */
    static constexpr bool IsMinuteValid(MinuteType minute) { return (minute >= kMinuteMinValue) && (minute <= kMinuteMaxValue); }

    /*!
     * \brief Returns whether `second` is a second of the minute.
     *
     * \param[in] second The second to check.
     * \return True if `second` lies from 0 to 59.
     */
    static constexpr bool IsSecondValid(SecondType second) { return (second >= kSecondMinValue) && (second <= kSecondMaxValue);  }
};

#endif
