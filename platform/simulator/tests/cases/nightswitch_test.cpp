// The window, the midnight wrap, the strip's supply, and who wins between the timer and a hand.

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "NightSwitch.h"
#include "Pixels.h"
#include "Power.h"
#include "RealTimeClock.h"

namespace {
// One tick of the night switch at a given time. Its own task rather than the scheduler's,
// because what is being checked is the edge and not the cycle counting that reaches it.
void TickAt(byte hour, byte minute)
{
    SetTime(hour, minute, 0u);
    NightSwitch::GetInstance().Task();
}

/*
 * A tick of both tasks, the way the scheduler runs them, with the render in between that
 * Power waits for. The cases about the supply need it: the night switch only asks, and a
 * case that ran the one task would watch a rail that never moves.
 */
void TickBothAt(byte hour, byte minute)
{
    SetTime(hour, minute, 0u);
    NightSwitch::GetInstance().Task();
    Pixels::GetInstance().ClearDirty();
    Power::GetInstance().Task();
}

// A lit clock face with the supply up and the sequence idle, which is where every case about
// the supply starts and what it has to leave behind for the next one.
void StartFromRunningClock()
{
    Display& display = Display::GetInstance();

    display.ResumeOutput();
    display.Enable();
    display.SetBrightness(120u);
    Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
    Expect(display.Show() == kEOk, "the clock face must reach the strip");
    Pixels::GetInstance().ClearDirty();
}

/* The window this module was given a default for, and the one the cases below all use. */
void UseNightWindow(byte night_brightness)
{
    NightSwitch& night_switch = NightSwitch::GetInstance();

    night_switch.SetStartHour(23u);
    night_switch.SetStartMinute(0u);
    night_switch.SetEndHour(6u);
    night_switch.SetEndMinute(0u);
    night_switch.SetNightBrightness(night_brightness);
    night_switch.SetIsActive(true);
}

} // namespace

// The window itself, including the one that crosses midnight - which is the ordinary case
// for a clock, and the only one where the test reads the other way round.
void TestNightWindowIncludesMidnight()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();

    night_switch.SetStartHour(23u);
    night_switch.SetStartMinute(0u);
    night_switch.SetEndHour(6u);
    night_switch.SetEndMinute(30u);

    SetTime(22u, 59u, 0u); Expect(!night_switch.IsNight(), "a minute before the start is still day");
    SetTime(23u, 0u, 0u);  Expect(night_switch.IsNight(),  "the start minute itself is night");
    SetTime(2u, 0u, 0u);   Expect(night_switch.IsNight(),  "the small hours are night");
    SetTime(6u, 29u, 0u);  Expect(night_switch.IsNight(),  "a minute before the end is still night");
    SetTime(6u, 30u, 0u);  Expect(!night_switch.IsNight(), "the end minute itself is day");
    SetTime(12u, 0u, 0u);  Expect(!night_switch.IsNight(), "midday is day");

    /* A window inside one day, which must not be read as its own inverse. */
    night_switch.SetStartHour(1u);
    night_switch.SetEndHour(5u);
    night_switch.SetStartMinute(0u);
    night_switch.SetEndMinute(0u);
    SetTime(3u, 0u, 0u);   Expect(night_switch.IsNight(),  "a window inside one day holds in the middle");
    SetTime(23u, 0u, 0u);  Expect(!night_switch.IsNight(), "and not outside it");

    /* Both ends the same minute is nothing configured, not a whole day. */
    night_switch.SetEndHour(1u);
    SetTime(1u, 0u, 0u);   Expect(!night_switch.IsNight(), "an empty window is never night");
}

// What the window does to the display, and what it does not do to the brightness setting -
// which is the whole reason the night dimming is its own level and not the fade.
void TestNightSwitchDimsWithoutLosingTheSetting()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();
    Display& display = Display::GetInstance();

    display.Enable();
    display.SetBrightness(120u);
    Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
    Expect(display.Show() == kEOk, "the clock face must reach the strip");

    night_switch.SetStartHour(23u);
    night_switch.SetStartMinute(0u);
    night_switch.SetEndHour(6u);
    night_switch.SetEndMinute(0u);
    night_switch.SetNightBrightness(0u);
    night_switch.SetIsActive(true);

    TickAt(22u, 0u);
    Expect(IsAnyOutputPixelLit(), "before the window the display is on");

    TickAt(23u, 0u);
    Expect(!IsAnyOutputPixelLit(), "a brightness of zero switches the display off for the night");

    TickAt(6u, 0u);
    Expect(IsAnyOutputPixelLit(), "morning gives it back");
    Expect(display.GetBrightness() == 120u, "and gives back the brightness that was set");

    /*
     * Dimming rather than switching off: still lit, still not the day brightness, and the
     * setting untouched - Persistence reads that setting, so a night that wrote it would
     * be a night that changed the clock for good.
     */
    night_switch.SetNightBrightness(16u);
    TickAt(23u, 0u);
    Expect(IsAnyOutputPixelLit(), "a dimmed night is still lit");
    Expect(display.GetBrightness() == 120u, "and leaves the brightness setting alone");
    Expect(display.GetBrightnessNightLevel() == 16u, "the dimming is the night level");

    TickAt(6u, 0u);
    Expect(display.GetBrightnessNightLevel() == 255u, "which morning clears again");

    night_switch.SetIsActive(false);
}

