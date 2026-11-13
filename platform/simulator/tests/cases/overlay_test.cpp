// The three overlays - when they may start, what they show, and how long they hold the display.

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Overlays.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "Temperature.h"
#include "cases.h"
#include "check.h"
#include "sim/DS3231.h"

/*
 * The procedures that act now rather than at the next word change. What makes them worth
 * testing is the state they are asked for in: the display has been written to by something
 * that is not the clock, and nothing in the firmware takes it back - the latch sees the
 * same word set and leaves the letters alone, for up to five minutes.
 */
void TestShowNowProcedures() {
  Clock::GetInstance().SetMode(Clock::ModeType::kWessi);
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kNone);

  DisplayManager& display_manager = DisplayManager::GetInstance();
  Overlays& overlays = Overlays::GetInstance();
  Display& display = Display::GetInstance();

  SetTime(10u, 4u, 0u);
  display_manager.Task();
  PixelBufferType const clock_face = ReadPixels();

  display.Test();
  Expect(!ArePixelsEqual(ReadPixels(), clock_face), "the display test must reach the letters");
  display_manager.Task();
  Expect(!ArePixelsEqual(ReadPixels(), clock_face),
         "a task inside the same word set must not undo the display test");
  Expect(display_manager.RefreshClock() == kEOk, "the clock must be refreshable");
  Expect(ArePixelsEqual(ReadPixels(), clock_face), "the refresh must put the clock face back");

  animations.SetAnimation(FirstAnimation());
  Expect(display_manager.StartAnimation() == kEOk, "the selected animation must start on demand");
  Expect(animations.GetState() == Animations::StateType::kPending,
         "the started animation must be running");
  Expect(display_manager.AbortAnimation() == kEOk, "a running animation must be abortable");
  Expect(animations.GetState() == Animations::StateType::kIdle,
         "the aborted animation must be idle");
  Expect(ArePixelsEqual(ReadPixels(), clock_face), "the abort must put the clock face back");
  animations.SetAnimation(Animations::AnimationIdType::kNone);

  // An overlay owns the display while it shows, so both clock procedures step aside
  // instead of drawing underneath it.
  overlays.SetDateIsActive(true);
  Expect(overlays.ShowDateNow() == kEOk, "an active overlay must start on demand");
  Expect(overlays.GetState() == Overlays::OverlayType::StateType::kShow,
         "the started overlay must show");
  Expect(overlays.ShowDateNow() == kENotOk, "a second overlay must not start while one shows");
  Expect(display_manager.RefreshClock() == kENotOk,
         "the clock must not be refreshed under an overlay");
  Expect(display_manager.StartAnimation() == kENotOk, "no animation must start under an overlay");

  // Stands in for what the overlay's text draws, which needs the scheduler this test
  // does not run: what matters here is that the clock comes back over it.
  display_manager.Task();
  display.Test();

  Expect(overlays.Abort() == kEOk, "a showing overlay must be abortable");
  Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
         "the aborted overlay must stop showing");
  Expect(overlays.Abort() == kENotOk, "aborting with no overlay showing must be refused");

  display_manager.Task();
  Expect(ArePixelsEqual(ReadPixels(), clock_face),
         "the end of an overlay must put the clock face back");

  overlays.SetDateIsActive(false);
  Expect(overlays.ShowDateNow() == kENotOk, "a switched-off overlay must not be started");
}

/*
 * The temperature overlay's one rule that is not about text: it shows nothing at all
 * until a reading exists. A clock built without the chip stays in that state for good, so
 * what is checked here is the state and not the string - the string is checked against a
 * real register pair in the ESP32 host tests, where there is a driver to read it with.
 */
