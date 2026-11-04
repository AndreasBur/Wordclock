// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief One glyph of a font, as rows or as columns of bits.
 */

#ifndef _FONT_CHAR_H_
#define _FONT_CHAR_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include <array>

/* FontChar configuration parameter */

/* FontChar parameter */

class FontChar
{
  public:
  
  private:
    byte width_;

  protected:
    constexpr FontChar() : width_(0u) { }
    constexpr FontChar(byte width) : width_(width) { }

  public:
    // get methods
    byte GetWidth() const { return width_; }

    // set methods
    void SetWidth(byte width) { width_ = width; }

    // methods
};

template <typename RowType, byte RowsSize> class FontCharHorizontal : public FontChar
{
  public:

  private:
    using RowsType = std::array<RowType, RowsSize>;
    RowsType rows_;

  public:
    constexpr FontCharHorizontal(byte width, std::array<RowType, RowsSize> rows) : FontChar(width), rows_(rows) { }
    constexpr FontCharHorizontal() : FontChar(0u), rows_{} { }

    /*
     * Two forms of the reader, and the argument list says which: the one taking a reference
     * answers whether the row is part of the glyph, the one taking none answers the row -
     * and a row outside the glyph is empty, which draws nothing.
     */
    // get methods
    RowType GetRow(byte index) const {
        if(!IsIndexValid(index)) { return RowType{}; }
        return rows_[index];
    }
    StdReturnType GetRow(byte index, RowType& row) const {
        if(IsIndexValid(index)) {
            row = rows_[index];
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    RowsType GetRows() const { return rows_; }

    // set methods
    StdReturnType SetRow(byte index, RowType row) {
        if(IsIndexValid(index)) {
            rows_[index] = row;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    void SetRows(RowsType rows) { rows_ = rows; }

    // methods
    static constexpr bool IsIndexValid(byte index) { return index < RowsSize; }

};

template <typename ColumnType, byte ColumnsSize> class FontCharVertical : public FontChar
{
  public:

  private:
    using ColumnsType = std::array<ColumnType, ColumnsSize>;
    ColumnsType columns_;

  public:
    constexpr FontCharVertical(byte width, std::array<ColumnType, ColumnsSize> columns) : FontChar(width), columns_(columns) { }

    constexpr FontCharVertical() : FontChar(0), columns_{} { }

    // The same two forms as the horizontal glyph above, for the same reason: a column
    // outside the glyph is empty rather than whatever follows it in memory.
    // get methods
    ColumnType GetColumn(byte index) const {
        if(!IsIndexValid(index)) { return ColumnType{}; }
        return columns_[index];
    }
    StdReturnType GetColumn(byte index, ColumnType& column) const {
        if(IsIndexValid(index)) {
            column = columns_[index];
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    ColumnsType GetColumns() const { return columns_; }

    // set methods
    StdReturnType SetColumn(byte index, ColumnType column) {
        if(IsIndexValid(index)) {
            columns_[index] = column;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    void SetColumns(ColumnsType columns) { columns_ = columns; }

    // methods
    static constexpr bool IsIndexValid(byte index) { return index < ColumnsSize; }

};

#endif
