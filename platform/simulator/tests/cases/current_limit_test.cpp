/*
 * That the supply's budget is what decides how bright the display may get.
 *
 * Two halves. The arithmetic is checked over the whole range of lit counts, because the property
 * that matters is not one value but that no value exceeds the budget. The rest is checked on the
 * display, because where the cap is applied decides whether a restored clock face comes back at the
 * brightness it had.
 */

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "DisplayCurrentLimit.h"
#include "DisplayManager.h"
#include "Pixels.h"

namespace {
constexpr uint32_t kBudgetMicroAmps{static_cast<uint32_t>(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS) * 1000u};
constexpr uint16_t kWhiteChannelSum{DisplayCurrentLimit::ToChannelSum(255u, 255u, 255u)};
constexpr uint16_t kRedChannelSum{DisplayCurrentLimit::ToChannelSum(255u, 0u, 0u)};

// The master the strip is carrying, which is where the cap lives - not the setting, which
// GetBrightness() answers and which the cap deliberately leaves alone.
byte GetMaster()
{
    return Pixels::GetInstance().GetBrightness();
}

// What the strip would draw with a given master, in microamps. The idle current does not
// scale: a WS2812 held dark by the master still draws it.
uint32_t ToCurrentMicroAmps(byte lit_pixels, uint16_t channel_sum, byte master)
{
    const uint32_t idle = DisplayCurrentLimit::GetIdleCurrentMicroAmps(PIXELS_NUMBER_OF_LEDS);
    const uint32_t at_full = DisplayCurrentLimit::GetCurrentAtFullBrightnessMicroAmps(PIXELS_NUMBER_OF_LEDS, lit_pixels, channel_sum);

    return idle + ((at_full - idle) * master / 256u);
}

// White, enabled, and a clock face on the display - what every case here starts from and has
// to leave behind, since the cases after this file expect a white display.
void StartFromWhiteClockFace()
{
    Display& display = Display::GetInstance();

    display.Enable();
    display.ResetColor();
    DrawClockFace(10u, 4u);
    DisplayManager::GetInstance().Task();
}

}

/*
 * The property the class exists for, checked as a property rather than at one point: for every
 * number of lit letters, the current at the brightness the solver allows must fit the budget,
 * and one more lit letter must never allow more brightness than the last.
 */
void TestCurrentLimitFitsTheBudgetAtEveryLitCount()
{
    Expect(DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, 0u, kWhiteChannelSum) == DisplayCurrentLimit::kNoLimit,
           "a display with nothing lit must not be held back");
    Expect(DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, kWhiteChannelSum) < DisplayCurrentLimit::kNoLimit,
           "a full white display must be held back, or this budget leaves nothing to test");

    byte previous = DisplayCurrentLimit::kNoLimit;
    for(byte lit = 0u; lit <= PIXELS_NUMBER_OF_LEDS; lit++) {
        const byte limit = DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, lit, kWhiteChannelSum);

        Expect(limit <= previous, "one more lit letter must never allow more brightness");
        Expect(ToCurrentMicroAmps(lit, kWhiteChannelSum, limit) <= kBudgetMicroAmps,
               "the brightness the limit allows must stay inside the budget");
        previous = limit;
    }
}

/*
 * The colour decides as much as the count does: red asks a third of what white asks at the
 * same brightness, and on this display that is the difference between fitting the budget and
 * not. A limiter that only counted letters would hold both back equally.
 */
void TestCurrentLimitFollowsTheColour()
{
    const byte white_limit = DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, kWhiteChannelSum);
    const byte red_limit = DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, kRedChannelSum);

    Expect(white_limit < red_limit, "white must be held back harder than red on the same letters");

    Display& display = Display::GetInstance();

    StartFromWhiteClockFace();
    display.Test();
    Expect(GetMaster() == white_limit, "a full white display must be capped at the white limit");

    display.SetColor(255u, 0u, 0u);
    Expect(GetMaster() == red_limit, "and turning it red must lift the cap to the red one");

    StartFromWhiteClockFace();
}

/*
 * Where the cap is applied, checked by what a restored clock face looks like. The cap rides on
 * the strip's master, so the buffer keeps the colour it was written with and a face that comes
 * back after a full display is the face that went away. Applying the cap to the colour instead
 * would leave it a fifth darker until the next task run, which is what this case would catch.
 */
void TestCurrentLimitLeavesTheRestoredFaceAlone()
{
    Display& display = Display::GetInstance();

    StartFromWhiteClockFace();
    const PixelBufferType clock_face = ReadPixels();
    Expect(GetMaster() == DisplayCurrentLimit::kNoLimit, "a clock face on this budget must run uncapped");

    display.Test();
    Expect(GetMaster() < DisplayCurrentLimit::kNoLimit, "the display test must be capped");

    DrawClockFace(10u, 4u);
    DisplayManager::GetInstance().Task();
    Expect(GetMaster() == DisplayCurrentLimit::kNoLimit, "the cap must lift again once the letters go");
    Expect(ArePixelsEqual(ReadPixels(), clock_face), "and the face must come back exactly as it was");

    StartFromWhiteClockFace();
}

// The master is also what Disable() uses, so the two must not fight over it: a task on a dark
// display must leave it dark rather than write a cap into it.
void TestCurrentLimitDoesNotSwitchTheDisplayOn()
{
    Display& display = Display::GetInstance();

    StartFromWhiteClockFace();
    display.Test();
    display.Disable();
    Expect(GetMaster() == 0u, "a disabled display must have its master at zero");

    DisplayManager::GetInstance().Task();
    Expect(GetMaster() == 0u, "and a task must not put a cap there instead");

    display.Enable();
    Expect(GetMaster() == DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, kWhiteChannelSum),
           "switching back on must go through the cap rather than to full");

    StartFromWhiteClockFace();
}
