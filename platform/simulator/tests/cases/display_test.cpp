/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       display_test.cpp
 *      \brief      What reaches the pixels: the word-set latch, the fade, the colour channels and the letter table
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayCharacters.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"

/* How much light the whole display is putting out, which is what a fade changes and a
   redraw does not. */
uint32_t totalIntensity(const PixelBufferType& Pixels)
{
    uint32_t Total = 0u;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        Total += Pixels[index].getRed();
        Total += Pixels[index].getGreen();
        Total += Pixels[index].getBlue();
    }
    return Total;
}

/* The latch itself: the display is redrawn when the word set changes and left alone
   when it does not. Pixels::isDirty() is what makes "left alone" observable at all - the
   buffer staying equal would not tell a skipped redraw from one that rewrote the same
   words. Must run before any other test touches DisplayManager. */
void testDisplayManagerLatch()
{
    Clock::getInstance().setMode(Clock::MODE_WESSI);
    /* Without an animation the word change draws straight to the buffer, so what the
       latch decided is visible in the same task. */
    Animations::getInstance().setMode(Animations::MODE_FIXED);
    Animations::getInstance().setAnimation(Animations::ANIMATION_ID_NONE);

    Pixels& pixels = Pixels::getInstance();
    DisplayManager& displayManager = DisplayManager::getInstance();

    setTime(10u, 4u, 0u);
    pixels.clearDirty();
    displayManager.task();
    expect(pixels.isDirty(), "the first task must draw the time it finds");

    const PixelBufferType afterFirstTask = readPixels();
    expect(isAnyPixelLit(afterFirstTask), "the first task must light some letters");

    /* Same five-minute step, one task later. */
    setTime(10u, 4u, 30u);
    pixels.clearDirty();
    displayManager.task();
    expect(!pixels.isDirty(), "an unchanged word set must not redraw");
    expect(arePixelsEqual(readPixels(), afterFirstTask),
           "an unchanged word set must leave the letters as they were");

    setTime(10u, 5u, 0u);
    pixels.clearDirty();
    displayManager.task();
    expect(pixels.isDirty(), "a word change must redraw");
    expect(!arePixelsEqual(readPixels(), afterFirstTask),
           "a word change must reach the letters");

    /* And back to standing still on the new step. */
    const PixelBufferType afterChange = readPixels();
    setTime(10u, 9u, 59u);
    pixels.clearDirty();
    displayManager.task();
    expect(!pixels.isDirty(), "the rest of the step must not redraw either");
    expect(arePixelsEqual(readPixels(), afterChange),
           "the rest of the step must leave the letters as they were");
}

/* The fade's own promise, which the display it leaves behind cannot show: that it dims on
   the way. It used to count a brightness nobody applied - the variable went down and up
   and reached no pixel - so the animation was a pause with a hard change at the end of it.
   The other half of this is what it must not touch: the brightness setting, which is what
   Persistence writes to the store. A fade that dimmed by turning the setting down would
   save its own half-way value as what the user asked for. */
void testFadeDimsAndComesBack()
{
    Animations& animations = Animations::getInstance();
    Display& display = Display::getInstance();

    const PixelBufferType Target = drawClockFace(10u, 35u);
    const byte BrightnessBefore = display.getBrightness();

    drawClockFace(10u, 4u);
    const uint32_t IntensityBefore = totalIntensity(readPixels());

    animations.setAnimation(Animations::ANIMATION_ID_FADE);
    animations.setTime(10u, 35u);

    /* Far enough in to be measurable, far short of the swap: the level counts down one per
       task from 255. */
    for(int Tick = 0; Tick < 64; Tick++) { animations.task(true); }
    expect(totalIntensity(readPixels()) < IntensityBefore, "the fade must dim what is on the display");

    int Ticks = 0;
    while((animations.getState() == Animations::STATE_PENDING) && (Ticks < 5000)) {
        animations.task(true);
        Ticks++;
    }

    expect(arePixelsEqual(readPixels(), Target), "the fade must come back to full brightness");
    expect(display.getBrightness() == BrightnessBefore, "the fade must leave the brightness setting alone");

    animations.setAnimation(Animations::ANIMATION_ID_NONE);
}

