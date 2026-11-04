// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The base class of all animations, with the state and helpers they share.
 */

#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"

/* Animation configuration parameter */

/* Animation parameter */

class Animation
{
  public:
    enum class StateType {
        kNone,
        kUninit,
        kIdle,
        kClearTime,
        kSetTime
    };

  private:

  protected:
    /* odd factor of the hash in SelectByTime(), see there */
    static constexpr byte kSelectHashFactor{181u};

    StateType state_{StateType::kNone};

    // functions
    void Init(StateType state) { state_ = state; }
    /*
     * Every animation whose set phase places the letters itself ends through one of
     * these two instead of just going idle: drawing the target once more makes the last
     * frame authoritative, so a pixel the animation skipped or hit twice cannot leave a
     * word of the previous time behind. AnimationFade is the one animation outside this,
     * because its set phase only ramps the brightness of letters already in place.
     */
    void FinishWithClockWords(const ClockWords::WordsListType& clock_words_table) {
        Display::GetInstance().Clear();
        Clock::GetInstance().SetTime(clock_words_table);
        state_ = StateType::kIdle;
    }
    /* For the animations that kept the time rather than the word list. */
    void FinishWithTime(byte hour, byte minute) {
        Display::GetInstance().Clear();
        Clock::GetInstance().SetTime(hour, minute);
        state_ = StateType::kIdle;
    }
    static byte SelectByTime(byte, byte, byte);

  public:
    constexpr Animation() : state_{StateType::kNone} { }
    ~Animation() { }

    // get methods
    StateType GetState() const { return state_; }

    // set methods

    // methods
    static StdReturnType Show() { return Display::GetInstance().Show(); }
    static bool IsPixelPartOfClockWords(ClockWords::WordsListType, byte, byte);
    static bool IsPixelPartOfClockWords(ClockWords::WordsListType, byte);
};

#endif
