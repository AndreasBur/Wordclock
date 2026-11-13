// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the flicker animation.
 */

#include "AnimationFlicker.h"
#include "Display.h"
#include "Clock.h"

void AnimationFlicker::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationFlicker::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        // A running flicker takes the newer time and ends on it, rather than starting over,
        // as every other animation does.
        hour_ = hour;
        minute_ = minute;
        if(state_ == StateType::kIdle) {
            display_was_enabled_ = Display::GetInstance().IsEnabled();
            state_ = StateType::kClearTime;
            return_value = kEOk;
        }
    }
    return return_value;
}

void AnimationFlicker::Task()
{
    if(state_ == StateType::kClearTime) { ClearTimeTask(); }
    else if(state_ == StateType::kSetTime) { SetTimeTask(); }
}

void AnimationFlicker::Reset()
{
    /*
     * The flicker is made by switching the display off and on, and it ends on an off:
     * ClearTimeTask() toggles first and only then decides whether it was the last run, so
     * the final toggle is always the one that darkens. Nothing switched it back, which
     * left a clock that had shown this animation dark until something else happened to
     * turn it on.
     *
     * Here rather than at the end of the animation, because Reset() is also where an
     * abort lands - a flicker cut short in the middle had the same ending.
     *
     * Only what the flicker switched off, which is_clock_set_ says: Init() runs before every
     * run and on every switch of animation, and an Enable() regardless lit a display that was
     * switched off by hand or for the night - and the night switch then gave its supply back.
     */
    if(is_clock_set_) { Display::GetInstance().Enable(); }

    minute_ = 0u;
    hour_ = 0u;
    is_clock_set_ = false;
    flicker_counter_ = ANIMATION_FLICKER_COUNTER_INIT_VALUE;
}

void AnimationFlicker::ClearTimeTask()
{
    // A display that was off when the flicker started stays off; it only counts its steps.
    if(display_was_enabled_) {
        is_clock_set_ = !is_clock_set_;

        if(is_clock_set_) {
            Display::GetInstance().Disable();
        } else {
            Display::GetInstance().Enable();
        }
    }

    if(flicker_counter_-- <= 0u) state_ = StateType::kSetTime;
}

void AnimationFlicker::SetTimeTask()
{
    /* before Reset(), which clears Hour and Minute */
    FinishWithTime(hour_, minute_);
    Reset();
}
