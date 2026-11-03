/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       overlay_test.cpp
 *      \brief      The three overlays - when they may start, what they show, and how long they hold the display
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Overlays.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "Temperature.h"
#include "sim/DS3231.h"

/* The procedures that act now rather than at the next word change. What makes them worth
   testing is the state they are asked for in: the display has been written to by something
   that is not the clock, and nothing in the firmware takes it back - the latch sees the
   same word set and leaves the letters alone, for up to five minutes. */
void TestShowNowProcedures()
{
    Clock::GetInstance().SetMode(Clock::ModeType::kWessi);
    Animations& animations = Animations::GetInstance();
    animations.SetMode(Animations::ModeType::kFixed);
    animations.SetAnimation(Animations::AnimationIdType::kNone);

    DisplayManager& display_manager = DisplayManager::GetInstance();
    Overlays& overlays = Overlays::GetInstance();
    Display& display = Display::GetInstance();

    SetTime(10u, 4u, 0u);
    display_manager.Task();
    const PixelBufferType clock_face = ReadPixels();

    display.Test();
    Expect(!ArePixelsEqual(ReadPixels(), clock_face), "the display test must reach the letters");
    display_manager.Task();
    Expect(!ArePixelsEqual(ReadPixels(), clock_face),
           "a task inside the same word set must not undo the display test");
    Expect(display_manager.RefreshClock() == kEOk, "the clock must be refreshable");
    Expect(ArePixelsEqual(ReadPixels(), clock_face), "the refresh must put the clock face back");

    animations.SetAnimation(FirstAnimation());
    Expect(display_manager.StartAnimation() == kEOk, "the selected animation must start on demand");
    Expect(animations.GetState() == Animations::StateType::kPending, "the started animation must be running");
    Expect(display_manager.AbortAnimation() == kEOk, "a running animation must be abortable");
    Expect(animations.GetState() == Animations::StateType::kIdle, "the aborted animation must be idle");
    Expect(ArePixelsEqual(ReadPixels(), clock_face), "the abort must put the clock face back");
    animations.SetAnimation(Animations::AnimationIdType::kNone);

    /* An overlay owns the display while it shows, so both clock procedures step aside
       instead of drawing underneath it. */
    overlays.SetDateIsActive(true);
    Expect(overlays.ShowDateNow() == kEOk, "an active overlay must start on demand");
    Expect(overlays.GetState() == Overlays::OverlayType::StateType::kShow, "the started overlay must show");
    Expect(overlays.ShowDateNow() == kENotOk, "a second overlay must not start while one shows");
    Expect(display_manager.RefreshClock() == kENotOk, "the clock must not be refreshed under an overlay");
    Expect(display_manager.StartAnimation() == kENotOk, "no animation must start under an overlay");

    /* Stands in for what the overlay's text draws, which needs the scheduler this test
       does not run: what matters here is that the clock comes back over it. */
    display_manager.Task();
    display.Test();

    Expect(overlays.Abort() == kEOk, "a showing overlay must be abortable");
    Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow,
           "the aborted overlay must stop showing");
    Expect(overlays.Abort() == kENotOk, "aborting with no overlay showing must be refused");

    display_manager.Task();
    Expect(ArePixelsEqual(ReadPixels(), clock_face), "the end of an overlay must put the clock face back");

    overlays.SetDateIsActive(false);
    Expect(overlays.ShowDateNow() == kENotOk, "a switched-off overlay must not be started");
}

/* The temperature overlay's one rule that is not about text: it shows nothing at all
   until a reading exists. A clock built without the chip stays in that state for good, so
   what is checked here is the state and not the string - the string is checked against a
   real register pair in the ESP32 host tests, where there is a driver to read it with. */
void TestTemperatureOverlayWithoutSensor()
{
    Overlays& overlays = Overlays::GetInstance();

    DS3231::ClearSimulatedTemperature();
    Temperature::GetInstance().Task();

    overlays.SetTemperatureIsActive(true);
    Expect(overlays.ShowTemperatureNow() == kENotOk,
           "an overlay with no reading must not start");

    DS3231::SetSimulatedTemperature(215);
    Temperature::GetInstance().Task();
    Expect(overlays.ShowTemperatureNow() == kEOk, "a reading must let the overlay start");
    /* Split around the escape on purpose: "\xB0C" would swallow the C as a third hex digit
       and stop compiling, which is the trap this string invites. */
    Expect(strcmp(overlays.GetTemperatureString(), "21.5\xB0" "C") == 0,
           "the overlay must show the dialled-in reading");

    /* The reported form must stay free of the degree sign: it travels the web socket, and
       a text frame carrying a raw Latin-1 byte is closed by the browser as invalid UTF-8
       rather than drawn. This is the one assertion standing between that and a regression. */
    char reported[Temperature::kStringLength]{0};
    Expect(Temperature::GetInstance().GetTemperatureString(reported) == kEOk
               && strcmp(reported, "21.5C") == 0,
           "the reported reading must carry no degree sign");
    Expect(overlays.Abort() == kEOk, "the overlay must end again");

    /* Left as it was found: an active temperature overlay would take the display away
       from whatever runs after this. */
    overlays.SetTemperatureIsActive(false);
    DS3231::ClearSimulatedTemperature();
    Temperature::GetInstance().Task();
}

/* When an overlay shows and for how long, which is the part of it no display can reveal:
   the raster is a minute count and one particular second, and the endurance is counted in
   the same task. Driven a second at a time, the way the clock does. */
void TestOverlayPeriodAndEndurance()
{
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

    /* It ends by itself, and within the endurance it was given rather than whenever. */
    int seconds = 0;
    for(byte second = 31u; (second < 59u) && (overlays.GetState() == Overlays::OverlayType::StateType::kShow); second++) {
        SetTime(10u, 8u, second);
        overlays.Task();
        seconds++;
    }
    Expect(overlays.GetState() != Overlays::OverlayType::StateType::kShow, "an overlay must end by itself");
    Expect(seconds <= 5, "and within the endurance it was given");

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
