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
 *                  it checks is only unlatched before the first task.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <array>
#include <cstdlib>
#include <iostream>

#include "Animations.h"
#include "Clock.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "Scheduler.h"

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

} // namespace

/******************************************************************************************************************************************************
 * M A I N
******************************************************************************************************************************************************/
int main()
{
    testDisplayManagerLatch();
    testSchedulerSpeedRoundTrip();
    testWordsChangeOnFiveMinuteStepsOnly();
    testInvalidTimeIsRejected();
    testClockModes();
    testClockWordsComparison();
    testPixelColorChannels();

    if(Failures == 0) {
        std::cout << "All Wordclock tests passed.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
