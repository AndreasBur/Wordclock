// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the explode-pixels animation.
 */

#define _ANIMATION_CLOCK_IMPLODE_SOURCE_

#include "AnimationExplodePixels.h"
#include "Display.h"
#include "DisplayWords.h"
#include "Clock.h"

void AnimationExplodePixels::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationExplodePixels::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        Display::GetInstance().SetPixel(kColumnCenter, kRowCenter);
        shift_counter_ = 0u;
        state_ = StateType::kClearTime;
    }
    return return_value;
}

void AnimationExplodePixels::Task()
{
    if(state_ == StateType::kClearTime) {
        if(shift_counter_ < kShiftCounterMaxValue) {
            shift_counter_++;
            ClearTimeTask();
        } else {
            SetStateToSetTime();
        }
    }
    if(state_ == StateType::kSetTime) {
        if(shift_counter_ >= 2u) {
            shift_counter_--;
            SetTimeTask();
        } else {
            FinishWithClockWords(clock_words_table_);
        }
    }
}

void AnimationExplodePixels::Reset()
{
    shift_counter_ = 0u;
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
}

void AnimationExplodePixels::ClearTimeTask()
{
    ShiftQuadrantUpperLeft();
    ShiftQuadrantLowerRight();
    ShiftQuadrantUpperRight();
    ShiftQuadrantLowerLeft();
}

void AnimationExplodePixels::SetTimeTask()
{
    DisplayPixels pixels;
    Display::GetInstance().Clear();
    SetTime(pixels);
    ShiftQuadrants(pixels, shift_counter_);
    Display::GetInstance().SetPixel(kColumnCenter, kRowCenter);
}

void AnimationExplodePixels::SetStateToSetTime()
{
    state_ = StateType::kSetTime;
    Display::GetInstance().Clear();
    shift_counter_ = kShiftCounterMaxValue;
}

void AnimationExplodePixels::ShiftQuadrants(DisplayPixels& pixels, byte number_of_shifts)
{
    for(uint8_t row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        for(uint8_t column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(pixels.GetPixel(column, row)) {
                byte column_next = column, row_next = row;
                if(column < kColumnCenter) column_next = ShiftRight(column, number_of_shifts);
                if(column > kColumnCenter) column_next = ShiftLeft(column, number_of_shifts);
                if(row < kRowCenter) row_next = ShiftDown(row, number_of_shifts);
                if(row > kRowCenter) row_next = ShiftUp(row, number_of_shifts);
                SetNewPixel(column, row, column_next, row_next);
            }
        }
    }
}

void AnimationExplodePixels::ShiftQuadrantUpperLeft()
{
    for(int8_t column = kColumnCenter; column >= 0; column--) {
        for(int8_t row = kRowCenter; row >= 0; row--) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftDownRight(column, row); }
        }
    }
}

void AnimationExplodePixels::ShiftDownRight(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column < kColumnCenter) column_next++;
    if(row < kColumnCenter) row_next++;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

void AnimationExplodePixels::ShiftQuadrantUpperRight()
{
    for(uint8_t column = kColumnCenter + 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = kRowCenter - 1u; row >= 0; row--) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftDownLeft(column, row); }
        }
    }
}

void AnimationExplodePixels::ShiftDownLeft(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column > kColumnCenter) column_next--;
    if(row < kColumnCenter) row_next++;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

void AnimationExplodePixels::ShiftQuadrantLowerLeft()
{
    for(int8_t column = kColumnCenter - 1u; column >= 0; column--) {
        for(int8_t row = kRowCenter + 1u; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftUpRight(column, row); }
        }
    }
}

void AnimationExplodePixels::ShiftUpRight(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column < kColumnCenter) column_next++;
    if(row > kColumnCenter) row_next--;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

void AnimationExplodePixels::ShiftQuadrantLowerRight()
{
    for(uint8_t column = kColumnCenter; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = kRowCenter; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftUpLeft(column, row); }
        }
    }
}

void AnimationExplodePixels::ShiftUpLeft(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column > kColumnCenter) column_next--;
    if(row > kColumnCenter) row_next--;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

void AnimationExplodePixels::ClearOldAndSetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new)
{
    if((column_old != column_new) || (row_old != row_new))
    {
        Display::GetInstance().ClearPixel(column_old, row_old);
        Display::GetInstance().SetPixel(column_new, row_new);
    }
}

void AnimationExplodePixels::SetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new)
{
    if((column_old != column_new) || (row_old != row_new))
    {
        Display::GetInstance().SetPixel(column_new, row_new);
    }
}
