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
/**     \file       ClockDateTime.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _CLOCK_DATE_TIME_H_
#define _CLOCK_DATE_TIME_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDate.h"
#include "ClockTime.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ClockDateTime configuration parameter */


/* ClockDateTime parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   R E A L   T I M E   C L O C K
******************************************************************************************************************************************************/
class ClockDateTime
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockTime::HourType;
    using MinuteType = ClockTime::MinuteType;
    using SecondType = ClockTime::SecondType;

    using YearType = ClockDate::YearType;
    using MonthType = ClockDate::MonthType;
    using DayType = ClockDate::DayType;
    using WeekdayType = ClockDate::WeekdayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockTime Time;
    ClockDate Date;

    // functions

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr ClockDateTime() {}
    ~ClockDateTime() {}

	// get methods
    ClockTime getTime() const { return Time; }
    HourType getTimeHour() const { return Time.getHour(); }
    MinuteType getTimeMinute() const { return Time.getMinute(); }
    SecondType getTimeSecond() const { return Time.getSecond(); }

    ClockDate getDate() const { return Date; }
    YearType getDateYear() const { return Date.getYear(); }
    MonthType getDateMonth() const { return Date.getMonth(); }
    DayType getDateDay() const { return Date.getDay(); }
    WeekdayType getDateWeekday() const { return Date.getWeekday(); }

    // set methods
    void setTime(ClockTime sTime) { Time = sTime; }
    StdReturnType setTimeHour(HourType Hour) { return Time.setHour(Hour); }
    StdReturnType setTimeMinute(MinuteType Minute) { return Time.setMinute(Minute); }
    StdReturnType setTimeSecond(SecondType Second) { return Time.setSecond(Second); }

    void setDate(ClockDate sDate) { Date = sDate; }
    StdReturnType setDateYear(YearType Year) { return Date.setYear(Year); }
    StdReturnType setDateMonth(MonthType Month) { return Date.setMonth(Month); }
    StdReturnType setDateDay(DayType Day) { return Date.setDay(Day); }

	// methods
    DayType getDaysOfMonth(MonthType Month, YearType Year) { return Date.getDaysOfMonth(Month, Year); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
