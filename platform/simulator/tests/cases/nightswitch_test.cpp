/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       nightswitch_test.cpp
 *      \brief      The window, the midnight wrap, and who wins between the timer and a hand
******************************************************************************************************************************************************/

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "NightSwitch.h"
#include "Pixels.h"
#include "RealTimeClock.h"

namespace {

/* One tick of the night switch at a given time. Its own task rather than the scheduler's,
   because what is being checked is the edge and not the cycle counting that reaches it. */
void tickAt(byte Hour, byte Minute)
{
    setTime(Hour, Minute, 0u);
    NightSwitch::getInstance().task();
}

} // namespace

/* The window itself, including the one that crosses midnight - which is the ordinary case
   for a clock, and the only one where the test reads the other way round. */
void testNightWindowIncludesMidnight()
{
    NightSwitch& nightSwitch = NightSwitch::getInstance();

    nightSwitch.setStartHour(23u);
    nightSwitch.setStartMinute(0u);
    nightSwitch.setEndHour(6u);
    nightSwitch.setEndMinute(30u);

    setTime(22u, 59u, 0u); expect(!nightSwitch.isNight(), "a minute before the start is still day");
    setTime(23u, 0u, 0u);  expect(nightSwitch.isNight(),  "the start minute itself is night");
    setTime(2u, 0u, 0u);   expect(nightSwitch.isNight(),  "the small hours are night");
    setTime(6u, 29u, 0u);  expect(nightSwitch.isNight(),  "a minute before the end is still night");
    setTime(6u, 30u, 0u);  expect(!nightSwitch.isNight(), "the end minute itself is day");
    setTime(12u, 0u, 0u);  expect(!nightSwitch.isNight(), "midday is day");

    /* A window inside one day, which must not be read as its own inverse. */
    nightSwitch.setStartHour(1u);
    nightSwitch.setEndHour(5u);
    nightSwitch.setStartMinute(0u);
    nightSwitch.setEndMinute(0u);
    setTime(3u, 0u, 0u);   expect(nightSwitch.isNight(),  "a window inside one day holds in the middle");
    setTime(23u, 0u, 0u);  expect(!nightSwitch.isNight(), "and not outside it");

    /* Both ends the same minute is nothing configured, not a whole day. */
    nightSwitch.setEndHour(1u);
    setTime(1u, 0u, 0u);   expect(!nightSwitch.isNight(), "an empty window is never night");
}

/* What the window does to the display, and what it does not do to the brightness setting -
   which is the whole reason the night dimming is its own level and not the fade. */
void testNightSwitchDimsWithoutLosingTheSetting()
{
    NightSwitch& nightSwitch = NightSwitch::getInstance();
    Display& display = Display::getInstance();

    display.enable();
    display.setBrightness(120u);
    Clock::getInstance().setTime(10u, 5u);
    expect(display.show() == E_OK, "the clock face must reach the strip");

    nightSwitch.setStartHour(23u);
    nightSwitch.setStartMinute(0u);
    nightSwitch.setEndHour(6u);
    nightSwitch.setEndMinute(0u);
    nightSwitch.setNightBrightness(0u);
    nightSwitch.setIsActive(true);

    tickAt(22u, 0u);
    expect(isAnyOutputPixelLit(), "before the window the display is on");

    tickAt(23u, 0u);
    expect(!isAnyOutputPixelLit(), "a brightness of zero switches the display off for the night");

    tickAt(6u, 0u);
    expect(isAnyOutputPixelLit(), "morning gives it back");
    expect(display.getBrightness() == 120u, "and gives back the brightness that was set");

    /* Dimming rather than switching off: still lit, still not the day brightness, and the
       setting untouched - Persistence reads that setting, so a night that wrote it would
       be a night that changed the clock for good. */
    nightSwitch.setNightBrightness(16u);
    tickAt(23u, 0u);
    expect(isAnyOutputPixelLit(), "a dimmed night is still lit");
    expect(display.getBrightness() == 120u, "and leaves the brightness setting alone");
    expect(display.getBrightnessNightLevel() == 16u, "the dimming is the night level");

    tickAt(6u, 0u);
    expect(display.getBrightnessNightLevel() == 255u, "which morning clears again");

    nightSwitch.setIsActive(false);
}

/* The edge, not the state. A clock switched on by hand in the small hours has to stay on,
   or a timer is something people switch off after one night. */
void testNightSwitchActsOnTheCrossingOnly()
{
    NightSwitch& nightSwitch = NightSwitch::getInstance();
    Display& display = Display::getInstance();

    display.enable();
    display.setBrightness(120u);
    Clock::getInstance().setTime(10u, 5u);
    expect(display.show() == E_OK, "the clock face must reach the strip");

    nightSwitch.setStartHour(23u);
    nightSwitch.setStartMinute(0u);
    nightSwitch.setEndHour(6u);
    nightSwitch.setEndMinute(0u);
    nightSwitch.setNightBrightness(0u);
    nightSwitch.setIsActive(true);

    tickAt(22u, 0u);
    tickAt(23u, 0u);
    expect(!isAnyOutputPixelLit(), "the window switches the display off");

    display.enable();
    expect(isAnyOutputPixelLit(), "a hand switches it back on");

    tickAt(2u, 0u);
    tickAt(3u, 0u);
    expect(isAnyOutputPixelLit(), "and the timer leaves it on until the window ends");

    tickAt(6u, 0u);
    expect(isAnyOutputPixelLit(), "the end of the window is not a reason to switch it off");

    /* And switching the whole timer off hands the display back rather than leaving it
       wherever the last edge put it. */
    tickAt(23u, 0u);
    expect(!isAnyOutputPixelLit(), "the next night switches it off again");
    nightSwitch.setIsActive(false);
    expect(isAnyOutputPixelLit(), "switching the timer off gives the display back");
}