void TestTemperatureOverlayWithoutSensor() {
  Overlays& overlays = Overlays::GetInstance();

  DS3231::ClearSimulatedTemperature();
  Temperature::GetInstance().Task();

  overlays.SetTemperatureIsActive(true);
  Expect(overlays.ShowTemperatureNow() == kENotOk, "an overlay with no reading must not start");

  DS3231::SetSimulatedTemperature(215);
  Temperature::GetInstance().Task();
  Expect(overlays.ShowTemperatureNow() == kEOk, "a reading must let the overlay start");
  // Split around the escape on purpose: "\xB0C" would swallow the C as a third hex digit
  // and stop compiling, which is the trap this string invites.
  Expect(strcmp(overlays.GetTemperatureString(),
                "21.5\xB0"
                "C") == 0,
         "the overlay must show the dialled-in reading");

  /*
   * The reported form must stay free of the degree sign: it travels the web socket, and
   * a text frame carrying a raw Latin-1 byte is closed by the browser as invalid UTF-8
   * rather than drawn. This is the one assertion standing between that and a regression.
   */
  char reported[Temperature::kStringLength]{0};
  Expect(Temperature::GetInstance().GetTemperatureString(reported) == kEOk &&
             strcmp(reported, "21.5C") == 0,
         "the reported reading must carry no degree sign");
  Expect(overlays.Abort() == kEOk, "the overlay must end again");

  // Left as it was found: an active temperature overlay would take the display away
  // from whatever runs after this.
  overlays.SetTemperatureIsActive(false);
  DS3231::ClearSimulatedTemperature();
  Temperature::GetInstance().Task();
}

/*
 * When an overlay shows and for how long, which is the part of it no display can reveal:
 * the raster is a minute count and one particular second, and the endurance is counted in
 * the same task. Driven a second at a time, the way the clock does.
 */
void TestOverlayPeriodAndEndurance() {
  Overlays& overlays = Overlays::GetInstance();

  /* Every fourth minute, for three seconds. */
  Expect(overlays.SetDatePeriodInMinutes(4u) == kEOk, "the period must be accepted");
  Expect(overlays.SetDateEnduranceInSeconds(3u) == kEOk, "the endurance must be accepted");
  overlays.SetDateMonth(0u);
  overlays.SetDateDay(0u);
  overlays.SetDateValidInDays(0u);
  overlays.SetDateIsActive(true);

  /* A minute the raster covers, but before the second it starts on. */
  SetTime(10u, 8u, 29u);
  overlays.Task();
  Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
         "an overlay must not start before its second");

  SetTime(10u, 8u, 30u);
  overlays.Task();
  Expect(overlays.GetState() == Overlays::OverlayType::StateType::kShow,
         "an overlay must start on its second");

  /*
   * It ends by itself, after exactly the endurance it was given: three seconds, ending on
   * the third. It used to ask whether the time was up before counting, and ran one longer.
   */
  int seconds = 0;
  for (byte second = 31u;
       (second < 59u) && (overlays.GetState() == Overlays::OverlayType::StateType::kShow);
       second++) {
    SetTime(10u, 8u, second);
    overlays.Task();
    seconds++;
  }
  Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
         "an overlay must end by itself");
  Expect(seconds == 3, "and after exactly the endurance it was given");

  /* A minute the raster does not cover. */
  SetTime(10u, 9u, 30u);
  overlays.Task();
  Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
         "an overlay must not start outside its period");

  /* Switched off, it does not fire at all. */
  overlays.SetDateIsActive(false);
  SetTime(10u, 12u, 30u);
  overlays.Task();
  Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
         "an overlay that is switched off must stay away");
}

// Starts the date overlay by its period in `minute` of 10 o'clock, if it will, and says whether
// it did; a show it started is ended again. Overlays acts once per second, so the second before
// is passed through first.
static bool DateOverlayStartsIn(byte minute) {
  Overlays& overlays = Overlays::GetInstance();

  SetTime(10u, minute, 29u);
  overlays.Task();
  SetTime(10u, minute, 30u);
  overlays.Task();
  bool const started = overlays.GetState() == Overlays::OverlayType::StateType::kShow;
  overlays.Abort();
  return started;
}

static void ConfigureDateOverlayEveryMinute() {
  Overlays& overlays = Overlays::GetInstance();

  overlays.SetDatePeriodInMinutes(1u);
  overlays.SetDateEnduranceInSeconds(10u);
  overlays.SetDateMonth(0u);
  overlays.SetDateDay(0u);
  overlays.SetDateValidInDays(0u);
  overlays.SetDateIsActive(true);
}

/*
 * An overlay switched off, or reset, while it shows ends its show as Abort() would: its text
 * stops, and the next overlay starts by its period. It used to go straight to switched off,
 * leaving the text scrolling over the clock and the show timer standing, and no overlay
 * started by its period again until a "show now" had run.
 */
