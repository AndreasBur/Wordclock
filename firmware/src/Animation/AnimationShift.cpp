// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the shift animation.
 */

#define _ANIMATION_SHIFT_SOURCE_

#include "AnimationShift.h"
#include "Display.h"
#include "Clock.h"

void AnimationShift::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationShift::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        state_ = StateType::kClearTime;
    }
    return return_value;
}

void AnimationShift::Task()
{
    if(state_ == StateType::kClearTime) { ClearTimeTask(); }
    else if(state_ == StateType::kSetTime) { SetTimeTask(); }
}

void AnimationShift::Reset()
{
#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON)
    current_column_ = 0u;
#endif

#if (ANIMATION_SHIFT_VERTICAL == STD_ON)
    current_row_ = 0u;
#endif
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
}

void AnimationShift::ClearTimeTask()
{
#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON)
    if(current_column_ < DISPLAY_NUMBER_OF_COLUMNS) {
        wc_transformation_.ShiftRight();
        current_column_++;
    } else {
        state_ = StateType::kSetTime;
        current_column_ = 0u;
    }
#endif

#if (ANIMATION_SHIFT_VERTICAL == STD_ON)
    if(current_row_ < DISPLAY_NUMBER_OF_ROWS) {
        wc_transformation_.ShiftDown();
        current_row_++;
    } else {
        state_ = StateType::kSetTime;
        current_row_ = 0u;
    }
#endif

}

void AnimationShift::SetTimeTask()
{
#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON)
    if(current_column_ < DISPLAY_NUMBER_OF_COLUMNS) {
        wc_transformation_.ShiftRight();
        for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
            if(IsPixelPartOfClockWords(clock_words_table_, DISPLAY_NUMBER_OF_COLUMNS - current_column_ - 1u, row)) {
                Display::GetInstance().SetPixel(0u, row);
            }
        }
        current_column_++;
    } else {
        FinishWithClockWords(clock_words_table_);
    }
#endif

#if (ANIMATION_SHIFT_VERTICAL == STD_ON)
    if(current_row_ < DISPLAY_NUMBER_OF_ROWS) {
        wc_transformation_.ShiftDown();
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(IsPixelPartOfClockWords(clock_words_table_, column, DISPLAY_NUMBER_OF_ROWS - current_row_ - 1u)) {
                Display::GetInstance().SetPixel(column, 0u);
            }
        }
        current_row_++;
    } else {
        FinishWithClockWords(clock_words_table_);
    }
#endif
}
