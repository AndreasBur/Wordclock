// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Where one word of the clock sits on the letter grid.
 */

#ifndef WORDCLOCK_SRC_DISPLAY_DISPLAYWORD_H_
#define WORDCLOCK_SRC_DISPLAY_DISPLAYWORD_H_

#include "Arduino.h"

/*!
 * \brief Where one word stands on the letter grid: the column and the row of its first letter, and
 *        its length.
 *
 * The word runs along its row towards the higher columns. Columns and rows count from 0, as in
 * DisplayCharacters. Nothing here checks that the word fits the grid.
 */
class DisplayWord {
 private:
  byte column_;
  byte row_;
  byte length_;

 public:
  /*!
   * \brief Makes a word of `length` letters whose first letter stands at `column` and `row`.
   *
   * \param[in] column The column of the first letter, counted from 0.
   * \param[in] row The row, counted from 0.
   * \param[in] length The number of letters.
   */
  constexpr DisplayWord(byte column, byte row, byte length)
      : column_(column), row_(row), length_(length) {}

  /*! \brief Makes a word of length 0 at column 0 of row 0, which lights nothing. */
  constexpr DisplayWord() : column_(0u), row_(0u), length_(0u) {}

  /*! \brief Returns the number of letters. */
  byte GetLength() const { return length_; }

  /*! \brief Returns the row, counted from 0. */
  byte GetRow() const { return row_; }

  /*! \brief Returns the column of the first letter, counted from 0. */
  byte GetColumn() const { return column_; }

  /*!
   * \brief Sets the number of letters.
   *
   * \param[in] length The number of letters.
   */
  void SetLength(byte length) { length_ = length; }

  /*!
   * \brief Sets the row.
   *
   * \param[in] row The row, counted from 0.
   */
  void SetRow(byte row) { row_ = row; }

  /*!
   * \brief Sets the column of the first letter.
   *
   * \param[in] column The column, counted from 0.
   */
  void SetColumn(byte column) { column_ = column; }
};

#endif  // WORDCLOCK_SRC_DISPLAY_DISPLAYWORD_H_
