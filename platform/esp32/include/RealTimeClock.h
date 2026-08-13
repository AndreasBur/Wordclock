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
 *      \brief      ESP32 real-time clock, backed by the system clock
 *
 *      \details    Mirrors the public API of the simulator's real-time clock, so the
 *                  shared firmware core reads the time the same way on both.
 *
 *                  There is no RTC chip behind this. The ESP32's system clock *is* the
 *                  clock, kept right by SNTP in the background and converted to local
 *                  time by the zone rule the application installs - which is also what
 *                  makes daylight saving somebody else's problem. task() therefore only
 *                  reads, and no seconds are counted here.
 *
 *                  Nothing survives a power cut: the system clock starts at the epoch
 *                  and stays there until the first SNTP answer arrives. isSystemTimeSet()
 *                  is what tells the two states apart.
 *
******************************************************************************************************************************************************/
#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* RealTimeClock configuration parameter */


/* RealTimeClock parameter */


/******************************************************************************************************************************************************
 *  C L A S S   R E A L   T I M E   C L O C K
******************************************************************************************************************************************************/
class RealTimeClock
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

    using YearType = ClockDateTime::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using WeekdayType = ClockDateTime::WeekdayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockDateTime DateTime;

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    static bool readSystemDateTime(ClockDateTime&);
    static StdReturnType writeSystemDateTime(const ClockDateTime&);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static RealTimeClock& getInstance() {
        static RealTimeClock singletonInstance;
        return singletonInstance;
    }

    // get methods
    ClockDateTime getDateTime() const { return DateTime; }
    ClockTime getTime() const { return DateTime.getTime(); }
    ClockDate getDate() const { return DateTime.getDate(); }

    /* Whether the system clock has been set at all, by SNTP or by a command. Until it
       has, task() leaves the date and time at their defaults rather than handing the
       display the epoch, which ClockDate would reject anyway. */
    static bool isSystemTimeSet();

    // set methods
    /* Written through to the system clock, not just stored: task() reads the system
       clock back on the very next tick, so a value kept only here would be overwritten
       within 10 ms and the "time" command would look like it did nothing. */
    void setDateTime(ClockDateTime sDateTime);
    void setTime(ClockTime Time);
    void setDate(ClockDate Date);

    // methods
    void task();
};

#endif // _REAL_TIME_CLOCK_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
