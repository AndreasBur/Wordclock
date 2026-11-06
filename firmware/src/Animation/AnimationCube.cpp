// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the cube animation.
 */

#define _ANIMATION_CUBE_SOURCE_

#include "AnimationCube.h"
#include "StandardTypes.h"
#include "Display.h"
#include "Clock.h"

void AnimationCube::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationCube::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        SetMaxBorder();
        state_ = StateType::kClearTime;
    }
    return return_value;
}

void AnimationCube::Task()
{
    if(state_ == StateType::kClearTime) { ClearTimeTask(); }
    else if(state_ == StateType::kSetTime) { SetTimeTask(); }
}

void AnimationCube::Reset()
{
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
    SetMinBorder();
}

void AnimationCube::SetStateToSetTime()
{
    ClearBorderPixels();
    state_ = StateType::kSetTime;
    SetMinBorder();
}

void AnimationCube::ClearTimeTask()
{
    switch(border_state_)
    {
        case BorderStateType::kMax:
            SetBorderPixels();
            border_state_ = BorderStateType::kBetween;
            break;
        case BorderStateType::kBetween:
            ClearBorderPixels();
            if(DecreaseBorder() == kENotOk) { SetStateToSetTime(); } 
            else { SetBorderPixels(); }
            break;
        default:
            break;
    }
}

void AnimationCube::SetTimeTask()
{
    switch(border_state_)
    {
        case BorderStateType::kMin:
            SetBorderPixels();
            border_state_ = BorderStateType::kBetween;
            break;
        case BorderStateType::kBetween:
            ClearBorderPixelsWithoutClockPixels();
            if(IncreaseBorder() == kENotOk) { FinishWithClockWords(clock_words_table_); }
            else { SetBorderPixels(); }
            break;
        default:
            break;
    }
}

void AnimationCube::ClearBorderPixelsWithoutClockPixels()
{
    for(byte column = border_.column_start; column <= border_.column_end; column++) {
        if(IsPixelPartOfClockWords(clock_words_table_, column, border_.row_start) == false) {
            // clear border top
            Display::GetInstance().ClearPixel(column, border_.row_start);
        }
        if(IsPixelPartOfClockWords(clock_words_table_, column, border_.row_end) == false) {
            // clear border bottom
            Display::GetInstance().ClearPixel(column, border_.row_end);
        }
    }
    for(byte row = border_.row_start; row <= border_.row_end; row++) {
        if(IsPixelPartOfClockWords(clock_words_table_, border_.column_start, row) == false) {
            // clear border left
            Display::GetInstance().ClearPixel(border_.column_start, row);
        }
        if(IsPixelPartOfClockWords(clock_words_table_, border_.column_end, row) == false) {
            // clear border right
            Display::GetInstance().ClearPixel(border_.column_end, row);
        }
    }
}

void AnimationCube::WriteBorderPixels(bool value) const
{
    for(byte column = border_.column_start; column <= border_.column_end; column++) {
        // set border top
        Display::GetInstance().WritePixel(column, border_.row_start, value);
        // set border bottom
        Display::GetInstance().WritePixel(column, border_.row_end, value);
    }
    for(byte row = border_.row_start; row <= border_.row_end; row++) {
        // set border left
        Display::GetInstance().WritePixel(border_.column_start, row, value);
        // set border right
        Display::GetInstance().WritePixel(border_.column_end, row, value);
    }
}

StdReturnType AnimationCube::IncreaseBorder()
{
    StdReturnType return_value = kEOk;

    if(border_.column_start > 0u) { border_.column_start--; }
    else { return_value = kENotOk; }
    if(border_.column_end < DISPLAY_NUMBER_OF_COLUMNS - 1) { border_.column_end++; }
    else { return_value = kENotOk; }
    if(border_.row_start > 0u) { border_.row_start--; }
    else { return_value = kENotOk; }
    if(border_.row_end < DISPLAY_NUMBER_OF_ROWS - 1) { border_.row_end++; }
    else { return_value = kENotOk; }

    return return_value;
}

StdReturnType AnimationCube::DecreaseBorder()
{
    StdReturnType return_value = kEOk;

    if(border_.column_start < ANIMATION_CUBE_COLUMN_START_MAX_VALUE) { border_.column_start++; }
    else { return_value = kENotOk; }
    if(border_.column_end > ANIMATION_CUBE_COLUMN_END_MIN_VALUE) {border_. column_end--; }
    else { return_value = kENotOk; }
    if(border_.row_start < ANIMATION_CUBE_ROW_START_MAX_VALUE) { border_.row_start++; }
    else { return_value = kENotOk; }
    if(border_.row_end > ANIMATION_CUBE_ROW_END_MIN_VALUE) { border_.row_end--; }
    else { return_value = kENotOk; }

    return return_value;
}

void AnimationCube::SetMaxBorder()
{
    border_.column_start = 0u;
    border_.column_end = DISPLAY_NUMBER_OF_COLUMNS - 1u;
    border_.row_start = 0u;
    border_.row_end = DISPLAY_NUMBER_OF_ROWS - 1u;
    border_state_ = BorderStateType::kMax;
}

void AnimationCube::SetMinBorder()
{
    border_.column_start = ANIMATION_CUBE_COLUMN_START_MAX_VALUE;
    border_.column_end = ANIMATION_CUBE_COLUMN_END_MIN_VALUE;
    border_.row_start = ANIMATION_CUBE_ROW_START_MAX_VALUE;
    border_.row_end = ANIMATION_CUBE_ROW_END_MIN_VALUE;
    border_state_ = BorderStateType::kMin;
}
