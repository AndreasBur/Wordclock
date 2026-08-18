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
/**     \file       current_limit_test.cpp
 *      \brief      That the supply's budget is what decides how bright the display may get
 *
 *      \details    Two halves. The arithmetic is checked over the whole range of lit counts,
 *                  because the property that matters is not one value but that no value
 *                  exceeds the budget. The rest is checked on the display, because where the
 *                  cap is applied decides whether a restored clock face comes back at the
 *                  brightness it had.
 *
******************************************************************************************************************************************************/

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "DisplayCurrentLimit.h"
#include "DisplayManager.h"
#include "Pixels.h"

namespace {

constexpr uint32_t BudgetMicroAmps{static_cast<uint32_t>(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS) * 1000u};
constexpr uint16_t WhiteChannelSum{DisplayCurrentLimit::toChannelSum(255u, 255u, 255u)};
constexpr uint16_t RedChannelSum{DisplayCurrentLimit::toChannelSum(255u, 0u, 0u)};

/* The master the strip is carrying, which is where the cap lives - not the setting, which
   getBrightness() answers and which the cap deliberately leaves alone. */
byte getMaster()
{
    return Pixels::getInstance().getBrightness();
}

/* What the strip would draw with a given master, in microamps. The idle current does not
   scale: a WS2812 held dark by the master still draws it. */
uint32_t toCurrentMicroAmps(byte LitPixels, uint16_t ChannelSum, byte Master)
{
    const uint32_t Idle = DisplayCurrentLimit::getIdleCurrentMicroAmps(PIXELS_NUMBER_OF_LEDS);
    const uint32_t AtFull = DisplayCurrentLimit::getCurrentAtFullBrightnessMicroAmps(PIXELS_NUMBER_OF_LEDS, LitPixels, ChannelSum);

    return Idle + ((AtFull - Idle) * Master / 256u);
}

/* White, enabled, and a clock face on the display - what every case here starts from and has
   to leave behind, since the cases after this file expect a white display. */
void startFromWhiteClockFace()
{
    Display& display = Display::getInstance();

    display.enable();
    display.resetColor();
    drawClockFace(10u, 4u);
    DisplayManager::getInstance().task();
}

} /* namespace */


/* The property the class exists for, checked as a property rather than at one point: for every
   number of lit letters, the current at the brightness the solver allows must fit the budget,
   and one more lit letter must never allow more brightness than the last. */
void testCurrentLimitFitsTheBudgetAtEveryLitCount()
{
    expect(DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, 0u, WhiteChannelSum) == DisplayCurrentLimit::NoLimit,
           "a display with nothing lit must not be held back");
    expect(DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, WhiteChannelSum) < DisplayCurrentLimit::NoLimit,
           "a full white display must be held back, or this budget leaves nothing to test");

    byte Previous = DisplayCurrentLimit::NoLimit;
    for(byte Lit = 0u; Lit <= PIXELS_NUMBER_OF_LEDS; Lit++) {
        const byte Limit = DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, Lit, WhiteChannelSum);

        expect(Limit <= Previous, "one more lit letter must never allow more brightness");
        expect(toCurrentMicroAmps(Lit, WhiteChannelSum, Limit) <= BudgetMicroAmps,
               "the brightness the limit allows must stay inside the budget");
        Previous = Limit;
    }
}


/* The colour decides as much as the count does: red asks a third of what white asks at the
   same brightness, and on this display that is the difference between fitting the budget and
   not. A limiter that only counted letters would hold both back equally. */
void testCurrentLimitFollowsTheColour()
{
    const byte WhiteLimit = DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, WhiteChannelSum);
    const byte RedLimit = DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, RedChannelSum);

    expect(WhiteLimit < RedLimit, "white must be held back harder than red on the same letters");

    Display& display = Display::getInstance();

    startFromWhiteClockFace();
    display.test();
    expect(getMaster() == WhiteLimit, "a full white display must be capped at the white limit");

    display.setColor(255u, 0u, 0u);
    expect(getMaster() == RedLimit, "and turning it red must lift the cap to the red one");

    startFromWhiteClockFace();
}


/* Where the cap is applied, checked by what a restored clock face looks like. The cap rides on
   the strip's master, so the buffer keeps the colour it was written with and a face that comes
   back after a full display is the face that went away. Applying the cap to the colour instead
   would leave it a fifth darker until the next task run, which is what this case would catch. */
void testCurrentLimitLeavesTheRestoredFaceAlone()
{
    Display& display = Display::getInstance();

    startFromWhiteClockFace();
    const PixelBufferType ClockFace = readPixels();
    expect(getMaster() == DisplayCurrentLimit::NoLimit, "a clock face on this budget must run uncapped");

    display.test();
    expect(getMaster() < DisplayCurrentLimit::NoLimit, "the display test must be capped");

    drawClockFace(10u, 4u);
    DisplayManager::getInstance().task();
    expect(getMaster() == DisplayCurrentLimit::NoLimit, "the cap must lift again once the letters go");
    expect(arePixelsEqual(readPixels(), ClockFace), "and the face must come back exactly as it was");

    startFromWhiteClockFace();
}


/* The master is also what disable() uses, so the two must not fight over it: a task on a dark
   display must leave it dark rather than write a cap into it. */
void testCurrentLimitDoesNotSwitchTheDisplayOn()
{
    Display& display = Display::getInstance();

    startFromWhiteClockFace();
    display.test();
    display.disable();
    expect(getMaster() == 0u, "a disabled display must have its master at zero");

    DisplayManager::getInstance().task();
    expect(getMaster() == 0u, "and a task must not put a cap there instead");

    display.enable();
    expect(getMaster() == DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, WhiteChannelSum),
           "switching back on must go through the cap rather than to full");

    startFromWhiteClockFace();
}
