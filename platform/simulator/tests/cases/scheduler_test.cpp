/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       scheduler_test.cpp
 *      \brief      The speed/cycle round trip and the uptime counter
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Scheduler.h"
#include "Uptime.h"

/* Only the round trip. That speed 1 maps to cycle 255 and speed 255 to cycle 1 is
   asserted at compile time in Scheduler.h, so repeating those points here would test
   the compiler rather than the code. */
void testSchedulerSpeedRoundTrip()
{
    for(unsigned int speed = 0u; speed <= UINT8_MAX; speed++) {
        const byte originalSpeed = static_cast<byte>(speed);
        const byte cycle = Scheduler::convertSpeedToTaskCycle(originalSpeed);
        expect(Scheduler::convertTaskCycleToSpeed(cycle) == originalSpeed,
               "speed and task cycle conversion must round-trip");
    }
}

/* The uptime, driven through its own task rather than through the clock: the thing worth
   proving is the carry, and a test that waited for it would take a day. Counting is what
   replaced dividing a millisecond counter down, which ran out after 45 days on a field a
   wall clock outlives - so the boundaries checked here are the three carries and the fact
   that a day's worth of ticks lands exactly on one day, not near it. */
void testUptimeCountsWithoutWrapping()
{
    Uptime& uptime = Uptime::getInstance();

    expect(uptime.getDays() == 0u && uptime.getHours() == 0u && uptime.getMinutes() == 0u,
           "a clock that just started has been up for no time at all");

    for(unsigned int second = 0u; second < 59u; second++) { uptime.task(); }
    expect(uptime.getMinutes() == 0u, "the minute must not turn before the sixtieth second");
    uptime.task();
    expect(uptime.getMinutes() == 1u, "and must turn on it");

    /* On to the hour, from the one minute already counted. */
    for(unsigned int second = 0u; second < 59u * 60u; second++) { uptime.task(); }
    expect(uptime.getHours() == 1u && uptime.getMinutes() == 0u,
           "sixty minutes must be one hour and no minutes");

    for(unsigned int second = 0u; second < 23u * 60u * 60u; second++) { uptime.task(); }
    expect(uptime.getDays() == 1u && uptime.getHours() == 0u && uptime.getMinutes() == 0u,
           "twenty-four hours must be one day exactly");

    /* A second day, because a carry that works once can still be a carry that only works
       from zero. */
    for(unsigned int second = 0u; second < 24u * 60u * 60u; second++) { uptime.task(); }
    expect(uptime.getDays() == 2u && uptime.getHours() == 0u && uptime.getMinutes() == 0u,
           "and the day after it must follow the same way");
}