void TestOverlaySwitchedOffWhileItShows() {
  Overlays& overlays = Overlays::GetInstance();

  ConfigureDateOverlayEveryMinute();
  SetTime(10u, 20u, 29u);
  overlays.Task();
  SetTime(10u, 20u, 30u);
  overlays.Task();
  SetTime(10u, 20u, 32u);
  overlays.Task();
  Expect(overlays.GetState() == Overlays::OverlayType::StateType::kShow,
         "the date overlay must show");

  overlays.SetDateIsActive(false);
  Expect(Text::GetInstance().GetState() == Text::StateType::kIdle,
         "an overlay switched off while it shows must stop its text");
  overlays.SetDateIsActive(true);
  Expect(DateOverlayStartsIn(21u), "and the next start by period must come");

  SetTime(10u, 22u, 29u);
  overlays.Task();
  SetTime(10u, 22u, 30u);
  overlays.Task();
  overlays.ResetToDefaults();
  Expect(Text::GetInstance().GetState() == Text::StateType::kIdle,
         "an overlay reset while it shows must stop its text");
  ConfigureDateOverlayEveryMinute();
  Expect(DateOverlayStartsIn(23u), "and the next start by period must come after a reset too");

  overlays.SetDateIsActive(false);
}

/*
 * An overlay that starts ends a running animation, by its period and by "show now" alike. Both
 * draw on the same display, and an animation still running at second 30 - Fade at its default
 * speed, any at a low one - drew its pixels into the scrolling text.
 */
void TestStartingOverlayEndsTheAnimation() {
  Overlays& overlays = Overlays::GetInstance();
  Animations& animations = Animations::GetInstance();

  ConfigureDateOverlayEveryMinute();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(FirstAnimation());

  DrawClockFace(10u, 4u);
  animations.SetTime(10u, 35u);
  SetTime(10u, 30u, 29u);
  overlays.Task();
  Expect(animations.GetState() == Animations::StateType::kPending, "the animation must be running");
  SetTime(10u, 30u, 30u);
  overlays.Task();
  Expect(overlays.GetState() == Overlays::OverlayType::StateType::kShow,
         "the date overlay must start");
  Expect(animations.GetState() == Animations::StateType::kIdle,
         "an overlay that starts by its period must end a running animation");
  overlays.Abort();

  DrawClockFace(10u, 4u);
  animations.SetTime(10u, 35u);
  Expect(overlays.ShowDateNow() == kEOk, "the date overlay must start now");
  Expect(animations.GetState() == Animations::StateType::kIdle,
         "an overlay shown now must end a running animation");
  overlays.Abort();

  animations.SetAnimation(Animations::AnimationIdType::kNone);
  overlays.SetDateIsActive(false);
}

/*
 * Valid-in-days counts from the date on: 24 December and three days is 24 to 27 December, and
 * across the end of the year as well. It used to count in both directions, and the days before
 * the date only within its month - 21 December, but not 31 December for 2 January.
 */
void TestValidInDaysCountsFromTheDate() {
  Overlays& overlays = Overlays::GetInstance();

  ConfigureDateOverlayEveryMinute();
  overlays.SetDateMonth(12u);
  overlays.SetDateDay(24u);
  overlays.SetDateValidInDays(3u);

  RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 12u, 21u));
  Expect(!DateOverlayStartsIn(40u), "valid-in-days must not reach the days before the date");
  RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 12u, 24u));
  Expect(DateOverlayStartsIn(41u), "the date itself must be in the window");
  RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 12u, 27u));
  Expect(DateOverlayStartsIn(42u), "the last day of the window must be in it");
  RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 12u, 28u));
  Expect(!DateOverlayStartsIn(43u), "the day after the window must not be in it");

  overlays.SetDateMonth(12u);
  overlays.SetDateDay(30u);
  overlays.SetDateValidInDays(5u);
  RealTimeClock::GetInstance().SetDate(ClockDate(2027u, 1u, 3u));
  Expect(DateOverlayStartsIn(44u), "the window must reach across the end of the year");

  overlays.SetDateMonth(1u);
  overlays.SetDateDay(2u);
  RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 12u, 31u));
  Expect(!DateOverlayStartsIn(45u), "nor the days before the date across the year");

  overlays.SetDateMonth(0u);
  overlays.SetDateDay(0u);
  overlays.SetDateValidInDays(0u);
  overlays.SetDateIsActive(false);
}
