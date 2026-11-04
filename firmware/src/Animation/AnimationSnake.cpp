// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the snake animation.
 */

#define _ANIMATION_CLOCK_SNAKE_SOURCE_

#include "AnimationSnake.h"
#include "Bit.h"

void AnimationSnake::Init()
{
    Animation::Init(StateType::kIdle);
    Reset();
}

StdReturnType AnimationSnake::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        return_value = kEOk;
        snake_begin_index_ = 0u;
        snake_end_index_ = 0u;
        state_ = StateType::kSetTime;
    }
    return return_value;
}

void AnimationSnake::Task()
{
    if(state_ == StateType::kSetTime) {
        byte snake_end_index_trans = TransformToSerpentine(snake_end_index_);
        Display::GetInstance().SetPixel(TransformToSerpentine(snake_begin_index_));

        if((snake_begin_index_ - snake_end_index_) == ANIMATION_SNAKE_LENGTH ||
           (snake_begin_index_ >= DISPLAY_NUMBER_OF_LEDS - 1u && snake_end_index_ < DISPLAY_NUMBER_OF_LEDS))
        {
            if(IsPixelPartOfClockWords(clock_words_table_, snake_end_index_trans) == false) Display::GetInstance().ClearPixel(snake_end_index_trans);
            snake_end_index_++;
        }
        if(snake_begin_index_ < DISPLAY_NUMBER_OF_LEDS - 1u) snake_begin_index_++;
        if(snake_end_index_ >= DISPLAY_NUMBER_OF_LEDS) FinishWithClockWords(clock_words_table_);
    }
}

void AnimationSnake::Reset()
{
    snake_begin_index_ = 0u;
    snake_end_index_ = 0u;
    snake_state_ = SnakeStateType::kToRight;
}

byte AnimationSnake::TransformToSerpentine(byte column, byte row)
{
    byte index;

    if(!Bit<byte>::ReadBit(row, 0u)) index = (row * DISPLAY_NUMBER_OF_COLUMNS) + column;
    else index = (row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - column - 1u);

    return index;
}

byte AnimationSnake::TransformToSerpentine(byte index)
{
    byte column = Display::GetInstance().IndexToColumn(index);
    byte row = Display::GetInstance().IndexToRow(index);

    return TransformToSerpentine(column, row);
}
