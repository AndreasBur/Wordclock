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

/* The colour the strip is actually carrying, read off the first lit pixel. The buffer holds
   the colour already dimmed by the brightness, so the cases below set a brightness that
   leaves the channels recognisable rather than comparing against exact values. */
Pixel firstLitPixel()
{
    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_PIXELS; Index++) {
        const Pixel Candidate = Pixels::getInstance().getPixel(Index);

        if(Candidate.getRed() || Candidate.getGreen() || Candidate.getBlue()) { return Candidate; }
    }
    return Pixel(0u, 0u, 0u);
}

bool arePixelsSame(Pixel Left, Pixel Right)
{
    return (Left.getRed() == Right.getRed()) && (Left.getGreen() == Right.getGreen())
        && (Left.getBlue() == Right.getBlue());
}

byte getMaster() { return Pixels::getInstance().getBrightness(); }

/* A lit clock face at full brightness, the cycle off, and a colour that is nobody's hue on
   the wheel - so a pixel carrying the wheel's colour cannot be mistaken for one carrying the
   setting. Also what every case here has to leave behind, white included, because the cases
   after this file start from a white display. */
void startFromWhiteClockFace()
{
    Display& display = Display::getInstance();

    ColorCycle::getInstance().setIsActive(false);
    display.enable();
    display.setBrightness(255u);
    display.setColor(255u, 255u, 255u);
    Clock::getInstance().setTime(10u, 5u);
    expect(display.show() == E_OK, "the clock face must reach the strip");
}

} // namespace

/* The whole point of the cycle being a level of its own: what it paints is on the strip, and
   what somebody chose is still what the clock answers and what Persistence would store. */
void testColourCycleLeavesTheChosenColourAlone()
{
    Display& display = Display::getInstance();
    ColorCycle& colorCycle = ColorCycle::getInstance();

    startFromWhiteClockFace();
    /* Deliberately not on the wheel: full saturation is what the cycle paints, so a colour
       with three unequal channels cannot be one of its. */
    display.setColor(200u, 120u, 60u);
    const Pixel Chosen = firstLitPixel();

    colorCycle.setIsActive(true);
    const Pixel FirstOfTheWheel = firstLitPixel();
    expect(!arePixelsSame(Chosen, FirstOfTheWheel), "switching the cycle on must change what the strip carries");
    expect(display.getColorRed() == 200u && display.getColorGreen() == 120u && display.getColorBlue() == 60u,
           "and must not touch the colour that was chosen");

    /* A step of the wheel, which is one task run - and a visible one, since what is being
       checked is that the strip follows it. */
    colorCycle.task();
    expect(!arePixelsSame(FirstOfTheWheel, firstLitPixel()), "a step of the wheel must reach the strip");
    expect(display.getColorRed() == 200u, "and still must not touch the setting");

    colorCycle.setIsActive(false);
    expect(arePixelsSame(Chosen, firstLitPixel()), "switching it off must put the chosen colour back");
    /* Nothing was remembered to put back: the setting was never overwritten, which is what
       makes this work rather than a saved copy somewhere. */
    expect(!display.isColorCycleActive(), "with no cycle level left over the top");

    startFromWhiteClockFace();
}

/* What the wheel is: a full round of saturated colours. Checked through the module rather
   than against the conversion, because the property is what reaches the strip. */
void testColourCycleWalksTheWholeWheel()
{
    Display& display = Display::getInstance();
    ColorCycle& colorCycle = ColorCycle::getInstance();

    startFromWhiteClockFace();
    colorCycle.setIsActive(true);

    const Pixel Start = display.getColorCycle();
    bool RedLed = false;
    bool GreenLed = false;
    bool BlueLed = false;
    bool EveryOneSaturated = true;
    bool NoneWashedOut = true;

    /* 256 steps, which is the round: the hue is a byte and closes by overflowing. */
    for(uint16_t Step = 0u; Step < 256u; Step++) {
        const Pixel Shown = display.getColorCycle();
        const byte Red = Shown.getRed();
        const byte Green = Shown.getGreen();
        const byte Blue = Shown.getBlue();

        /* One channel at full is the outside edge of the wheel, one channel dark is what
           keeps it from being a paler white - together they are "saturated". */
        if((Red != 255u) && (Green != 255u) && (Blue != 255u)) { EveryOneSaturated = false; }
        if((Red != 0u) && (Green != 0u) && (Blue != 0u)) { NoneWashedOut = false; }

        if((Red > Green) && (Red > Blue)) { RedLed = true; }
        if((Green > Red) && (Green > Blue)) { GreenLed = true; }
        if((Blue > Red) && (Blue > Green)) { BlueLed = true; }

        colorCycle.task();
    }

    expect(EveryOneSaturated, "every colour of the round must have a channel at full");
    expect(NoneWashedOut, "and one at zero, or it is a pale white rather than a hue");
    expect(RedLed && GreenLed && BlueLed, "the round must pass through all three primaries");
    expect(arePixelsSame(Start, display.getColorCycle()), "and 256 steps must close it");

    colorCycle.setIsActive(false);
    startFromWhiteClockFace();
}

