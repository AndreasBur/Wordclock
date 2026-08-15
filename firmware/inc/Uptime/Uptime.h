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
/**     \file       Uptime.h
 *      \brief      How long the clock has been running, counted rather than measured
 *
 *      \details    This used to be a platform's job: each backend divided its millisecond
 *                  counter down and the status command reported a uint16 of minutes. Both
 *                  halves of that wrapped. The field ran out after 45 days 12 hours, and
 *                  underneath it the 32-bit millisecond counters on the ESP32 and the AVR
 *                  come round after 49 days 17 hours - so a clock that had stood for seven
 *                  weeks reported nine minutes, which is the sort of wrong answer a reader
 *                  believes.
 *
 *                  Counting instead of asking is what fixes it. The source is the
 *                  scheduler's tick, which does not wrap, and what is kept is the answer
 *                  itself rather than a number to divide: seconds into minutes into hours
 *                  into days. Days are a uint16, so the count reaches 179 years, and it
 *                  saturates rather than rolling over even there.
 *
 *                  In the core and not in a platform, because none of it is hardware -
 *                  which also takes getUptimeInMinutes() out of the platform contract that
 *                  three backends each had to implement and each got wrong the same way.
 *
******************************************************************************************************************************************************/
#ifndef _UPTIME_H_
#define _UPTIME_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Uptime configuration parameter */
/* In scheduler ticks, so 100 * 10 ms = 1 s. The scheduler counts cycles in a byte, which
   caps this at 255 - a second is the coarsest unit that still carries a minute exactly. */
#define UPTIME_TASK_CYCLE                               100u

/******************************************************************************************************************************************************
 *  C L A S S   U P T I M E
******************************************************************************************************************************************************/
class Uptime
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{UPTIME_TASK_CYCLE};

    static constexpr byte SecondsPerMinute{60u};
    static constexpr byte MinutesPerHour{60u};
    static constexpr byte HoursPerDay{24u};

    /* 179 years, which no clock reaches - but it holds there rather than rolling over,
       because the whole point of this class is that it never reports a small number for a
       long time. */
    static constexpr uint16_t DaysMaximum{65535u};

    byte Seconds{0u};
    byte Minutes{0u};
    byte Hours{0u};
    uint16_t Days{0u};

    Uptime() { }
    ~Uptime() { }

    /* One place of the count: steps it and says whether it rolled into the next one. The
       carry is the whole of the arithmetic here, so it is worth its own name - task() then
       reads as the three places it applies to. */
    static bool carry(byte& Value, byte Limit) {
        Value++;
        if(Value < Limit) { return false; }

        Value = 0u;
        return true;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Uptime& getInstance() {
        static Uptime SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    uint16_t getDays() const { return Days; }
    byte getHours() const { return Hours; }
    byte getMinutes() const { return Minutes; }

    // methods
    void task() {
        if(!carry(Seconds, SecondsPerMinute)) { return; }
        if(!carry(Minutes, MinutesPerHour)) { return; }
        if(!carry(Hours, HoursPerDay)) { return; }

        if(Days < DaysMaximum) { Days++; }
    }
};

#endif // _UPTIME_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
