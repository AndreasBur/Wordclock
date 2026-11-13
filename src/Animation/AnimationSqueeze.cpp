// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the squeeze animation.
 */

#include "AnimationSqueeze.h"
#include "Display.h"
#include "DisplayWords.h"
#include "Clock.h"

void AnimationSqueeze::Init() {
  Animation::Init(StateType::kIdle);
  Reset();
}

StdReturnType AnimationSqueeze::SetTime(byte hour, byte minute) {
  StdReturnType return_value{kENotOk};

  if (Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk &&
      state_ == StateType::kIdle) {
    return_value = kEOk;
    direction_ = static_cast<DirectionType>(SelectByTime(hour, minute, kNumberOfDirections));
    state_ = StateType::kClearTime;
  }
  return return_value;
}

void AnimationSqueeze::Task() {
  if (state_ == StateType::kClearTime) {
    ClearTimeTask();
    if (Display::GetInstance().IsCleared()) {
      SetMaxWordLength();
      state_ = StateType::kSetTime;
    }
  } else if (state_ == StateType::kSetTime) {
    SetTimeTask();
    if (current_length_ > max_word_length_) {
      FinishWithClockWords(clock_words_table_);
    }
  }
}

void AnimationSqueeze::Reset() {
  clock_words_table_.fill(DisplayWords::WordIdType::kNone);
  max_word_length_ = 0u;
  current_length_ = 1u;
  direction_ = DirectionType::kFromLeft;
}

void AnimationSqueeze::ClearTimeTask() {
  /*
   * Row by row, not over the linear pixel index: on the index a block at the end of
   * one row and a block at the start of the next count as one, which makes the
   * result depend on where the rows wrap.
   */
  for (byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
    ClearRow(row);
  }
}

void AnimationSqueeze::ClearRow(byte row) {
  // Takes one letter off every block of the row, on the side the direction says, so
  // all words of the row get shorter at the same time.
  if (direction_ == DirectionType::kFromLeft) {
    for (byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
      if (Display::GetInstance().GetPixel(column, row)) {
        Display::GetInstance().ClearPixel(column, row);

        /* skip the rest of this block, only its first letter goes this frame */
        while ((column + 1u < DISPLAY_NUMBER_OF_COLUMNS) &&
               Display::GetInstance().GetPixel(column + 1u, row)) {
          column++;
        }
      }
    }
  } else {
    for (int8_t column = DISPLAY_NUMBER_OF_COLUMNS - 1; column >= 0; column--) {
      if (Display::GetInstance().GetPixel(static_cast<byte>(column), row)) {
        Display::GetInstance().ClearPixel(static_cast<byte>(column), row);

        while ((column - 1 >= 0) &&
               Display::GetInstance().GetPixel(static_cast<byte>(column - 1), row)) {
          column--;
        }
      }
    }
  }
}

void AnimationSqueeze::SetTimeTask() {
  Clock::GetInstance().SetTime(clock_words_table_, current_length_);
  current_length_++;
}

void AnimationSqueeze::SetMaxWordLength() {
  for (byte index = 0u; index < clock_words_table_.size(); index++) {
    byte current_word_length = words_.GetDisplayWordLength(clock_words_table_[index]);
    if (current_word_length > max_word_length_) {
      max_word_length_ = current_word_length;
    }
  }
}