/* A dark display is not a display to cycle. What this protects is not the arithmetic but the
   supply switch: it waits for a pixel buffer that has settled, and a wheel stepping through
   the night would hold that wait open for as long as the night lasts. */
void testColourCycleStandsStillOnADarkDisplay()
{
    Display& display = Display::getInstance();
    ColorCycle& colorCycle = ColorCycle::getInstance();

    startFromWhiteClockFace();
    colorCycle.setIsActive(true);
    colorCycle.task();

    const byte HueWhenLit = colorCycle.getHue();

    display.disable();
    colorCycle.task();
    colorCycle.task();
    expect(colorCycle.getHue() == HueWhenLit, "a display that is off must not step the wheel");

    display.enable();
    colorCycle.task();
    expect(colorCycle.getHue() != HueWhenLit, "and it must carry on where it left off once it is back");

    colorCycle.setIsActive(false);
    startFromWhiteClockFace();
}

/* The one place where getting this wrong is not cosmetic. The supply cap is computed from a
   colour, and with the cycle running the colour on the strip is not the colour in the
   setting - so a cap taken from the setting would let a saturated hue draw against the
   budget of whatever pale value happens to be stored underneath it. */
void testColourCycleBudgetsTheColourItShows()
{
    Display& display = Display::getInstance();
    ColorCycle& colorCycle = ColorCycle::getInstance();

    startFromWhiteClockFace();

    /* A tenth of one channel: the cheapest colour there is, and one this display draws so
       little of that the cap lifts entirely. Every pixel lit, so the count is not what makes
       the difference below. */
    display.setColor(25u, 0u, 0u);
    display.test();
    expect(getMaster() == DisplayCurrentLimit::NoLimit, "a tenth of one channel must run uncapped");

    /* Stepped to a corner of the wheel rather than to a chosen hue: two channels near full is
       where a saturated colour costs enough for the cap to bite, and which hue that is follows
       from the conversion rather than from a number written down here. */
    colorCycle.setIsActive(true);
    for(uint16_t Step = 0u; Step < 256u; Step++) {
        const Pixel Shown = display.getColorCycle();

        if(DisplayCurrentLimit::toChannelSum(Shown.getRed(), Shown.getGreen(), Shown.getBlue()) >= 500u) { break; }
        colorCycle.task();
    }

    const Pixel Shown = display.getColorCycle();
    const uint16_t ShownSum = DisplayCurrentLimit::toChannelSum(Shown.getRed(), Shown.getGreen(), Shown.getBlue());
    const byte ShownLimit = DisplayCurrentLimit::toBrightnessLimit(PIXELS_NUMBER_OF_LEDS, PIXELS_NUMBER_OF_LEDS, ShownSum);

    expect(ShownSum >= 500u, "the wheel must reach a colour with two channels near full");
    expect(ShownLimit < DisplayCurrentLimit::NoLimit, "which on a full display is more than the budget allows");
    expect(getMaster() == ShownLimit, "so the cap must follow the colour the cycle is showing");
    /* The failure this guards: budgeting the stored colour would leave the strip uncapped
       while it draws twenty times what that colour would. */
    expect(getMaster() != DisplayCurrentLimit::NoLimit, "rather than the tenth of a channel that is merely set");

    colorCycle.setIsActive(false);
    expect(getMaster() == DisplayCurrentLimit::NoLimit, "and lift again when the cycle stops");

    startFromWhiteClockFace();
}
