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
#include "DS3231.h"

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
    /* No hour, so the first pass with a system time writes the chip. A byte the hours
       cannot reach, rather than a second flag beside it. */
    static constexpr byte NoHourWritten{0xFFu};

    /* Ticks between two attempts to read the chip, so once a second. This runs from the
       application's tick rather than through the scheduler, and a read is two transfers -
       trying on every tick would put a hundred of them a second on a bus the light sensor
       shares, for a value that only has to arrive before the display is worth looking at. */
    static constexpr byte ChipReadInterval{100u};

    ClockDateTime DateTime;

    /* An instance of its own, next to the one Temperature keeps. The chip has no state
       that two readers could disturb, both open the same bus - which Wire.begin() allows
       twice - and sharing one would mean either a singleton the simulator's stand-in would
       have to grow too, or the time-keeping side reaching through the core's Temperature,
       which knows nothing about time. */
    DS3231 Chip;
    byte HourWrittenToChip{NoHourWritten};
    /* Zero, so the first tick without a system time asks the chip straight away. */
    byte ChipReadCountdown{0u};

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    static bool readSystemDateTime(ClockDateTime&);
    static StdReturnType writeSystemDateTime(const ClockDateTime&);

    StdReturnType readChip();
    void writeChip();

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
