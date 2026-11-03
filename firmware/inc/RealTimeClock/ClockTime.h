/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       ClockTime.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _CLOCK_TIME_H_
#define _CLOCK_TIME_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ClockTime configuration parameter */


/* ClockTime parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class ClockTime
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = byte;
    using MinuteType = byte;
    using SecondType = byte;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
