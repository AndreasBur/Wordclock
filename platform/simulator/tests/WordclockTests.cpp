/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       WordclockTests.cpp
 *      \brief      Regression tests for the firmware core that run without a display
 *
 *      \details    Covers DisplayManager's word-set latch and the two rules it rests on:
 *                  that the word set is a pure function of mode, hour and minute, and
 *                  that it changes exactly on the five-minute steps. Also pins down
 *                  Pixel's colour channels, which no display here can reveal but the
 *                  LED strip can.
 *
 *                  Nothing here needs a display: the pixel buffer lives in Pixels and
 *                  the window that renders it is a separate PixelsFrame, which is never
 *                  constructed. testDisplayManagerLatch() runs first, because the latch
 *                  it checks is only unlatched before the first task, and testPersistence()
 *                  runs last, because it leaves settings of its own behind and writes to
 *                  the real store in the working directory.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <array>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayCharacters.h"
#include "DisplayManager.h"
#include "Illuminance.h"
#include "Overlays.h"
#include "Temperature.h"
#include "Text.h"
#include "sim/DS3231.h"
#include "Persistence.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "Scheduler.h"
#include "sim/Storage.h"

/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N S
******************************************************************************************************************************************************/
namespace {

int Failures{0};

void expect(bool Condition, const char* Description)
{
    if(Condition) { return; }

    std::cerr << "FAILED: " << Description << '\n';
    Failures++;
}

ClockWords wordsAt(byte Hour, byte Minute)
{
    ClockWords words;
    expect(Clock::getInstance().getClockWords(Hour, Minute, words) == E_OK,
           "valid time must produce clock words");
    return words;
}

using PixelBufferType = std::array<Pixels::PixelType, PIXELS_NUMBER_OF_PIXELS>;

PixelBufferType readPixels()
{
    PixelBufferType pixels;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        pixels[index] = Pixels::getInstance().getPixelFast(index);
    }
    return pixels;
}

/* Pixel has no operator==, and comparing the three channels is all this needs. */
bool arePixelsEqual(const PixelBufferType& Left, const PixelBufferType& Right)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(Left[index].getRed()   != Right[index].getRed()   ||
           Left[index].getGreen() != Right[index].getGreen() ||
           Left[index].getBlue()  != Right[index].getBlue()) { return false; }
    }
    return true;
}

bool isAnyPixelLit(const PixelBufferType& Pixels)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(Pixels[index].getRed() != 0u || Pixels[index].getGreen() != 0u ||
           Pixels[index].getBlue() != 0u) { return true; }
    }
    return false;
}

void setTime(byte Hour, byte Minute, byte Second)
{
    RealTimeClock::getInstance().setTime(ClockTime(Hour, Minute, Second));
}

/* The latch itself: the display is redrawn when the word set changes and left alone
   when it does not. Pixels::isDirty() is what makes "left alone" observable at all - the
   buffer staying equal would not tell a skipped redraw from one that rewrote the same
   words. Must run before any other test touches DisplayManager. */
void testDisplayManagerLatch()
{
    Clock::getInstance().setModeFast(Clock::MODE_WESSI);
    /* Without an animation the word change draws straight to the buffer, so what the
       latch decided is visible in the same task. */
    Animations::getInstance().setModeFast(Animations::MODE_FIXED);
    Animations::getInstance().setAnimationFast(Animations::ANIMATION_ID_NONE);

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
    expect(strcmp(overlays.getTemperatureString(), "21.5C") == 0,
           "the overlay must show the dialled-in reading");
    expect(overlays.abort() == E_OK, "the overlay must end again");

    /* Left as it was found: an active temperature overlay would take the display away
       from whatever runs after this. */
    overlays.setTemperatureIsActive(false);
    DS3231::clearSimulatedTemperature();
    Temperature::getInstance().task();
}



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

/* Draws a time the way the clock does when no animation is selected: the reference every
   animation has to arrive at. */
PixelBufferType drawClockFace(byte Hour, byte Minute)
{
    Display& display = Display::getInstance();

    display.clear();
    Clock::getInstance().setTime(Hour, Minute);
    display.show();
    return readPixels();
}

/* The one promise every animation makes, and the one that is easy to break: whatever it
   does in between, the display it leaves behind is the new time and nothing else.
   Animation::finishWithClockWords() exists because that was got wrong once - an animation
   that places letters itself can leave a pixel of the previous time lit, and on a word
   clock that is a word nobody notices is wrong.
   The second promise is that they end at all. Both are checked for every animation there
   is, so an added one is covered by being added to the enumeration. */