/* Switching the display off and on again, seen where it happens: the output rather than
   the buffer. disable() leaves every colour in the buffer where it was and takes the
   brightness to zero, so a check on the buffer alone would call a dark clock lit. */
void testDisplayOffAndOnAgain()
{
    Display& display = Display::getInstance();

    /* Switched on first rather than assumed on: the flicker animation leaves it off, and
       until this test there was nothing that could notice. */
    display.enable();
    display.setBrightness(120u);
    Clock::getInstance().setTime(10u, 5u);
    expect(display.show() == E_OK, "the clock face must reach the strip");
    expect(isAnyOutputPixelLit(), "a display that is on must put something out");

    const PixelBufferType Lit = readPixels();

    display.disable();
    expect(!isAnyOutputPixelLit(), "a display that is off must put nothing out");
    expect(arePixelsEqual(readPixels(), Lit),
           "and must leave the buffer alone, so switching back on needs no redraw");

    display.enable();
    expect(isAnyOutputPixelLit(), "switching back on must put something out again");
    expect(display.getBrightness() == 120u,
           "and must come back on the brightness that was set, not on full");
}

/* What "one further" means for a switch: the other one. The toggle is the whole of what a
   button on the case can send, so it has to agree with the enable and disable a phone
   sends - a toggle that only tracked its own calls would drift apart from ids 3 and 4 the
   first time both were used. */
void testDisplayToggleFollowsTheState()
{
    Display& display = Display::getInstance();
    const byte BrightnessBefore = display.getBrightness();

    display.enable();
    display.setBrightness(120u);
    Clock::getInstance().setTime(10u, 5u);
    display.show();
    expect(display.isEnabled(), "a display that was enabled must say so");

    display.toggle();
    expect(!display.isEnabled(), "a toggle must switch an enabled display off");
    expect(!isAnyOutputPixelLit(), "and must reach the strip, not only the flag");

    display.toggle();
    expect(display.isEnabled(), "a second toggle must switch it back on");
    expect(isAnyOutputPixelLit(), "and must put something out again");

    /* The other direction: a phone switching it off, a knob switching it back on. */
    display.disable();
    expect(!display.isEnabled(), "disable must be visible to the toggle too");
    display.toggle();
    expect(display.isEnabled() && isAnyOutputPixelLit(),
           "a toggle after a disable must switch the display on");

    display.setBrightness(BrightnessBefore);
}

/* The two settings that are a yes or a no, turned round by a control that cannot read
   them. Both go through the setters that apply what they set, so this also says that a
   toggle does not need a task to take effect. */
void testBrightnessTogglesInvertTheSetting()
{
    Display& display = Display::getInstance();

    display.setBrightnessUseAutomatic(false);
    display.toggleBrightnessAutomatic();
    expect(display.getBrightnessUseAutomatic(), "a toggle must switch the automatic on");
    display.toggleBrightnessAutomatic();
    expect(!display.getBrightnessUseAutomatic(), "and the next one must switch it off again");

    display.setBrightnessUseGammaCorrection(false);
    display.toggleBrightnessGammaCorrection();
    expect(display.getBrightnessUseGammaCorrection(), "a toggle must switch the gamma correction on");
    display.toggleBrightnessGammaCorrection();
    expect(!display.getBrightnessUseGammaCorrection(), "and the next one must switch it off again");
}

/* What the colour reset needs to be worth having, and what it did not have: a colour that
   reaches the strip. The pixels carry the colour already dimmed by the brightness, and that
   dimmed copy used to be recomputed only when the brightness moved - so every colour change,
   the reset and the six increment procedures alike, stayed invisible until something else
   happened to touch the brightness. A redraw did not help: it wrote the same stale copy. */
