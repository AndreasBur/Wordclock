// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the animation base class does itself.
 */

#include "Animation.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayWord.h"
#include "DisplayWords.h"

bool Animation::IsPixelPartOfClockWords(ClockWords::WordsListType const clock_words_table,
                                        byte column, byte row) {
  DisplayWords words;

  for (uint8_t word_index = 0u; word_index < clock_words_table.size(); word_index++) {
    if (clock_words_table[word_index] == DisplayWords::WordIdType::kNone) {
      break;
    }
    DisplayWord word = words.GetDisplayWord(clock_words_table[word_index]);
    if (word.GetRow() == row) {
      if ((column >= word.GetColumn()) && (column < word.GetColumn() + word.GetLength())) {
        return true;
      }
    }
  }
  return false;
}

bool Animation::IsPixelPartOfClockWords(ClockWords::WordsListType const clock_words_table,
                                        byte index) {
  byte row = index / DISPLAY_NUMBER_OF_COLUMNS;
  byte column = index % DISPLAY_NUMBER_OF_COLUMNS;

  return IsPixelPartOfClockWords(clock_words_table, column, row);
}

byte Animation::SelectByTime(byte hour, byte minute, byte number_of_values) {
  /*
   * Lets an animation vary from minute to minute without keeping a random generator
   * of its own. A plain modulo of the minute would not do, because it walks through
   * the values in a fixed order; two multiplications with an xor fold in between
   * scatter them as a random generator would.
   */
  byte hash = static_cast<byte>(((hour * 60u) + minute) * kSelectHashFactor);
  hash ^= static_cast<byte>(hash >> 4u);
  hash = static_cast<byte>(hash * kSelectHashFactor);

  // scaled by multiplication instead of a modulo, which would favour the lower
  // values because 256 is not a multiple of number_of_values
  return static_cast<byte>((static_cast<uint16_t>(hash) * number_of_values) >> 8u);
}

void Animation::FinishWithClockWords(ClockWords::WordsListType const& clock_words_table) {
  Display::GetInstance().Clear();
  Clock::GetInstance().SetTime(clock_words_table);
  state_ = StateType::kIdle;
}

void Animation::FinishWithTime(byte hour, byte minute) {
  Display::GetInstance().Clear();
  Clock::GetInstance().SetTime(hour, minute);
  state_ = StateType::kIdle;
}

StdReturnType Animation::Show() { return Display::GetInstance().Show(); }
