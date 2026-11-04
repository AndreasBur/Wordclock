/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       colorcycle_test.cpp
 *      \brief      The wheel, what it leaves alone, and what the supply is told about it
******************************************************************************************************************************************************/

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "ColorCycle.h"
#include "Display.h"
#include "DisplayCurrentLimit.h"
#include "Pixels.h"

namespace {

/*
 * The colour the strip is actually carrying, read off the first lit pixel. The buffer holds
 * the colour already dimmed by the brightness, so the cases below set a brightness that
 * leaves the channels recognisable rather than comparing against exact values.
 */
Pixel FirstLitPixel()
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        const Pixel candidate = Pixels::GetInstance().GetPixel(index);

        if(candidate.GetRed() || candidate.GetGreen() || candidate.GetBlue()) { return candidate; }
    }
    return Pixel(0u, 0u, 0u);
}

bool ArePixelsSame(Pixel left, Pixel right)
{
    return (left.GetRed() == right.GetRed()) && (left.GetGreen() == right.GetGreen())
        && (left.GetBlue() == right.GetBlue());
}

byte GetMaster() { return Pixels::GetInstance().GetBrightness(); }

/*
 * A lit clock face at full brightness, the cycle off, and a colour that is nobody's hue on
 * the wheel - so a pixel carrying the wheel's colour cannot be mistaken for one carrying the
 * setting. Also what every case here has to leave behind, white included, because the cases
 * after this file start from a white display.
 */
void StartFromWhiteClockFace()
{
    Display& display = Display::GetInstance();

    ColorCycle::GetInstance().SetIsActive(false);
    display.Enable();
    display.SetBrightness(255u);
    display.SetColor(255u, 255u, 255u);
    Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
    Expect(display.Show() == kEOk, "the clock face must reach the strip");
}

} // namespace

// The whole point of the cycle being a level of its own: what it paints is on the strip, and
// what somebody chose is still what the clock answers and what Persistence would store.
void TestColourCycleLeavesTheChosenColourAlone()
{
    Display& display = Display::GetInstance();
    ColorCycle& color_cycle = ColorCycle::GetInstance();

    StartFromWhiteClockFace();
    // Deliberately not on the wheel: full saturation is what the cycle paints, so a colour
    // with three unequal channels cannot be one of its.
    display.SetColor(200u, 120u, 60u);
    const Pixel chosen = FirstLitPixel();

    color_cycle.SetIsActive(true);
    const Pixel first_of_the_wheel = FirstLitPixel();
    Expect(!ArePixelsSame(chosen, first_of_the_wheel), "switching the cycle on must change what the strip carries");
    Expect(display.GetColorRed() == 200u && display.GetColorGreen() == 120u && display.GetColorBlue() == 60u,
           "and must not touch the colour that was chosen");

    // A step of the wheel, which is one task run - and a visible one, since what is being
    // checked is that the strip follows it.
    color_cycle.Task();
    Expect(!ArePixelsSame(first_of_the_wheel, FirstLitPixel()), "a step of the wheel must reach the strip");
    Expect(display.GetColorRed() == 200u, "and still must not touch the setting");

    color_cycle.SetIsActive(false);
    Expect(ArePixelsSame(chosen, FirstLitPixel()), "switching it off must put the chosen colour back");
    // Nothing was remembered to put back: the setting was never overwritten, which is what
    // makes this work rather than a saved copy somewhere.
    Expect(!display.IsColorCycleActive(), "with no cycle level left over the top");

    StartFromWhiteClockFace();
}

