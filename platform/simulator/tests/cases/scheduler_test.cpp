// The speed/cycle round trip, the uptime counter, and when the scheduler runs a task.

#include "check.h"
#include "cases.h"

#include "ColorCycle.h"
#include "Display.h"
#include "NightSwitch.h"
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

namespace {
/*
 * How often the colour cycle's task ran since the hue was `from`: it steps the hue by one per
 * run and does nothing else a case could count, which makes it the scheduler's tally here. The
 * byte wraps with the hue, so the difference stays right across 255.
 */
byte RunsSince(byte from)
{
    return static_cast<byte>(ColorCycle::GetInstance().GetHue() - from);
}

void Tick(Scheduler& scheduler, unsigned ticks)
{
    for(unsigned tick = 0u; tick < ticks; tick++) { scheduler.Task(); }
}

/*
 * A lit face, the night switch out of the way and the colour cycle running at `cycle`: the
 * cycle stands still on a dark display, and a night starting in the middle of a case would
 * darken it. Every module's task runs on these ticks, the persistence's included, which writes
 * the store the way the persistence cases after these do.
 */
void StartColourCycle(byte cycle)
{
    NightSwitch::GetInstance().SetIsActive(false);
    Display::GetInstance().Enable();
    SetTime(10u, 0u, 0u);
    DrawClockFace(10u, 0u);
    ColorCycle::GetInstance().SetTaskCycle(cycle);
    ColorCycle::GetInstance().SetIsActive(true);
}

void StopColourCycle(byte task_cycle_before)
{
    ColorCycle::GetInstance().SetIsActive(false);
    ColorCycle::GetInstance().SetTaskCycle(task_cycle_before);
    Display::GetInstance().Clear();
}
}

void TestSchedulerRunsATaskFirstThenEveryCycle()
{
    const byte task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
    Scheduler scheduler;

    StartColourCycle(5u);
    const byte hue = ColorCycle::GetInstance().GetHue();

    Tick(scheduler, 1u);
    Expect(RunsSince(hue) == 1u, "every task is due on the scheduler's first tick");
    Tick(scheduler, 4u);
    Expect(RunsSince(hue) == 1u, "and not again before its cycle has passed");
    Tick(scheduler, 1u);
    Expect(RunsSince(hue) == 2u, "but on the fifth tick after, with a cycle of five");
    Tick(scheduler, 10u);
    Expect(RunsSince(hue) == 4u, "and on every fifth after that");

    StopColourCycle(task_cycle_before);
}

/*
 * The cycle is read when the task runs, not on every tick, so a period already begun is
 * finished at the old length - what the slider of a front end changes is the next period.
 */
void TestSchedulerTakesANewCycleOnTheNextRun()
{
    const byte task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
    Scheduler scheduler;

    StartColourCycle(5u);
    const byte hue = ColorCycle::GetInstance().GetHue();
    Tick(scheduler, 1u);

    ColorCycle::GetInstance().SetTaskCycle(2u);
    Tick(scheduler, 4u);
    Expect(RunsSince(hue) == 1u, "a shorter cycle does not cut short the period already begun");
    Tick(scheduler, 1u);
    Expect(RunsSince(hue) == 2u, "which ends at its old length");
    Tick(scheduler, 4u);
    Expect(RunsSince(hue) == 4u, "and the new cycle counts from there");

    StopColourCycle(task_cycle_before);
}

void TestSchedulerCycleZeroSwitchesATaskOff()
{
    const byte task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
    Scheduler scheduler;

    StartColourCycle(5u);
    const byte hue = ColorCycle::GetInstance().GetHue();
    Tick(scheduler, 1u);

    ColorCycle::GetInstance().SetTaskCycle(0u);
    Tick(scheduler, 20u);
    Expect(RunsSince(hue) == 1u, "a cycle of zero does not run the task at all");

    // Two ticks into the old period of five when it was switched off. Kept, the remainder
    // would hold the task back three more ticks.
    ColorCycle::GetInstance().SetTaskCycle(5u);
    Tick(scheduler, 1u);
    Expect(RunsSince(hue) == 2u, "and switched back on, the task runs on the very next tick");

    StopColourCycle(task_cycle_before);
}
