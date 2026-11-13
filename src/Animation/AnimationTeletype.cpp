// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of the teletype animation.
 */

#include "AnimationTeletype.h"

#include "Clock.h"
#include "Display.h"

void AnimationTeletype::Init() {
  Animation::Init(StateType::kIdle);
  Reset();
}

StdReturnType AnimationTeletype::SetTime(byte hour, byte minute) {
  StdReturnType return_value{kENotOk};

  if (Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk &&
      state_ == StateType::kIdle) {
    return_value = kEOk;
    current_word_index_ = 0u;
    current_char_index_ = 0u;
    current_word_length_ = words_.GetDisplayWordLength(clock_words_table_[current_word_index_]);
    state_ = StateType::kSetTime;
  }
  return return_value;
}

void AnimationTeletype::Task() {
  if (state_ == StateType::kSetTime) {
    if (current_char_index_ >= current_word_length_) {
      if (SetNextWordIndex() == kENotOk) {
        FinishWithClockWords(clock_words_table_);
        return;
      }
      current_char_index_ = 0u;
      current_word_length_ = words_.GetDisplayWordLength(clock_words_table_[current_word_index_]);
    }
    current_char_index_++;
    Display::GetInstance().SetWord(clock_words_table_[current_word_index_], current_char_index_);
  }
}

void AnimationTeletype::Reset() {
  clock_words_table_.fill(DisplayWords::WordIdType::kNone);
  current_word_index_ = 0u;
  current_word_length_ = 0u;
  current_char_index_ = 0u;
}

StdReturnType AnimationTeletype::SetNextWordIndex() {
  if (current_word_index_ + 1u < static_cast<byte>(clock_words_table_.size())) {
    current_word_index_++;
    return kEOk;
  }
  return kENotOk;
}