// What the wheel is: a full round of saturated colours. Checked through the module rather
// than against the conversion, because the property is what reaches the strip.
void TestColourCycleWalksTheWholeWheel()
{
    Display& display = Display::GetInstance();
    ColorCycle& color_cycle = ColorCycle::GetInstance();

    StartFromWhiteClockFace();
    color_cycle.SetIsActive(true);

    const Pixel start = display.GetColorCycle();
    bool red_led = false;
    bool green_led = false;
    bool blue_led = false;
    bool every_one_saturated = true;
    bool none_washed_out = true;

    /* 256 steps, which is the round: the hue is a byte and closes by overflowing. */
    for(uint16_t step = 0u; step < 256u; step++) {
        const Pixel shown = display.GetColorCycle();
        const byte red = shown.GetRed();
        const byte green = shown.GetGreen();
        const byte blue = shown.GetBlue();

        // One channel at full is the outside edge of the wheel, one channel dark is what
        // keeps it from being a paler white - together they are "saturated".
        if((red != 255u) && (green != 255u) && (blue != 255u)) { every_one_saturated = false; }
        if((red != 0u) && (green != 0u) && (blue != 0u)) { none_washed_out = false; }

        if((red > green) && (red > blue)) { red_led = true; }
        if((green > red) && (green > blue)) { green_led = true; }
        if((blue > red) && (blue > green)) { blue_led = true; }

        color_cycle.Task();
    }

    Expect(every_one_saturated, "every colour of the round must have a channel at full");
    Expect(none_washed_out, "and one at zero, or it is a pale white rather than a hue");
    Expect(red_led && green_led && blue_led, "the round must pass through all three primaries");
    Expect(ArePixelsSame(start, display.GetColorCycle()), "and 256 steps must close it");

    color_cycle.SetIsActive(false);
    StartFromWhiteClockFace();
}

/*
 * A dark display is not a display to cycle. What this protects is not the arithmetic but the
 * supply switch: it waits for a pixel buffer that has settled, and a wheel stepping through
 * the night would hold that wait open for as long as the night lasts.
 */
void TestColourCycleStandsStillOnADarkDisplay()
{
    Display& display = Display::GetInstance();
    ColorCycle& color_cycle = ColorCycle::GetInstance();

    StartFromWhiteClockFace();
    color_cycle.SetIsActive(true);
    color_cycle.Task();

    const byte hue_when_lit = color_cycle.GetHue();

    display.Disable();
    color_cycle.Task();
    color_cycle.Task();
    Expect(color_cycle.GetHue() == hue_when_lit, "a display that is off must not step the wheel");

    display.Enable();
    color_cycle.Task();
    Expect(color_cycle.GetHue() != hue_when_lit, "and it must carry on where it left off once it is back");

    color_cycle.SetIsActive(false);
    StartFromWhiteClockFace();
}

/*
 * The one place where getting this wrong is not cosmetic. The supply cap is computed from a
 * colour, and with the cycle running the colour on the strip is not the colour in the
 * setting - so a cap taken from the setting would let a saturated hue draw against the
 * budget of whatever pale value happens to be stored underneath it.
 */
void TestColourCycleBudgetsTheColourItShows()
{
    Display& display = Display::GetInstance();
    ColorCycle& color_cycle = ColorCycle::GetInstance();

    StartFromWhiteClockFace();

    /*
     * A tenth of one channel: the cheapest colour there is, and one this display draws so
     * little of that the cap lifts entirely. Every pixel lit, so the count is not what makes
     * the difference below.
     */
    display.SetColor(25u, 0u, 0u);
    display.Test();
    Expect(GetMaster() == DisplayCurrentLimit::kNoLimit, "a tenth of one channel must run uncapped");

    /*
     * Stepped to a corner of the wheel rather than to a chosen hue: two channels near full is
     * where a saturated colour costs enough for the cap to bite, and which hue that is follows
     * from the conversion rather than from a number written down here.
     */
    color_cycle.SetIsActive(true);
    for(uint16_t step = 0u; step < 256u; step++) {
        const Pixel shown = display.GetColorCycle();

        if(DisplayCurrentLimit::ToChannelSum(shown.GetRed(), shown.GetGreen(), shown.GetBlue()) >= 500u) { break; }
        color_cycle.Task();
    }

    const Pixel shown = display.GetColorCycle();
    const uint16_t shown_sum = DisplayCurrentLimit::ToChannelSum(shown.GetRed(), shown.GetGreen(), shown.GetBlue());
    const byte shown_limit = DisplayCurrentLimit::ToBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, shown_sum);

    Expect(shown_sum >= 500u, "the wheel must reach a colour with two channels near full");
    Expect(shown_limit < DisplayCurrentLimit::kNoLimit, "which on a full display is more than the budget allows");
    Expect(GetMaster() == shown_limit, "so the cap must follow the colour the cycle is showing");
    // The failure this guards: budgeting the stored colour would leave the strip uncapped
    // while it draws twenty times what that colour would.
    Expect(GetMaster() != DisplayCurrentLimit::kNoLimit, "rather than the tenth of a channel that is merely set");

    color_cycle.SetIsActive(false);
    Expect(GetMaster() == DisplayCurrentLimit::kNoLimit, "and lift again when the cycle stops");

    StartFromWhiteClockFace();
}
