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
    static constexpr byte kTaskCycle{UPTIME_TASK_CYCLE};

    static constexpr byte kSecondsPerMinute{60u};
    static constexpr byte kMinutesPerHour{60u};
    static constexpr byte kHoursPerDay{24u};

    /* 179 years, which no clock reaches - but it holds there rather than rolling over,
       because the whole point of this class is that it never reports a small number for a
       long time. */
    static constexpr uint16_t kDaysMaximum{65535u};

    byte seconds_{0u};
    byte minutes_{0u};
    byte hours_{0u};
    uint16_t days_{0u};

    Uptime() { }
    ~Uptime() { }

    /* One place of the count: steps it and says whether it rolled into the next one. The
       carry is the whole of the arithmetic here, so it is worth its own name - task() then
       reads as the three places it applies to. */
    static bool Carry(byte& value, byte limit) {
        value++;
        if(value < limit) { return false; }

        value = 0u;
        return true;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Uptime& GetInstance() {
        static Uptime singleton_instance;
        return singleton_instance;
    }

    // get methods
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    uint16_t GetDays() const { return days_; }
    byte GetHours() const { return hours_; }
    byte GetMinutes() const { return minutes_; }

    // methods
    void Task() {
        if(!Carry(seconds_, kSecondsPerMinute)) { return; }
        if(!Carry(minutes_, kMinutesPerHour)) { return; }
        if(!Carry(hours_, kHoursPerDay)) { return; }

        if(days_ < kDaysMaximum) { days_++; }
    }
};

#endif // _UPTIME_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
