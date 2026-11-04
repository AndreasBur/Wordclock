// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Where one word of the clock sits on the letter grid.
 */

#ifndef _DISPLAY_WORD_H_
#define _DISPLAY_WORD_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* DisplayWord configuration parameter */

/* DisplayWord parameter */

class DisplayWord
{
  public:
  
  private:
    byte column_;
    byte row_;
    byte length_;
  
  public:
    constexpr DisplayWord(byte column, byte row, byte length) : column_(column), row_(row), length_(length) { }
    constexpr DisplayWord() : column_(0u), row_(0u), length_(0u) { }

    // get methods
    byte GetLength() const { return length_; }
    byte GetRow() const { return row_; }
    byte GetColumn() const { return column_; }
    
    // set methods
    void SetLength(byte length) { length_ = length; }
    void SetRow(byte row) { row_ = row; }
    void SetColumn(byte column) { column_ = column; }

    // methods

};

#endif