// The edge, not the state. A clock switched on by hand in the small hours has to stay on,
// or a timer is something people switch off after one night.
void TestNightSwitchActsOnTheCrossingOnly()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();
    Display& display = Display::GetInstance();

    display.Enable();
    display.SetBrightness(120u);
    Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
    Expect(display.Show() == kEOk, "the clock face must reach the strip");

    night_switch.SetStartHour(23u);
    night_switch.SetStartMinute(0u);
    night_switch.SetEndHour(6u);
    night_switch.SetEndMinute(0u);
    night_switch.SetNightBrightness(0u);
    night_switch.SetIsActive(true);

    TickAt(22u, 0u);
    TickAt(23u, 0u);
    Expect(!IsAnyOutputPixelLit(), "the window switches the display off");

    display.Enable();
    Expect(IsAnyOutputPixelLit(), "a hand switches it back on");

    TickAt(2u, 0u);
    TickAt(3u, 0u);
    Expect(IsAnyOutputPixelLit(), "and the timer leaves it on until the window ends");

    TickAt(6u, 0u);
    Expect(IsAnyOutputPixelLit(), "the end of the window is not a reason to switch it off");

    // And switching the whole timer off hands the display back rather than leaving it
    // wherever the last edge put it.
    TickAt(23u, 0u);
    Expect(!IsAnyOutputPixelLit(), "the next night switches it off again");
    night_switch.SetIsActive(false);
    Expect(IsAnyOutputPixelLit(), "switching the timer off gives the display back");
}

// The whole return on the switch: a night the display spends off is a night the strip need
// not be powered for, and that is the one state where cutting the rail costs nothing.
void TestNightWithNoBrightnessCutsTheSupply()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();
    Display& display = Display::GetInstance();
    Power& power = Power::GetInstance();

    Expect(Power::IsSwitchFitted(), "the simulator stands in for a board that has the switch");

    StartFromRunningClock();
    UseNightWindow(0u);

    TickBothAt(22u, 0u);
    Expect(power.IsSupplyOn(), "before the window the strip is powered");

    TickBothAt(23u, 0u);
    Expect(!IsAnyOutputPixelLit(), "the window darkens the display");
    Expect(!power.IsSupplyOn(), "and takes the supply with it, which is what saves the current");
    Expect(display.IsOutputSuspended(), "with the data line gated behind it");

    TickBothAt(2u, 0u);
    Expect(!power.IsSupplyOn(), "and the rail stays down for the rest of the night");

    TickBothAt(6u, 0u);
    Expect(power.IsSupplyOn(), "morning gives the supply back");
    Expect(!display.IsOutputSuspended(), "and releases the data line");
    Expect(IsAnyOutputPixelLit(), "so the clock face reaches the strip again");
    Expect(display.GetBrightness() == 120u, "with the brightness that was set");

    // A dimmed night is a lit strip, so there is nothing to save and the rail stays up -
    // cutting it there would take the dimming that was asked for with it.
    night_switch.SetNightBrightness(16u);
    TickBothAt(23u, 0u);
    Expect(power.IsSupplyOn(), "a dimmed night keeps the supply, because the strip is still lit");
    Expect(IsAnyOutputPixelLit(), "and stays lit on it");

    TickBothAt(6u, 0u);
    night_switch.SetIsActive(false);
    StartFromRunningClock();
}

/*
 * The one thing the crossing rule cannot answer on its own. Switching the clock on by hand
 * reaches the display and not the rail, so without this the switch is a switch that does
 * nothing - and a wall that stays dark is read as a fault, not as a timer working.
 */
void TestDisplaySwitchedOnAtNightGetsItsSupplyBack()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();
    Display& display = Display::GetInstance();
    Power& power = Power::GetInstance();

    StartFromRunningClock();
    UseNightWindow(0u);

    TickBothAt(22u, 0u);
    TickBothAt(23u, 0u);
    Expect(!power.IsSupplyOn() && display.IsOutputSuspended(), "the night has the strip off and gated");

    display.Enable();
    Expect(display.IsEnabled(), "a hand switches the display back on");
    Expect(!power.IsSupplyOn(), "which is all it can do - the supply is not the display's to ask for");

    TickBothAt(2u, 0u);
    Expect(power.IsSupplyOn(), "so the next tick hands the rail back");
    Expect(!display.IsOutputSuspended(), "and releases the line");
    Expect(IsAnyOutputPixelLit(), "which is what makes that hand mean something at two in the morning");

    TickBothAt(3u, 0u);
    Expect(power.IsSupplyOn(), "and nothing takes it away again before the window ends");
    Expect(IsAnyOutputPixelLit(), "with the display left where the hand put it");

    // A hand that switches it off again keeps its rail until the next edge, for the same
    // reason: a tick that undid what somebody just did is the behaviour this module avoids.
    display.Disable();
    TickBothAt(4u, 0u);
    Expect(power.IsSupplyOn(), "a display switched off by hand does not cut the supply");

    TickBothAt(6u, 0u);
    Expect(IsAnyOutputPixelLit(), "and morning gives the display back as it always did");

    night_switch.SetIsActive(false);
    StartFromRunningClock();
}
