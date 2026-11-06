// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the cursor animation.
 */

#define _ANIMATION_CLOCK_CURSOR_SOURCE_

#include "AnimationCursor.h"
#include "Display.h"
#include "Clock.h"

void AnimationCursor::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationCursor::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        current_pixel_index_ = 0u;
        state_ = StateType::kSetTime;
    }
    return return_value;
}

void AnimationCursor::Task()
{
    if(state_ == StateType::kSetTime) {
        if(current_pixel_index_ < DISPLAY_NUMBER_OF_PIXELS) { Display::GetInstance().SetPixel(current_pixel_index_); }
        if(current_pixel_index_ > 0u) {
            if(IsPixelPartOfClockWords(clock_words_table_, current_pixel_index_ - 1u) == false) {
                Display::GetInstance().ClearPixel(current_pixel_index_ - 1u);
            }
        }
        if(current_pixel_index_ >= DISPLAY_NUMBER_OF_PIXELS) {
            /*
             * The cursor normally clears every old pixel as it passes. Rebuild the
             * target once at the end as well, so a skipped or overlapping pixel can
             * never leave a word from the previous time on the display.
             */
            FinishWithClockWords(clock_words_table_);
        }
        current_pixel_index_++;
    }
}

void AnimationCursor::Reset()
{
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
    current_pixel_index_ = 0u;
}
