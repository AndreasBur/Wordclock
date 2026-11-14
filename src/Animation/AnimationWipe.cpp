// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the wipe animation.
 */

#include "AnimationWipe.h"

#include "Clock.h"
#include "Display.h"
#include "DisplayWords.h"

void AnimationWipe::Init() {
  Animation::Init(StateType::kIdle);
  Reset();
}

StdReturnType AnimationWipe::SetTime(byte hour, byte minute) {
  StdReturnType return_value{kENotOk};

  if (Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk &&
      state_ == StateType::kIdle) {
    return_value = kEOk;
    state_ = StateType::kClearTime;
  }
  return return_value;
}

void AnimationWipe::Task() {
  if (state_ == StateType::kClearTime) {
    ClearTimeTask();
  } else if (state_ == StateType::kSetTime) {
    SetTimeTask();
  }
}

void AnimationWipe::Reset() {
  clock_words_table_.fill(DisplayWords::WordIdType::kNone);
  index_ = 0u;
  set_pixel_state_ = SetPixelStateType::kDown;
}

void AnimationWipe::ClearTimeTask() {
  byte column;
  byte row;

  Display::GetInstance().IndexToColumnAndRow(index_, column, row);

  do {
    if (Display::GetInstance().GetPixel(column, row)) {
      if (set_pixel_state_ == SetPixelStateType::kDown) {
        SetPixelDown(column, row);
      } else {
        SetPixelRight(column, row);
      }
    }
  } while (column-- != 0u && row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

  if (set_pixel_state_ == SetPixelStateType::kDown) {
    set_pixel_state_ = SetPixelStateType::kRight;
  } else {
    set_pixel_state_ = SetPixelStateType::kDown;
  }

  if (SetNextIndex() == kENotOk) {
    index_ = 0u;
    state_ = StateType::kSetTime;
  }
}

void AnimationWipe::SetTimeTask() {
  byte column;
  byte row;

  Display::GetInstance().IndexToColumnAndRow(index_, column, row);

  do {
    if (IsPixelPartOfClockWords(clock_words_table_, column, row)) {
      Display::GetInstance().SetPixel(column, row);
    }
  } while (column-- != 0u && row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

  if (SetNextIndex() == kENotOk) {
    FinishWithClockWords(clock_words_table_);
  }
}

StdReturnType AnimationWipe::SetNextIndex() {
  StdReturnType return_value = kEOk;
  byte column;
  byte row;

  Display::GetInstance().IndexToColumnAndRow(index_, column, row);

  if (column < DISPLAY_NUMBER_OF_COLUMNS - 1u)
    column++;
  else if (row < DISPLAY_NUMBER_OF_ROWS - 1u)
    row++;
  else
    return_value = kENotOk;

  index_ = Display::GetInstance().ColumnAndRowToIndex(column, row);
  return return_value;
}

void AnimationWipe::SetPixelDown(byte column, byte row) {
  Display::GetInstance().ClearPixel(column, row);

  for (byte row_next = row + 1u; row_next < DISPLAY_NUMBER_OF_ROWS; row_next++) {
    if (!Display::GetInstance().GetPixel(column, row_next)) {
      Display::GetInstance().SetPixel(column, row_next);
      break;
    }
  }
}

void AnimationWipe::SetPixelRight(byte column, byte row) {
  Display::GetInstance().ClearPixel(column, row);

  for (byte column_next = column + 1u; column_next < DISPLAY_NUMBER_OF_COLUMNS; column_next++) {
    if (!Display::GetInstance().GetPixel(column_next, row)) {
      Display::GetInstance().SetPixel(column_next, row);
      break;
    }
  }
}
