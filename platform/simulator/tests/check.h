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

inline int failures{0};

inline void Expect(bool condition, const char* description)
{
    if(condition) { return; }

    std::cerr << "FAILED: " << description << '\n';
    failures++;
}

inline ClockWords WordsAt(byte hour, byte minute)
{
    ClockWords words;
    Expect(Clock::GetInstance().GetClockWords(hour, minute, words) == kEOk,
           "valid time must produce clock words");
    return words;
}

using PixelBufferType = std::array<Pixels::PixelType, PIXELS_NUMBER_OF_PIXELS>;

inline PixelBufferType ReadPixels()
{
    PixelBufferType pixels;

    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        pixels[index] = Pixels::GetInstance().GetPixel(index);
    }
    return pixels;
}

/* Pixel has no operator==, and comparing the three channels is all this needs. */
inline bool ArePixelsEqual(const PixelBufferType& left, const PixelBufferType& right)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(left[index].GetRed()   != right[index].GetRed()   ||
           left[index].GetGreen() != right[index].GetGreen() ||
           left[index].GetBlue()  != right[index].GetBlue()) { return false; }
    }
    return true;
}

inline bool IsAnyPixelLit(const PixelBufferType& pixels)
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        if(pixels[index].GetRed() != 0u || pixels[index].GetGreen() != 0u ||
           pixels[index].GetBlue() != 0u) { return true; }
    }
    return false;
}

/* What would reach the LEDs, which is not what readPixels() shows: that reads the buffer,
   and the brightness is folded in on the way out. The difference is the whole of what
   switching the display off does, so a test that asked the buffer would see nothing. */
inline bool IsAnyOutputPixelLit()
{
    for(byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
        const Pixels::PixelType pixel = Pixels::GetInstance().GetOutputPixel(index);

        if(pixel.GetRed() != 0u || pixel.GetGreen() != 0u || pixel.GetBlue() != 0u) { return true; }
    }
    return false;
}

inline void SetTime(byte hour, byte minute, byte second)
{
    RealTimeClock::GetInstance().SetTime(ClockTime(hour, minute, second));
}

/* More than one case needs a clock face on the display before it can say what changed
   about it, so it lives here rather than in whichever file got it first. */
/* Draws a time the way the clock does when no animation is selected: the reference every
   animation has to arrive at. */
inline PixelBufferType DrawClockFace(byte hour, byte minute)
{
    Display& display = Display::GetInstance();

    display.Clear();
    Expect(Clock::GetInstance().SetTime(hour, minute) == kEOk, "clock face time must be accepted");
    display.Show();
    return ReadPixels();
}

#endif // _CHECK_H_
