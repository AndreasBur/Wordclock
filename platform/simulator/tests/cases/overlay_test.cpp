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
void testShowNowProcedures()
{
    Clock::getInstance().setModeFast(Clock::MODE_WESSI);
    Animations& animations = Animations::getInstance();
    animations.setModeFast(Animations::MODE_FIXED);
    animations.setAnimationFast(Animations::ANIMATION_ID_NONE);

    DisplayManager& displayManager = DisplayManager::getInstance();
    Overlays& overlays = Overlays::getInstance();
    Display& display = Display::getInstance();

    setTime(10u, 4u, 0u);
    displayManager.task();
    const PixelBufferType clockFace = readPixels();

    display.test();
    expect(!arePixelsEqual(readPixels(), clockFace), "the display test must reach the letters");
    displayManager.task();
    expect(!arePixelsEqual(readPixels(), clockFace),
           "a task inside the same word set must not undo the display test");
    expect(displayManager.refreshClock() == E_OK, "the clock must be refreshable");
    expect(arePixelsEqual(readPixels(), clockFace), "the refresh must put the clock face back");

    animations.setAnimationFast(Animations::ANIMATION_ID_CURSOR);
    expect(displayManager.startAnimation() == E_OK, "the selected animation must start on demand");
    expect(animations.getState() == Animations::STATE_PENDING, "the started animation must be running");
    expect(displayManager.abortAnimation() == E_OK, "a running animation must be abortable");
    expect(animations.getState() == Animations::STATE_IDLE, "the aborted animation must be idle");
    expect(arePixelsEqual(readPixels(), clockFace), "the abort must put the clock face back");
    animations.setAnimationFast(Animations::ANIMATION_ID_NONE);

    /* An overlay owns the display while it shows, so both clock procedures step aside
       instead of drawing underneath it. */
    overlays.setDateIsActive(true);
    expect(overlays.showDateNow() == E_OK, "an active overlay must start on demand");
    expect(overlays.getState() == Overlays::OverlayType::STATE_SHOW, "the started overlay must show");
    expect(overlays.showDateNow() == E_NOT_OK, "a second overlay must not start while one shows");
    expect(displayManager.refreshClock() == E_NOT_OK, "the clock must not be refreshed under an overlay");
    expect(displayManager.startAnimation() == E_NOT_OK, "no animation must start under an overlay");

    /* Stands in for what the overlay's text draws, which needs the scheduler this test
       does not run: what matters here is that the clock comes back over it. */
    displayManager.task();
    display.test();

    expect(overlays.abort() == E_OK, "a showing overlay must be abortable");
    expect(overlays.getState() != Overlays::OverlayType::STATE_SHOW,
           "the aborted overlay must stop showing");
    expect(overlays.abort() == E_NOT_OK, "aborting with no overlay showing must be refused");

    displayManager.task();
    expect(arePixelsEqual(readPixels(), clockFace), "the end of an overlay must put the clock face back");

    overlays.setDateIsActive(false);
    expect(overlays.showDateNow() == E_NOT_OK, "a switched-off overlay must not be started");
}

/* The temperature overlay's one rule that is not about text: it shows nothing at all
   until a reading exists. A clock built without the chip stays in that state for good, so
   what is checked here is the state and not the string - the string is checked against a
   real register pair in the ESP32 host tests, where there is a driver to read it with. */
void testTemperatureOverlayWithoutSensor()
{
    Overlays& overlays = Overlays::getInstance();

    DS3231::clearSimulatedTemperature();
    Temperature::getInstance().task();

    overlays.setTemperatureIsActive(true);
    expect(overlays.showTemperatureNow() == E_NOT_OK,
           "an overlay with no reading must not start");

    DS3231::setSimulatedTemperature(215);
    Temperature::getInstance().task();
    expect(overlays.showTemperatureNow() == E_OK, "a reading must let the overlay start");
    /* Split around the escape on purpose: "\xB0C" would swallow the C as a third hex digit
       and stop compiling, which is the trap this string invites. */
    expect(strcmp(overlays.getTemperatureString(), "21.5\xB0" "C") == 0,
           "the overlay must show the dialled-in reading");

    /* The reported form must stay free of the degree sign: it travels the web socket, and
       a text frame carrying a raw Latin-1 byte is closed by the browser as invalid UTF-8
       rather than drawn. This is the one assertion standing between that and a regression. */
    char Reported[Temperature::StringLength]{0};
    expect(Temperature::getInstance().getTemperatureString(Reported) == E_OK
               && strcmp(Reported, "21.5C") == 0,
           "the reported reading must carry no degree sign");
    expect(overlays.abort() == E_OK, "the overlay must end again");

    /* Left as it was found: an active temperature overlay would take the display away
       from whatever runs after this. */
    overlays.setTemperatureIsActive(false);
    DS3231::clearSimulatedTemperature();
    Temperature::getInstance().task();
}

/* When an overlay shows and for how long, which is the part of it no display can reveal:
   the raster is a minute count and one particular second, and the endurance is counted in
   the same task. Driven a second at a time, the way the clock does. */
void testOverlayPeriodAndEndurance()
{
    Overlays& overlays = Overlays::getInstance();

    /* Every fourth minute, for three seconds. */
    expect(overlays.setDatePeriodInMinutes(4u) == E_OK, "the period must be accepted");
    expect(overlays.setDateEnduranceInSeconds(3u) == E_OK, "the endurance must be accepted");
    overlays.setDateMonth(0u);
    overlays.setDateDay(0u);
    overlays.setDateValidInDays(0u);
    overlays.setDateIsActive(true);

    /* A minute the raster covers, but before the second it starts on. */
    setTime(10u, 8u, 29u);
    overlays.task();
    expect(overlays.getState() != Overlays::OverlayType::STATE_SHOW,
           "an overlay must not start before its second");

    setTime(10u, 8u, 30u);
    overlays.task();
    expect(overlays.getState() == Overlays::OverlayType::STATE_SHOW,
           "an overlay must start on its second");

    /* It ends by itself, and within the endurance it was given rather than whenever. */
    int Seconds = 0;
    for(byte Second = 31u; (Second < 59u) && (overlays.getState() == Overlays::OverlayType::STATE_SHOW); Second++) {
        setTime(10u, 8u, Second);
        overlays.task();
        Seconds++;
    }
    expect(overlays.getState() != Overlays::OverlayType::STATE_SHOW, "an overlay must end by itself");
    expect(Seconds <= 5, "and within the endurance it was given");

    /* A minute the raster does not cover. */
    setTime(10u, 9u, 30u);
    overlays.task();
    expect(overlays.getState() != Overlays::OverlayType::STATE_SHOW,
           "an overlay must not start outside its period");

    /* Switched off, it does not fire at all. */
    overlays.setDateIsActive(false);
    setTime(10u, 12u, 30u);
    overlays.task();
    expect(overlays.getState() != Overlays::OverlayType::STATE_SHOW,
           "an overlay that is switched off must stay away");
}
