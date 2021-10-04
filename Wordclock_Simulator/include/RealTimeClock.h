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
/**     \file       RealTimeClock.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "RealTimeClockTime.h"
#include "RealTimeClockDate.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* RealTimeClock configuration parameter */


/* RealTimeClock parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   R E A L   T I M E   C L O C K
******************************************************************************************************************************************************/
class RealTimeClock
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = RealTimeClockTime::HourType;
    using MinuteType = RealTimeClockTime::MinuteType;
    using SecondType = RealTimeClockTime::SecondType;

    using YearType = RealTimeClockDate::YearType;
    using MonthType = RealTimeClockDate::MonthType;
    using DayType = RealTimeClockDate::DayType;
    using WeekdayType = RealTimeClockDate::WeekdayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    RealTimeClockTime Time;
    RealTimeClockDate Date;

    // functions
    constexpr RealTimeClock() {}
    ~RealTimeClock() {}

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static RealTimeClock& getInstance() {
        static RealTimeClock singletonInstance;
        return singletonInstance;
    }

	// get methods
    RealTimeClockTime getTime() const { return Time; }
    HourType getTimeHour() const { return Time.getHour(); }
    MinuteType getTimeMinute() const { return Time.getMinute(); }
    SecondType getTimeSecond() const { return Time.getSecond(); }

    RealTimeClockDate getDate() const { return Date; }
    YearType getDateYear() const { return Date.getYear(); }
    MonthType getDateMonth() const { return Date.getMonth(); }
    DayType getDateDay() const { return Date.getDay(); }
    WeekdayType getDateWeekday() const { return Date.getWeekday(); }

    // set methods
    void setTime(RealTimeClockTime sTime) { Time = sTime; }
    StdReturnType setTimeHour(HourType Hour) { return Time.setHour(Hour); }
    StdReturnType setTimeMinute(MinuteType Minute) { return Time.setMinute(Minute); }
    StdReturnType setTimeSecond(SecondType Second) { return Time.setSecond(Second); }

    void setDate(RealTimeClockDate sDate) { Date = sDate; }
    StdReturnType setDateYear(YearType Year) { return Date.setYear(Year); }
    StdReturnType setDateMonth(MonthType Month) { return Date.setMonth(Month); }
    StdReturnType setDateDay(DayType Day) { return Date.setDay(Day); }

	// methods
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
