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
    uint8_t getTimeHour() const { return Time.getHour(); }
    uint8_t getTimeMinute() const { return Time.getMinute(); }
    uint8_t getTimeSecond() const { return Time.getSecond(); }

    // set methods
    void setTime(RealTimeClockTime sTime) { Time = sTime; }
    StdReturnType setTimeHour(uint8_t Hour) { return Time.setHour(Hour); }
    StdReturnType setTimeMinute(uint8_t Minute) { return Time.setMinute(Minute); }
    StdReturnType setTimeSecond(uint8_t Second) { return Time.setSecond(Second); }

    void setDate(RealTimeClockDate sDate) { Date = sDate; }
    StdReturnType setDateYear(uint16_t Year) { return Date.setYear(Year); }
    StdReturnType setDateMonth(uint8_t Month) { return Date.setMonth(Month); }
    StdReturnType setDateDay(uint8_t Day) { return Date.setDay(Day); }

	// methods
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
