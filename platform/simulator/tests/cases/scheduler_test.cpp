// The speed/cycle round trip and the uptime counter.

#include "check.h"
#include "cases.h"

#include "Scheduler.h"
#include "Uptime.h"

/*
 * Only the round trip. That speed 1 maps to cycle 255 and speed 255 to cycle 1 is
 * asserted at compile time in Scheduler.h, so repeating those points here would test
 * the compiler rather than the code.
 */
void TestSchedulerSpeedRoundTrip()
{
    for(unsigned int speed = 0u; speed <= UINT8_MAX; speed++) {
        const byte original_speed = static_cast<byte>(speed);
        const byte cycle = Scheduler::ConvertSpeedToTaskCycle(original_speed);
        Expect(Scheduler::ConvertTaskCycleToSpeed(cycle) == original_speed,
               "speed and task cycle conversion must round-trip");
    }
}

/*
 * The uptime, driven through its own task rather than through the clock: the thing worth
 * proving is the carry, and a test that waited for it would take a day. Counting is what
 * replaced dividing a millisecond counter down, which ran out after 45 days on a field a
 * wall clock outlives - so the boundaries checked here are the three carries and the fact
 * that a day's worth of ticks lands exactly on one day, not near it.
 */
void TestUptimeCountsWithoutWrapping()
{
    Uptime& uptime = Uptime::GetInstance();

    Expect(uptime.GetDays() == 0u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
           "a clock that just started has been up for no time at all");

    for(unsigned int second = 0u; second < 59u; second++) { uptime.Task(); }
    Expect(uptime.GetMinutes() == 0u, "the minute must not turn before the sixtieth second");
    uptime.Task();
    Expect(uptime.GetMinutes() == 1u, "and must turn on it");

    /* On to the hour, from the one minute already counted. */
    for(unsigned int second = 0u; second < 59u * 60u; second++) { uptime.Task(); }
    Expect(uptime.GetHours() == 1u && uptime.GetMinutes() == 0u,
           "sixty minutes must be one hour and no minutes");

    for(unsigned int second = 0u; second < 23u * 60u * 60u; second++) { uptime.Task(); }
    Expect(uptime.GetDays() == 1u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
           "twenty-four hours must be one day exactly");

    // A second day, because a carry that works once can still be a carry that only works
    // from zero.
    for(unsigned int second = 0u; second < 24u * 60u * 60u; second++) { uptime.Task(); }
    Expect(uptime.GetDays() == 2u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
           "and the day after it must follow the same way");
}