void testEveryAnimationEndsOnTheNewTime()
{
    /* A generous bound rather than a tight one: what it is here for is a runaway, not a
       measurement. The longest of them walks all 110 pixels several times over. */
    constexpr int TickLimit{5000};

    Clock::getInstance().setModeFast(Clock::MODE_WESSI);
    Animations& animations = Animations::getInstance();
    animations.setModeFast(Animations::MODE_FIXED);

    /* Two times whose word sets differ in every part: hour, minute and the "it is". */
    const PixelBufferType Target = drawClockFace(10u, 35u);

    for(byte Id = 1u; Id < Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS; Id++) {
        const Animations::AnimationIdType AnimationId = static_cast<Animations::AnimationIdType>(Id);
        char Description[64];

        /* The display holds the previous time when a word change starts, which is what
           the animations transition away from. */
        drawClockFace(10u, 4u);
        animations.setAnimationFast(AnimationId);
        animations.setTime(10u, 35u);

        int Ticks = 0;
        while((animations.getState() == Animations::STATE_PENDING) && (Ticks < TickLimit)) {
            animations.task(true);
            Ticks++;
        }

        snprintf(Description, sizeof(Description), "animation %u must come to an end", Id);
        expect(Ticks < TickLimit, Description);

        snprintf(Description, sizeof(Description), "animation %u must leave the new time behind", Id);
        expect(arePixelsEqual(readPixels(), Target), Description);
    }

    animations.setAnimationFast(Animations::ANIMATION_ID_NONE);
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

    animations.setAnimationFast(Animations::ANIMATION_ID_FADE);
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

    animations.setAnimationFast(Animations::ANIMATION_ID_NONE);
}


/* Which characters the text overlay can draw, checked through the one entry point that
   says so: setChar() refuses what it cannot map to a glyph.
   The font tables hold 102 entries - ASCII 0x20 to 0x7F, then the six umlauts - and the
   conversion used to check only the lower bound. Where char is unsigned, which is what AVR
   makes it, every other Latin-1 byte passed that check and indexed past the end of the
   table. The host cannot show that: char is signed here, so those bytes come out negative
   and are refused for the wrong reason. The case is checked anyway, because it is the one
   a port would meet. */
void testCharacterToGlyphMapping()
{
    Text& text = Text::getInstance();

    for(char Character = ' '; Character < '\x7F'; Character++) {
        if(text.setChar(0u, 0u, Character, Text::FONT_5X8) != E_OK) {
            expect(false, "every printable character must have a glyph");
            break;
        }
    }

    expect(text.setChar(0u, 0u, '\n', Text::FONT_5X8) == E_NOT_OK, "a control character has no glyph");
    expect(text.setChar(0u, 0u, '\x1F', Text::FONT_5X8) == E_NOT_OK, "and neither has the last one below the space");

    /* The umlauts, which sit behind the ASCII range in every table. */
    const char Umlauts[] = {'\xC4', '\xD6', '\xDC', '\xE4', '\xF6', '\xFC'};
    for(const char Umlaut : Umlauts) {
        if(text.setChar(0u, 0u, Umlaut, Text::FONT_5X8) != E_OK) {
            expect(false, "every umlaut must have a glyph");
            break;
        }
    }

    expect(text.setChar(0u, 0u, '\xE0', Text::FONT_5X8) == E_NOT_OK,
           "a Latin-1 byte that is not one of the six must be refused");

    /* Every font carries the same character set, and a glyph nobody can see would be a
       table read as the wrong packing. */
    for(byte Font = 0u; Font < Text::FONT_NUMBER_OF_FONTS; Font++) {
        const Text::FontType FontType = static_cast<Text::FontType>(Font);
        char Description[64];

        snprintf(Description, sizeof(Description), "font %u must have a width for a letter", Font);
        expect(text.getFontCharWidth(FontType, 'A') > 0u, Description);

        snprintf(Description, sizeof(Description), "font %u must be able to draw a letter", Font);
        expect(text.setChar(0u, 0u, 'A', FontType) == E_OK, Description);
    }

    Display::getInstance().clear();
}


/* Whether a word is part of what the clock would light for a time, which is what the
   regional wordings differ in. */
bool wordsContain(const ClockWords& Words, DisplayWords::WordIdType Word)
{
    for(const DisplayWords::WordIdType Id : Words.getWordsList()) {
        if(Id == Word) { return true; }
    }
    return false;
}