void testColourChangesReachTheStrip()
{
    Display& display = Display::getInstance();
    const byte BrightnessBefore = display.getBrightness();

    display.enable();
    display.setBrightness(200u);
    display.resetColor();
    const PixelBufferType White = drawClockFace(10u, 5u);
    expect(isAnyPixelLit(White), "a clock face must light some letters to compare");

    /* No brightness change anywhere between here and the check, which is the whole point. */
    display.setColor(255u, 0u, 0u);
    expect(!arePixelsEqual(readPixels(), White),
           "a colour change must reach the letters that are already lit");

    /* Red, and the other two channels as far down as they go: DisplayColor's dimming
       rounds a channel up to one rather than to nothing, so "off" is a one here. */
    const PixelBufferType Red = readPixels();
    bool anyLitPixelIsRed = false;
    bool everyLitPixelIsRed = true;
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(Red[index].getRed() <= 1u) { continue; }

        anyLitPixelIsRed = true;
        if(Red[index].getGreen() > 1u || Red[index].getBlue() > 1u) { everyLitPixelIsRed = false; }
    }
    expect(anyLitPixelIsRed && everyLitPixelIsRed,
           "a red display must have nothing but the rounding left in the other two channels");

    display.resetColor();
    expect(arePixelsEqual(readPixels(), White),
           "the colour reset must put white back, on the strip and not only in the setting");
    expect(display.getColorRed() == 255u && display.getColorGreen() == 255u &&
           display.getColorBlue() == 255u, "and must reach all three channels");

    display.setBrightness(BrightnessBefore);
}

/* Guards a swap that this simulator cannot show: Pixel keeps its channels in the WS2812
   wire order rather than in argument order, so a constructor that filled the raw array
   positionally put red where getRed() does not look. Every colour built that way so far
   was grey or black, which hides it, and the window renders brightness rather than hue,
   which hides it again - it would first have appeared on the strip. */
void testPixelColorChannels()
{
    const Pixel color(10u, 20u, 30u);

    expect(color.getRed() == 10u, "the first constructor argument must be red");
    expect(color.getGreen() == 20u, "the second constructor argument must be green");
    expect(color.getBlue() == 30u, "the third constructor argument must be blue");

    Pixel assigned;
    assigned.setPixel(10u, 20u, 30u);
    expect(assigned.getRed() == color.getRed() &&
           assigned.getGreen() == color.getGreen() &&
           assigned.getBlue() == color.getBlue(),
           "setPixel and the constructor must place the channels alike");

    Pixel individual;
    individual.setRed(10u);
    individual.setGreen(20u);
    individual.setBlue(30u);
    expect(individual.getRed() == color.getRed() &&
           individual.getGreen() == color.getGreen() &&
           individual.getBlue() == color.getBlue(),
           "the per-channel setters must place the channels alike");
}

/* The letter table has four entry points that all have to agree, and until now they did
   not: two of them indexed the table transposed, which for the last column read past its
   end. Nothing called any of them, so the simulator kept a second copy of the letters
   instead - which is the duplication this pins down as unnecessary. */
void testDisplayCharacterLookup()
{
    const DisplayCharacters characters;

    /* First row is "ESKISTLF" then U with an umlaut, "NF". */
    expect(characters.getCharacter(0u) == 'E', "index 0 must be the first letter of the first row");
    expect(characters.getCharacter(2u) == 'K', "index 2 must be the third letter of the first row");
    expect(characters.getCharacter(0u, 0u) == 'E', "column 0, row 0 must be the first letter");

    /* Column 10 is the one a transposed lookup read past the end of the table for. */
    expect(characters.getCharacter(10u, 0u) == 'F', "the last column of the first row must be reachable");
    expect(characters.getCharacter(10u) == 'F', "index 10 must be the last letter of the first row");

    /* Last row is "BSECHSFMUHR", so the very last letter is its R. */
    constexpr byte lastIndex{DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS - 1u};
    expect(characters.getCharacter(lastIndex) == 'R', "the last index must be the last letter");
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS - 1u,
                                      DISPLAY_CHARACTERS_NUMBER_OF_ROWS - 1u) == 'R',
           "the last column of the last row must be the last letter");

    /* Every position has to read the same whether it is asked for by index or by column
       and row - that is what the two broken entry points disagreed on. */
    bool allAgree = true;
    for(byte index = 0u; index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; index++) {
        byte column, row;
        characters.indexToColumnAndRow(index, column, row);

        if(characters.getCharacter(index) != characters.getCharacter(column, row)) { allAgree = false; }
        if(characters.columnAndRowToIndex(column, row) != index) { allAgree = false; }
    }
    expect(allAgree, "index and column/row lookups must agree on every position");

    char character{'\0'};
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u, character) == E_NOT_OK,
           "a column past the last one must be rejected");
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS, character) == E_NOT_OK,
           "an index past the last one must be rejected");
}
