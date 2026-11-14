// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock words do themselves: comparison and the flat word list.
 */

#include "ClockWords.h"

bool ClockWords::operator==(ClockWords const& clock_words) const {
  return show_it_is_ == clock_words.GetShowItIs() && hour_words_ == clock_words.GetHourWords() &&
         minute_words_ == clock_words.GetMinuteWords();
}

bool ClockWords::operator!=(ClockWords const& clock_words) const {
  return !operator==(clock_words);
}

ClockWords::WordsListType ClockWords::GetWordsList() const {
  WordsListType clock_words;
  byte clock_words_index{0};

  if (show_it_is_) {
    clock_words[clock_words_index++] = DisplayWords::WordIdType::kEs;
    clock_words[clock_words_index++] = DisplayWords::WordIdType::kIst;
  }
  for (uint8_t index = 0; index < CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS; index++) {
    if (minute_words_[index] != DisplayWords::WordIdType::kNone) {
      clock_words[clock_words_index++] = minute_words_[index];
    }
  }
  for (uint8_t index = 0; index < CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS; index++) {
    if (hour_words_[index] != DisplayWords::WordIdType::kNone) {
      clock_words[clock_words_index++] = hour_words_[index];
    }
  }
  for (uint8_t index = clock_words_index; index < CLOCK_WORDS_MAX_NUMBER_OF_WORDS; index++) {
    clock_words[index] = DisplayWords::WordIdType::kNone;
  }
  return clock_words;
}