/* The four regional wordings, at the four times where they part company. This is the table
   in docs/serial-commands.md, asserted rather than described: the modes differ in how the
   quarters are said and in whether twenty past is counted from the hour or from the half
   hour, and in nothing else.
   What makes it worth pinning is that all four share one pair of tables, indexed by mode -
   a row edited in the wrong one is a clock that is wrong in one region and right in the
   other three. */
void testRegionalWordings()
{
    Clock& clock = Clock::getInstance();

    /* Quarter past four: named after the hour it is past, or after the one it counts
       towards. */
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_NACH),
           "Wessi says a quarter *past* four");
    clock.setModeFast(Clock::MODE_OSSI);
    expect(!wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_NACH),
           "Ossi says viertel five, with no past in it");
    expect(wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_VIERTEL),
           "and it is still a quarter");

    /* Quarter to five: the same difference, from the other side. */
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_VOR),
           "Wessi says a quarter *to* five");
    clock.setModeFast(Clock::MODE_SCHWABEN);
    expect(wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_DREIVIERTEL),
           "Schwaben says three quarters five");
    expect(!wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_VOR),
           "which has no to in it");

    /* Twenty past four: counted from the hour, or as ten before half five. */
    clock.setModeFast(Clock::MODE_RHEIN_RUHR);
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_NACH),
           "Rhein-Ruhr counts twenty past the hour");
    expect(!wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_HALB),
           "and does not reach for the half hour");
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_HALB),
           "Wessi counts ten before half five");
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_VOR),
           "which is a before");

    /* Twenty to five, the mirror of it. */
    clock.setModeFast(Clock::MODE_RHEIN_RUHR);
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_VOR),
           "Rhein-Ruhr counts twenty to the hour");
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_HALB),
           "Wessi counts ten past half five");
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_NACH),
           "which is a past");

    /* And what none of them differ in: the full hour and the half hour always say "it is".
       Whether the times between them do is the one thing about the wording that is a
       setting, so both of its answers are asked here rather than whichever one the build
       happens to carry. */
    clock.setModeFast(Clock::MODE_WESSI);
    const bool ShowItIsPermanentlyBefore = clock.getShowItIsPermanently();

    clock.setShowItIsPermanently(false);
    expect(wordsAt(4u, 0u).getShowItIs(), "the full hour says it is");
    expect(wordsAt(4u, 30u).getShowItIs(), "and so does the half hour");
    expect(!wordsAt(4u, 5u).getShowItIs(), "five past does not");
    expect(!wordsAt(4u, 45u).getShowItIs(), "and neither does a quarter to");

    clock.setShowItIsPermanently(true);
    expect(wordsAt(4u, 5u).getShowItIs(), "with it is set permanently, five past says it too");
    expect(wordsAt(4u, 45u).getShowItIs(), "and so does a quarter to");
    expect(wordsAt(4u, 0u).getShowItIs(), "and the full hour still does");

    clock.setShowItIsPermanently(ShowItIsPermanentlyBefore);

    clock.setModeFast(Clock::MODE_WESSI);
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

/* Only the round trip. That speed 1 maps to cycle 255 and speed 255 to cycle 1 is
   asserted at compile time in Scheduler.h, so repeating those points here would test
   the compiler rather than the code. */
void testSchedulerSpeedRoundTrip()
{
    for(unsigned int speed = 0u; speed <= UINT8_MAX; speed++) {
        const byte originalSpeed = static_cast<byte>(speed);
        const byte cycle = Scheduler::convertSpeedToTaskCycle(originalSpeed);
        expect(Scheduler::convertTaskCycleToSpeed(cycle) == originalSpeed,
               "speed and task cycle conversion must round-trip");
    }
}

/* The premise of driving the display off the word set instead of off the minute: a
   word change happens on every five-minute step and on none of the minutes between,
   over the whole day rather than at a few sample times. */
void testWordsChangeOnFiveMinuteStepsOnly()
{
    Clock::getInstance().setModeFast(Clock::MODE_WESSI);

    ClockWords previousWords = wordsAt(0u, 0u);

    for(byte hour = 0u; hour < 24u; hour++) {
        for(byte minute = 0u; minute < 60u; minute++) {
            if(hour == 0u && minute == 0u) { continue; }

            const ClockWords words = wordsAt(hour, minute);
            const bool isStep = (minute % 5u) == 0u;

            if(isStep) {
                expect(words != previousWords, "every five-minute step must change the words");
            } else {
                expect(words == previousWords, "minutes inside a step must leave the words alone");
            }
            previousWords = words;
        }
    }
}

