// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the drop animation.
 */

#define _ANIMATION_CLOCK_DROP_SOURCE_

#include "AnimationDrop.h"
#include "Display.h"
#include "Clock.h"

void AnimationDrop::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationDrop::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        SetNextWordIndex();
        if(SetNextActivePixelIndex() == kENotOk) { SetStateToSetTime(); }
        else { state_ = StateType::kClearTime; }
    }
    return return_value;
}

void AnimationDrop::Task()
{
    if(state_ == StateType::kClearTime) { ClearTimeTask(); }
    else if(state_ == StateType::kSetTime) { SetTimeTask(); }
}

void AnimationDrop::Reset()
{
    row_ = 0u;
    column_ = 0u;
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
    current_word_index_ = clock_words_table_.size() - 1u;
}

void AnimationDrop::ClearTimeTask()
{
    // toggle current Pixel
    if(row_ < DISPLAY_NUMBER_OF_ROWS && column_ < DISPLAY_NUMBER_OF_COLUMNS) { Display::GetInstance().TogglePixel(column_, row_); }
    // increment row and check for out of bounds
    if(row_ + 1u < DISPLAY_NUMBER_OF_ROWS) {
        // toggle Pixel in next row
        row_++;
        Display::GetInstance().TogglePixel(column_, row_);
    } else {
        // no more active pixels available
        if(SetNextActivePixelIndex() == kENotOk) { SetStateToSetTime(); }
    }
}

void AnimationDrop::SetTimeTask()
{
    DisplayWord current_word = words_.GetDisplayWord(clock_words_table_[current_word_index_]);
    const byte max_column = words_.GetDisplayWordColumn(clock_words_table_[current_word_index_]) + current_word.GetLength() - 1u;

    if(SetNextRow(current_word.GetRow()) == kEOk) {
        Display::GetInstance().ClearPixel(column_, row_ - 1u);
    } else {
        if(SetNextColumn(max_column) == kENotOk) {
            FinishWithClockWords(clock_words_table_);
            Reset();
            return;
        }
    }
    Display::GetInstance().SetPixel(column_, row_);
}

StdReturnType AnimationDrop::SetNextActivePixelIndex()
{
    for(int16_t index = DISPLAY_NUMBER_OF_PIXELS - 1u; index >= 0; index--) {
        if(Display::GetInstance().GetPixel(index)) {
            Display::GetInstance().IndexToColumnAndRow(index, column_, row_);
            return kEOk;
        }
    }
    return kENotOk;
}

StdReturnType AnimationDrop::SetNextWordIndex()
{
    /*
     * int rather than int8_t: the loop has to be able to go below zero to end, and
     * initialising a signed byte from the unsigned expression on the right got there by
     * an implementation-defined conversion rather than by arithmetic.
     */
    for(int index = static_cast<int>(current_word_index_) - 1; index >= 0; index--) {
        if(clock_words_table_[index] != DisplayWords::WordIdType::kNone) {
            current_word_index_ = index;
            return kEOk;
        }
    }
    return kENotOk;
}

void AnimationDrop::SetStateToSetTime()
{
    row_ = 0u;
    column_ = words_.GetDisplayWordColumn(clock_words_table_[current_word_index_]);
    Display::GetInstance().SetPixel(column_, row_);
    state_ = StateType::kSetTime;
}

StdReturnType AnimationDrop::SetNextRow(byte max_row)
{
    if(row_ >= max_row) {
        return kENotOk;
    } else {
        row_++;
        return kEOk;
    }
}

StdReturnType AnimationDrop::SetNextColumn(byte max_column)
{
    if(column_ >= max_column) {
         if(SetNextWordIndex() == kEOk) {
            column_ = words_.GetDisplayWordColumn(clock_words_table_[current_word_index_]);
            row_ = 0u;
            return kEOk;
        } else {
            return kENotOk;
        }
    } else {
        column_++;
        row_ = 0u;
        return kEOk;
    }
}
