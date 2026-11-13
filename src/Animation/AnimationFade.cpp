// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the fade animation.
 */

#include "AnimationFade.h"
#include "Display.h"
#include "Clock.h"

void AnimationFade::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationFade::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        // A running fade takes the newer time and ends on it, rather than starting over, as
        // every other animation does.
        hour_ = hour;
        minute_ = minute;
        if(state_ == StateType::kIdle) {
            state_ = StateType::kClearTime;
            // From the display as it is configured, whatever that currently is - the fade
            // scales it rather than replacing it.
            level_ = kLevelFull;
            return_value = kEOk;
        }
    }
    return return_value;
}

void AnimationFade::Task()
{
    if(state_ == StateType::kClearTime) { ClearTimeTask(); }
    else if(state_ == StateType::kSetTime) { SetTimeTask(); }
}

/*
 * Back to a display nobody is fading. Called from Init(), which is also what an aborted
 * animation goes through - a fade that was cut off half way would otherwise leave the
 * display dimmed with nothing left to undim it.
 */
void AnimationFade::Reset()
{
    level_ = kLevelFull;
    hour_ = 0u;
    minute_ = 0u;
    Display::GetInstance().ClearBrightnessFade();
}

void AnimationFade::ClearTimeTask()
{
    if(level_ > 0u) {
        level_--;
        Display::GetInstance().SetBrightnessFadeLevel(level_);
    } else {
        SetStateToSetTime();
    }
}

void AnimationFade::SetTimeTask()
{
    if(level_ < kLevelFull) {
        level_++;
        Display::GetInstance().SetBrightnessFadeLevel(level_);
    } else {
        /*
         * Cleared rather than left at full: a level that is set at all costs a comparison
         * on every ApplyBrightness(), and leaving it set would hide a fade that never
         * finished.
         */
        Display::GetInstance().ClearBrightnessFade();
        // Drawn again rather than left as the swap drew it, for a newer time that came in
        // while the fade rose. At full brightness and before the next Show(), nobody sees it.
        FinishWithTime(hour_, minute_);
    }
}

/*
 * Swaps the time behind a display that is dark. The clear is what was missing: setting the
 * new time writes its letters, it does not take the previous ones away, so both were lit at
 * once - and at the moment of the swap the display is black, which is the one moment where
 * nobody can see it happen.
 */
void AnimationFade::SetStateToSetTime()
{
    state_ = StateType::kSetTime;
    Display::GetInstance().Clear();
    Clock::GetInstance().SetTime(hour_, minute_);
    level_ = 0u;
}