void testInvalidTimeIsRejected()
{
    Clock& clock = Clock::getInstance();
    ClockWords invalidWords;

    expect(clock.getClockWords(24u, 0u, invalidWords) == E_NOT_OK,
           "hour 24 must be rejected");
    expect(clock.getClockWords(10u, 60u, invalidWords) == E_NOT_OK,
           "minute 60 must be rejected");
}

void testClockModes()
{
    Clock& clock = Clock::getInstance();
    clock.setModeFast(Clock::MODE_WESSI);
    const ClockWords wessiWords = wordsAt(10u, 15u);

    clock.setModeFast(Clock::MODE_OSSI);
    expect(wessiWords != wordsAt(10u, 15u),
           "changing the clock mode must change the words where modes differ");

    clock.setModeFast(Clock::MODE_WESSI);
    expect(wessiWords == wordsAt(10u, 15u),
           "the same mode and time must produce the same words again");
}

/* The comparison the display update hangs on: it has to notice a difference in each
   of the three fields, or a word change goes unnoticed and the display stays behind. */
void testClockWordsComparison()
{
    const ClockWords words = wordsAt(10u, 15u);

    expect(words == words, "clock words must equal themselves");

    ClockWords differentItIs = words;
    differentItIs.setShowItIs(!words.getShowItIs());
    expect(differentItIs != words, "a different \"it is\" must compare unequal");

    ClockWords differentHour = words;
    ClockWords::HourWordsType hourWords = words.getHourWords();
    hourWords[0] = (hourWords[0] == DisplayWords::WORD_NONE) ? DisplayWords::WORD_ES
                                                            : DisplayWords::WORD_NONE;
    differentHour.setHourWords(hourWords);
    expect(differentHour != words, "a different hour word must compare unequal");

    ClockWords differentMinute = words;
    ClockWords::MinutesWordsType minuteWords = words.getMinuteWords();
    minuteWords[0] = (minuteWords[0] == DisplayWords::WORD_NONE) ? DisplayWords::WORD_ES
                                                                : DisplayWords::WORD_NONE;
    differentMinute.setMinuteWords(minuteWords);
    expect(differentMinute != words, "a different minute word must compare unequal");
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
    expect(characters.getCharacterFast(0u) == 'E', "index 0 must be the first letter of the first row");
    expect(characters.getCharacterFast(2u) == 'K', "index 2 must be the third letter of the first row");
    expect(characters.getCharacterFast(0u, 0u) == 'E', "column 0, row 0 must be the first letter");

    /* Column 10 is the one a transposed lookup read past the end of the table for. */
    expect(characters.getCharacterFast(10u, 0u) == 'F', "the last column of the first row must be reachable");
    expect(characters.getCharacterFast(10u) == 'F', "index 10 must be the last letter of the first row");

    /* Last row is "BSECHSFMUHR", so the very last letter is its R. */
    constexpr byte lastIndex{DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS - 1u};
    expect(characters.getCharacterFast(lastIndex) == 'R', "the last index must be the last letter");
    expect(characters.getCharacterFast(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS - 1u,
                                      DISPLAY_CHARACTERS_NUMBER_OF_ROWS - 1u) == 'R',
           "the last column of the last row must be the last letter");

    /* Every position has to read the same whether it is asked for by index or by column
       and row - that is what the two broken entry points disagreed on. */
    bool allAgree = true;
    for(byte index = 0u; index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; index++) {
        byte column, row;
        characters.indexToColumnAndRow(index, column, row);

        if(characters.getCharacterFast(index) != characters.getCharacterFast(column, row)) { allAgree = false; }
        if(characters.columnAndRowToIndex(column, row) != index) { allAgree = false; }
    }
    expect(allAgree, "index and column/row lookups must agree on every position");

    char character{'\0'};
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u, character) == E_NOT_OK,
           "a column past the last one must be rejected");
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS, character) == E_NOT_OK,
           "an index past the last one must be rejected");
}

/* Reaches around Storage on purpose, which nothing else may: damaging a stored blob is
   the one thing the store's own interface will not do, and rejecting a damaged one is
   what has to be shown. */
