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
    ClockTime time_;
    ClockDate date_;

    // functions

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
