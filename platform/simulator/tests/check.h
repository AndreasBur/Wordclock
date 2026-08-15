/* The assertion these tests share, its tally, and the fixtures more than one case needs.

   Deliberately quiet on a pass, which is the opposite of the ESP32 tests' check() - and
   for the reason given there: ctest wraps these and counts them, so a passing run has
   nothing to say, while nothing wraps the ESP32 binaries and a silent one would leave no
   evidence that anything ran.

   inline throughout, so every case file can include this and still link: one variable and
   one function, however many translation units. */
#ifndef _CHECK_H_
#define _CHECK_H_

#include <array>
#include <iostream>

#include "Clock.h"
#include "Display.h"
#include "Pixels.h"
#include "RealTimeClock.h"

inline int Failures{0};

inline void expect(bool Condition, const char* Description)
{
    if(Condition) { return; }

    std::cerr << "FAILED: " << Description << '\n';
    Failures++;
}

inline ClockWords wordsAt(byte Hour, byte Minute)
{
    ClockWords words;
    expect(Clock::getInstance().getClockWords(Hour, Minute, words) == E_OK,
           "valid time must produce clock words");
    return words;
}

using PixelBufferType = std::array<Pixels::PixelType, PIXELS_NUMBER_OF_PIXELS>;

inline PixelBufferType readPixels()
{
    PixelBufferType pixels;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        pixels[index] = Pixels::getInstance().getPixelFast(index);
    }
    return pixels;
}

/* Pixel has no operator==, and comparing the three channels is all this needs. */
inline bool arePixelsEqual(const PixelBufferType& Left, const PixelBufferType& Right)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(Left[index].getRed()   != Right[index].getRed()   ||
           Left[index].getGreen() != Right[index].getGreen() ||
           Left[index].getBlue()  != Right[index].getBlue()) { return false; }
    }
    return true;
}

inline bool isAnyPixelLit(const PixelBufferType& Pixels)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(Pixels[index].getRed() != 0u || Pixels[index].getGreen() != 0u ||
           Pixels[index].getBlue() != 0u) { return true; }
    }
    return false;
}

inline void setTime(byte Hour, byte Minute, byte Second)
{
    RealTimeClock::getInstance().setTime(ClockTime(Hour, Minute, Second));
}

/* More than one case needs a clock face on the display before it can say what changed
   about it, so it lives here rather than in whichever file got it first. */
/* Draws a time the way the clock does when no animation is selected: the reference every
   animation has to arrive at. */
inline PixelBufferType drawClockFace(byte Hour, byte Minute)
{
    Display& display = Display::getInstance();

    display.clear();
    Clock::getInstance().setTime(Hour, Minute);
    display.show();
    return readPixels();
}

#endif // _CHECK_H_