bool damageStoredChecksum()
{
    std::array<byte, Storage::Capacity> blob{};
    size_t size{0u};

    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
    if(file == nullptr) { return false; }
    size = std::fread(blob.data(), 1u, blob.size(), file);
    std::fclose(file);

    if(size == 0u) { return false; }

    /* The last byte, which is behind every field the format names - so this flip leaves
       magic and version intact and is caught only if the checksum really covers the whole
       blob rather than the fields it happens to know about. */
    blob[size - 1u] = static_cast<byte>(blob[size - 1u] + 1u);

    file = std::fopen(STORAGE_FILE_NAME, "wb");
    if(file == nullptr) { return false; }
    const size_t written = std::fwrite(blob.data(), 1u, size, file);
    std::fclose(file);

    return written == size;
}

/* The whole persistence path, on the store the simulator really writes to. Runs last
   because it leaves the clock and the display on values of its own, and it clears the
   store afterwards so a second run starts where this one did. */
void testPersistence()
{
    Storage& storage = Storage::getInstance();
    Persistence& persistence = Persistence::getInstance();
    Display& display = Display::getInstance();
    Clock& clock = Clock::getInstance();

    expect(storage.clear() == E_OK, "clearing an empty store must succeed");
    expect(persistence.load() == E_NOT_OK, "an empty store must report that nothing was restored");

    display.setColor(10u, 20u, 30u);
    display.setBrightness(123u);
    display.setBrightnessUseGammaCorrection(true);
    clock.setModeFast(Clock::MODE_SCHWABEN);
    Animations::getInstance().setTaskCycleFast(Animations::ANIMATION_ID_NONE, 42u);
    Illuminance::getInstance().setCalibrationValuesMaxValue(4321u);

    persistence.task();

    /* Away from every stored value, so a restore that did nothing cannot pass. */
    display.setColor(1u, 2u, 3u);
    display.setBrightness(7u);
    display.setBrightnessUseGammaCorrection(false);
    clock.setModeFast(Clock::MODE_WESSI);
    Animations::getInstance().setTaskCycleFast(Animations::ANIMATION_ID_NONE, 1u);
    Illuminance::getInstance().setCalibrationValuesMaxValue(1u);

    expect(persistence.load() == E_OK, "a stored configuration must be restored");
    expect(display.getColorRed() == 10u && display.getColorGreen() == 20u && display.getColorBlue() == 30u,
           "the colour must come back");
    expect(display.getBrightness() == 123u, "the brightness must come back");
    expect(display.getBrightnessUseGammaCorrection(), "the gamma correction switch must come back");
    expect(clock.getMode() == Clock::MODE_SCHWABEN, "the clock mode must come back");
    expect(Animations::getInstance().getTaskCycle(Animations::ANIMATION_ID_NONE) == 42u,
           "an animation speed must come back");
    expect(Illuminance::getInstance().getCalibrationValuesMaxValue() == 4321u,
           "the sensor calibration must come back");

    /* Nothing changed since that restore, so nothing may be written - which is the whole
       reason there is no dirty flag to forget. */
    expect(storage.clear() == E_OK, "clearing a written store must succeed");
    persistence.task();
    expect(persistence.load() == E_NOT_OK, "an unchanged configuration must not have been written");

    /* And a blob whose checksum no longer fits must be refused rather than applied. */
    display.setColor(11u, 22u, 33u);
    persistence.task();
    expect(damageStoredChecksum(), "the test must be able to damage the stored blob");
    display.setColor(4u, 5u, 6u);
    expect(persistence.load() == E_NOT_OK, "a blob with a wrong checksum must be refused");
    expect(display.getColorRed() == 4u, "a refused blob must leave the settings alone");

    expect(storage.clear() == E_OK, "the test must leave the store as it found it");
}

/* The two procedures that reach the store directly. What they are for is what task()
   cannot do: write before the plug is pulled rather than within the next period, and get
   a clock back to what it left the factory as without erasing its flash over USB. */
