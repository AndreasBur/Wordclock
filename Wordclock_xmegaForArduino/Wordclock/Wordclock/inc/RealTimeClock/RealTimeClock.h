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

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    RealTimeClockDate Date;
    RealTimeClockTime Time;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    RealTimeClock() {}
    ~RealTimeClock() {}

	// get methods
    HourType getTimeHour() const { return Time.getHour(); }
    MinuteType getTimeMinute() const { return Time.getMinute(); }
    SecondType getTimeSecond() const { return Time.getSecond(); }

    YearType getDateYear() const { return Date.getYear(); }
    MonthType getDateMonth() const { return Date.getMonth(); }
    DayType getDateDay() const { return Date.getDay(); }

	// set methods
    //void setTime(Time sTime) { Time = sTime; }
    //void setTimeHour(uint8_t Hour) { Time.Hour = Hour; }
    //void setTimeMinute(uint8_t Minute) { Time.Minute = Minute; }
    //void setTimeSecond(uint8_t Second) { Time.Second = Second; }
//
    //void setDate(Date sDate) { Date = sDate; }
    //void setDateYear(uint16_t Year) { Date.Year = Year; }
    //void setDateMonth(uint8_t Month) { Date.Month = Month; }
	// methods
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/