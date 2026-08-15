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
 *      \brief      The time the core reads, kept by the clock chip
 *
 *      \details    Simpler than the ESP32's counterpart, because there is nothing to
 *                  reconcile: that backend has a network time and a chip and has to decide
 *                  which of the two is right. Here the chip is the only source there is, so
 *                  this reads it and hands on what it said.
 *
 *                  A command that sets the time writes through to the chip and keeps the
 *                  value at once, rather than waiting for the next read to bring it back.
 *                  Otherwise the answer to the command would still show the old time.
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
/* Application ticks between two reads of the chip, so once a second. A read is two
   transfers on a bus the light sensor shares, and the display only ever shows minutes -
   asking a hundred times a second would buy nothing. */
#define REAL_TIME_CLOCK_CHIP_READ_INTERVAL              100u

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
    static constexpr byte ChipReadInterval{REAL_TIME_CLOCK_CHIP_READ_INTERVAL};

    ClockDateTime DateTime;
    /* An instance of its own, next to the one the core's temperature side keeps. The chip
       has no state that two readers could disturb, and both reach it through the same bus
       driver. */
    DS3231 Chip;
    /* Zero, so the first tick asks the chip straight away rather than showing the default
       date for a second. */
    byte ChipReadCountdown{0u};

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    StdReturnType readChip();
    void writeChip();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static RealTimeClock& getInstance() {
        static RealTimeClock SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    ClockDateTime getDateTime() const { return DateTime; }
    ClockTime getTime() const { return DateTime.getTime(); }
    ClockDate getDate() const { return DateTime.getDate(); }

    // set methods
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
