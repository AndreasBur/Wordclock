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

/* ClockTime configuration parameter */

/* ClockTime parameter */

class ClockTime
{
  public:
    using HourType = byte;
    using MinuteType = byte;
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

    // functions

  public:
    constexpr ClockTime() { }
    constexpr ClockTime(HourType hour, MinuteType minute, SecondType second)
    {
        SetHour(hour);
        SetMinute(minute);
        SetSecond(second);
    }
    ~ClockTime() {}

	// get methods
    HourType GetHour() const { return hour_; }
    MinuteType GetMinute() const { return minute_; }
    SecondType GetSecond() const { return second_; }

	// set methods
    constexpr StdReturnType SetHour(HourType hour)
    {
        if(IsHourValid(hour)) {
            hour_ = hour;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    constexpr StdReturnType SetMinute(MinuteType minute)
    {
        if(IsMinuteValid(minute)) {
            minute_ = minute;
            return kEOk;
        } else {
            return kENotOk; }
    }
    constexpr StdReturnType SetSecond(SecondType second)
    {
        if(IsSecondValid(second)) {
            second_ = second;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

	// methods
    static constexpr bool IsHourValid(HourType hour) { return (hour >= kHourMinValue) && (hour <= kHourMaxValue); }
    static constexpr bool IsMinuteValid(MinuteType minute) { return (minute >= kMinuteMinValue) && (minute <= kMinuteMaxValue); }
    static constexpr bool IsSecondValid(SecondType second) { return (second >= kSecondMinValue) && (second <= kSecondMaxValue);  }
};

#endif
