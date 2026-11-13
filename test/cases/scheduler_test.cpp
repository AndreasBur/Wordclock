// The speed/cycle round trip, the uptime counter, and when and in which order the scheduler runs a
// task.

#include "Scheduler.h"

#include "Animations.h"
#include "BH1750.h"
#include "ColorCycle.h"
#include "Display.h"
#include "Illuminance.h"
#include "NightSwitch.h"
#include "Persistence.h"
#include "Pixels.h"
#include "Power.h"
#include "Uptime.h"
#include "cases.h"
#include "check.h"
#include "sim/SerialShim.h"

/*
 * Only the round trip. That speed 1 maps to cycle 255 and speed 255 to cycle 1 is
 * asserted at compile time in Scheduler.h, so repeating those points here would test
 * the compiler rather than the code.
 */
void TestSchedulerSpeedRoundTrip() {
  for (unsigned int speed = 0u; speed <= UINT8_MAX; speed++) {
    byte const original_speed = static_cast<byte>(speed);
    byte const cycle = Scheduler::ConvertSpeedToTaskCycle(original_speed);
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
void TestUptimeCountsWithoutWrapping() {
  Uptime& uptime = Uptime::GetInstance();

  Expect(uptime.GetDays() == 0u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
         "a clock that just started has been up for no time at all");

  for (unsigned int second = 0u; second < 59u; second++) {
    uptime.Task();
  }
  Expect(uptime.GetMinutes() == 0u, "the minute must not turn before the sixtieth second");
  uptime.Task();
  Expect(uptime.GetMinutes() == 1u, "and must turn on it");

  /* On to the hour, from the one minute already counted. */
  for (unsigned int second = 0u; second < 59u * 60u; second++) {
    uptime.Task();
  }
  Expect(uptime.GetHours() == 1u && uptime.GetMinutes() == 0u,
         "sixty minutes must be one hour and no minutes");

  for (unsigned int second = 0u; second < 23u * 60u * 60u; second++) {
    uptime.Task();
  }
  Expect(uptime.GetDays() == 1u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
         "twenty-four hours must be one day exactly");

  // A second day, because a carry that works once can still be a carry that only works
  // from zero.
  for (unsigned int second = 0u; second < 24u * 60u * 60u; second++) {
    uptime.Task();
  }
  Expect(uptime.GetDays() == 2u && uptime.GetHours() == 0u && uptime.GetMinutes() == 0u,
         "and the day after it must follow the same way");
}

namespace {
/*
 * How often the colour cycle's task ran since the hue was `from`: it steps the hue by one per
 * run and does nothing else a case could count, which makes it the scheduler's tally here. The
 * byte wraps with the hue, so the difference stays right across 255.
 */
byte RunsSince(byte from) { return static_cast<byte>(ColorCycle::GetInstance().GetHue() - from); }

void Tick(Scheduler& scheduler, unsigned ticks) {
  for (unsigned tick = 0u; tick < ticks; tick++) {
    scheduler.Task();
  }
}

/*
 * A lit face, the night switch out of the way and the colour cycle running at `cycle`: the
 * cycle stands still on a dark display, and a night starting in the middle of a case would
 * darken it. Every module's task runs on these ticks, the persistence's included, which writes
 * the store the way the persistence cases after these do.
 */
void StartColourCycle(byte cycle) {
  NightSwitch::GetInstance().SetIsActive(false);
  Display::GetInstance().Enable();
  SetTime(10u, 0u, 0u);
  DrawClockFace(10u, 0u);
  ColorCycle::GetInstance().SetTaskCycle(cycle);
  ColorCycle::GetInstance().SetIsActive(true);
}

void StopColourCycle(byte task_cycle_before) {
  ColorCycle::GetInstance().SetIsActive(false);
  ColorCycle::GetInstance().SetTaskCycle(task_cycle_before);
  Display::GetInstance().Clear();
}
}  // namespace

void TestSchedulerRunsATaskFirstThenEveryCycle() {
  byte const task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
  Scheduler scheduler;

  StartColourCycle(5u);
  byte const hue = ColorCycle::GetInstance().GetHue();

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
void TestSchedulerTakesANewCycleOnTheNextRun() {
  byte const task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
  Scheduler scheduler;

  StartColourCycle(5u);
  byte const hue = ColorCycle::GetInstance().GetHue();
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

void TestSchedulerCycleZeroSwitchesATaskOff() {
  byte const task_cycle_before = ColorCycle::GetInstance().GetTaskCycle();
  Scheduler scheduler;

  StartColourCycle(5u);
  byte const hue = ColorCycle::GetInstance().GetHue();
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

/*
 * The order of the tasks within a tick, which TriggerTasks() gives a reason for at each place it
 * matters. Each case below is a tick in which another order shows: a fresh scheduler, because
 * every task is due on its first tick, so a single Task() runs every one of them in that order
 * and nothing but the order decides which of them sees what the other did.
 */
namespace {
void TickAll() {
  Scheduler scheduler;
  scheduler.Task();
}
}  // namespace

// The sensor before the display manager, so the automatic works on this tick's reading: a
// room that went dark is dimmed for on the tick it is measured, not on the next.
void TestSchedulerReadsTheSensorBeforeTheBrightness() {
  Display& display = Display::GetInstance();
  Illuminance& illuminance = Illuminance::GetInstance();
  byte const brightness_before = display.GetBrightness();
  bool const automatic_before = display.GetBrightnessUseAutomatic();
  auto const calibration_before = illuminance.GetCalibrationValues();
  BH1750::IlluminanceType const illuminance_before = BH1750::GetSimulatedIlluminance();

  NightSwitch::GetInstance().SetIsActive(false);
  display.Enable();
  display.SetBrightness(255u);
  display.SetBrightnessUseAutomatic(true);
  illuminance.SetCalibrationValuesMinValue(10u);
  illuminance.SetCalibrationValuesMaxValue(1000u);
  // Read off the letters of a clock face: here the brightness is folded into the colour in
  // the buffer, as DISPLAY_USE_PIXELS_DIMMING has it, rather than set on the strip.
  SetTime(10u, 0u, 0u);
  DrawClockFace(10u, 0u);
  BH1750::SetSimulatedIlluminance(1000u);
  TickAll();
  PixelBufferType const bright = ReadPixels();
  Expect(IsAnyPixelLit(bright), "the clock face must be lit to compare");

  BH1750::SetSimulatedIlluminance(10u);
  TickAll();
  PixelBufferType const dimmed_at_once = ReadPixels();
  TickAll();
  Expect(!ArePixelsEqual(dimmed_at_once, bright) && ArePixelsEqual(dimmed_at_once, ReadPixels()),
         "a room gone dark must be dimmed for on the tick it is measured, not one later");

  display.SetBrightnessUseAutomatic(automatic_before);
  display.SetBrightness(brightness_before);
  illuminance.SetCalibrationValuesMinValue(calibration_before.min_value);
  illuminance.SetCalibrationValuesMaxValue(calibration_before.max_value);
  BH1750::SetSimulatedIlluminance(illuminance_before);
  TickAll();
}

// The colour cycle after the night switch: on the tick the night switches the display off, a
// cycle stepping after it would mark a buffer for a display that is meant to be dark.
void TestSchedulerLetsTheNightGoBeforeTheColourCycle() {
  NightSwitch& night_switch = NightSwitch::GetInstance();
  ColorCycle& cycle = ColorCycle::GetInstance();
  Display& display = Display::GetInstance();
  byte const task_cycle_before = cycle.GetTaskCycle();
  NightSwitch::HourType const start_hour_before = night_switch.GetStartHour();
  NightSwitch::MinuteType const start_minute_before = night_switch.GetStartMinute();
  NightSwitch::HourType const end_hour_before = night_switch.GetEndHour();
  NightSwitch::MinuteType const end_minute_before = night_switch.GetEndMinute();
  byte const night_brightness_before = night_switch.GetNightBrightness();

  Animations::GetInstance().SetAnimation(Animations::AnimationIdType::kNone);
  Animations::GetInstance().SetMode(Animations::ModeType::kFixed);
  display.ResumeOutput();
  display.Enable();
  night_switch.SetStartHour(22u);
  night_switch.SetStartMinute(0u);
  night_switch.SetEndHour(6u);
  night_switch.SetEndMinute(0u);
  night_switch.SetNightBrightness(0u);  // the night switches the display off
  cycle.SetTaskCycle(1u);
  cycle.SetIsActive(true);

  SetTime(21u, 59u, 0u);
  night_switch.SetIsActive(true);
  TickAll();
  Expect(display.IsEnabled(), "the minute before the night must leave the display on");
  byte const hue = cycle.GetHue();

  SetTime(22u, 0u, 0u);
  TickAll();
  Expect(!display.IsEnabled(), "the night must switch the display off on its first tick");
  Expect(cycle.GetHue() == hue, "and the colour cycle must not step on that tick");

  cycle.SetIsActive(false);
  cycle.SetTaskCycle(task_cycle_before);
  night_switch.SetIsActive(false);
  night_switch.SetStartHour(start_hour_before);
  night_switch.SetStartMinute(start_minute_before);
  night_switch.SetEndHour(end_hour_before);
  night_switch.SetEndMinute(end_minute_before);
  night_switch.SetNightBrightness(night_brightness_before);
  Power::GetInstance().SwitchSupplyOn();
  for (byte tick = 0u; tick < 4u; tick++) {
    Pixels::GetInstance().ClearDirty();
    Power::GetInstance().Task();
  }
  display.ResumeOutput();
  display.Enable();
  Expect(Power::IsSupplyOn() && display.IsEnabled(),
         "the case must leave the supply and the display on");
}

// The persistence after everything that may change a setting: a command that arrives over the
// line is saved on the tick it is carried out, not one cycle of the persistence later.
void TestSchedulerSavesWhatATickChanged() {
  Display& display = Display::GetInstance();
  byte const brightness_before = display.GetBrightness();

  display.SetBrightness(100u);
  TickAll();

  // The line is read on one tick and carried out on the next, as Communication does.
  char const line[] = "3 -B77\n";
  SerialShim::GetInstance().Inject(line, sizeof(line) - 1u);
  TickAll();
  TickAll();
  Expect(display.GetBrightness() == 77u, "the command must have been carried out");

  display.SetBrightness(10u);
  Expect(Persistence::Load() == kEOk && display.GetBrightness() == 77u,
         "and the store must hold what it set by the end of that tick");

  display.SetBrightness(brightness_before);
  TickAll();
}