void testPersistenceSaveAndReset()
{
    Storage& storage = Storage::getInstance();
    Persistence& persistence = Persistence::getInstance();
    Display& display = Display::getInstance();
    Clock& clock = Clock::getInstance();

    display.setColor(1u, 2u, 3u);
    clock.setModeFast(Clock::MODE_OSSI);
    clock.setShowItIsPermanently(false);
    expect(persistence.save() == E_OK, "saving must write the configuration");

    display.setColor(9u, 9u, 9u);
    clock.setShowItIsPermanently(true);
    expect(persistence.load() == E_OK, "what was saved must come back");
    expect(display.getColorRed() == 1u, "and it must be what was saved");
    expect(!clock.getShowItIsPermanently(), "including the it is rule");

    /* The overlays, which used to be the one configured thing a power cut took away. The
       text goes through the store as a whole buffer, so a shorter one written after a
       longer one must not leave the tail of the longer behind. */
    Overlays& overlays = Overlays::getInstance();

    overlays.setTextText("Herzlichen Glueckwunsch", 24u);
    (void)overlays.setTextPeriodInMinutes(17u);
    (void)overlays.setTextEnduranceInSeconds(9u);
    overlays.setTextMonth(7u);
    overlays.setTextDay(23u);
    overlays.setTextValidInDays(3u);
    overlays.setTextSpeed(4u);
    (void)overlays.setTextFont(Text::FONT_5X8);
    overlays.setTextIsActive(true);
    (void)overlays.setDatePeriodInMinutes(5u);
    overlays.setDateIsActive(true);

    expect(persistence.save() == E_OK, "saving must write the overlays too");

    overlays.setTextText("x", 2u);
    (void)overlays.setTextPeriodInMinutes(1u);
    overlays.setTextMonth(0u);
    overlays.setTextSpeed(1u);
    (void)overlays.setTextFont(Text::FONT_10X10);
    overlays.setTextIsActive(false);
    overlays.setDateIsActive(false);

    expect(persistence.load() == E_OK, "the stored overlays must come back");
    expect(strcmp(overlays.getTextText(), "Herzlichen Glueckwunsch") == 0,
           "the overlay text must come back whole");
    expect(overlays.getTextPeriodInMinutes() == 17u, "and its period");
    expect(overlays.getTextEnduranceInSeconds() == 9u, "and its endurance");
    expect(overlays.getTextMonth() == 7u && overlays.getTextDay() == 23u, "and its date");
    expect(overlays.getTextValidInDays() == 3u, "and how long that date stays valid");
    expect(overlays.getTextSpeed() == 4u, "and its speed");
    expect(overlays.getTextFont() == Text::FONT_5X8, "and its font");
    expect(overlays.getTextIsActive(), "and whether it is switched on");
    expect(overlays.getDatePeriodInMinutes() == 5u && overlays.getDateIsActive(),
           "a second overlay must not have been read out of the first one's slot");

    expect(persistence.reset() == E_OK, "resetting must empty the store");
    expect(overlays.getTextText()[0] == '\0', "the reset must empty the overlay text");
    expect(overlays.getTextPeriodInMinutes() == 1u && overlays.getTextSpeed() == 1u,
           "the reset must put the overlay settings back");
    expect(!overlays.getTextIsActive() && !overlays.getDateIsActive(),
           "the reset must switch every overlay off again");
    expect(display.getColorRed() == 255u && display.getColorGreen() == 255u && display.getColorBlue() == 255u,
           "the reset must put the colour back to white");
    expect(display.getBrightness() == 255u, "the reset must put the brightness back");
    /* Qualified because the macro expands to the bare enumerator, and asked through the
       macro rather than through the mode it happens to name today. */
    expect(clock.getMode() == Clock::CLOCK_INITIAL_MODE, "the reset must put the clock mode back");
    expect(Animations::getInstance().getAnimation() == Animations::ANIMATION_ID_NONE,
           "the reset must deselect the animation");

    /* The store stays empty rather than being refilled with the defaults on the next
       period - an empty store is what a clock that was never configured has. */
    persistence.task();
    expect(persistence.load() == E_NOT_OK, "the task after a reset must leave the store empty");

    expect(storage.clear() == E_OK, "the test must leave the store as it found it");
}

} // namespace

/******************************************************************************************************************************************************
 * M A I N
******************************************************************************************************************************************************/
int main()
{
    testDisplayManagerLatch();
    testShowNowProcedures();
    testTemperatureOverlayWithoutSensor();
    testEveryAnimationEndsOnTheNewTime();
    testFadeDimsAndComesBack();
    testCharacterToGlyphMapping();
    testRegionalWordings();
    testOverlayPeriodAndEndurance();
    testSchedulerSpeedRoundTrip();
    testWordsChangeOnFiveMinuteStepsOnly();
    testInvalidTimeIsRejected();
    testClockModes();
    testClockWordsComparison();
    testPixelColorChannels();
    testDisplayCharacterLookup();
    testPersistence();
    testPersistenceSaveAndReset();

    if(Failures == 0) {
        std::cout << "All Wordclock tests passed.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
