// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief One glyph of a font, as rows or as columns of bits.
 */

#ifndef WORDCLOCK_SRC_FONT_FONTCHAR_H_
#define WORDCLOCK_SRC_FONT_FONTCHAR_H_

#include <array>

#include "Arduino.h"
#include "StandardTypes.h"

/*!
 * \brief What every glyph has: its width in columns.
 *
 * FontCharHorizontal and FontCharVertical derive from it and add the pixels.
 */
class FontChar {
 public:
 private:
  byte width_;

 protected:
  /*! \brief Makes a glyph zero columns wide. */
  constexpr FontChar() : width_(0u) {}

  /*!
   * \brief Makes a glyph `width` columns wide.
   *
   * \param[in] width The width in columns.
   */
  constexpr FontChar(byte width) : width_(width) {}

 public:
  /*! \brief Returns the width of the glyph in columns. */
  byte GetWidth() const { return width_; }

  /*!
   * \brief Sets the width of the glyph in columns.
   *
   * \param[in] width The width in columns.
   */
  void SetWidth(byte width) { width_ = width; }
};

/*!
 * \brief A glyph packed by rows: its width and one word per pixel row.
 *
 * The rows count from 0 at the top. Bit 0 of a row is its leftmost pixel; the bits from the
 * glyph's width on are not drawn.
 *
 * \tparam RowType The word of a row, `byte` or `uint16_t`, with a bit per pixel of the row.
 * \tparam RowsSize The number of rows, the height of the font.
 */
template <typename RowType, byte RowsSize>
class FontCharHorizontal : public FontChar {
 public:
 private:
  using RowsType = std::array<RowType, RowsSize>;
  RowsType rows_;

 public:
  /*!
   * \brief Makes a glyph of `width` columns from its rows.
   *
   * \param[in] width The width in columns.
   * \param[in] rows The rows, from the top.
   */
  constexpr FontCharHorizontal(byte width, std::array<RowType, RowsSize> rows)
      : FontChar(width), rows_(rows) {}

  /*! \brief Makes an empty glyph: zero columns wide, every row cleared. */
  constexpr FontCharHorizontal() : FontChar(0u), rows_{} {}

  /*
   * Two forms of the reader, and the argument list says which: the one taking a reference
   * answers whether the row is part of the glyph, the one taking none answers the row -
   * and a row outside the glyph is empty, which draws nothing.
   */

  /*!
   * \brief Returns the row at `index`.
   *
   * \param[in] index The row, counted from 0 at the top.
   * \return The row, or an empty one if `index` lies past the last row.
   */
  RowType GetRow(byte index) const {
    if (!IsIndexValid(index)) {
      return RowType{};
    }
    return rows_[index];
  }

  /*!
   * \brief Copies the row at `index` into `row`.
   *
   * \param[in] index The row, counted from 0 at the top.
   * \param[out] row The row.
   * \return kEOk, or kENotOk if `index` lies past the last row; `row` is then left as it was.
   */
  StdReturnType GetRow(byte index, RowType& row) const {
    if (IsIndexValid(index)) {
      row = rows_[index];
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*! \brief Returns all the rows, from the top. */
  RowsType GetRows() const { return rows_; }

  /*!
   * \brief Sets the row at `index` to `row`.
   *
   * \param[in] index The row, counted from 0 at the top.
   * \param[in] row The new row.
   * \return kEOk, or kENotOk if `index` lies past the last row; the glyph is then left as it was.
   */
  StdReturnType SetRow(byte index, RowType row) {
    if (IsIndexValid(index)) {
      rows_[index] = row;
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*!
   * \brief Sets all the rows.
   *
   * \param[in] rows The new rows, from the top.
   */
  void SetRows(RowsType rows) { rows_ = rows; }

  /*!
   * \brief Returns whether `index` is a row of the glyph.
   *
   * \param[in] index The row.
   * \return True if `index` lies below RowsSize.
   */
  static constexpr bool IsIndexValid(byte index) { return index < RowsSize; }
};

/*!
 * \brief A glyph packed by columns: its width and one word per pixel column.
 *
 * The columns count from 0 at the left; those from the glyph's width on are not drawn. Bit 0 of a
 * column is its top pixel.
 *
 * \tparam ColumnType The word of a column, `byte` or `uint16_t`, with a bit per pixel of the
 *                    column.
 * \tparam ColumnsSize The number of columns, the width of the font.
 */
template <typename ColumnType, byte ColumnsSize>
class FontCharVertical : public FontChar {
 public:
 private:
  using ColumnsType = std::array<ColumnType, ColumnsSize>;
  ColumnsType columns_;

 public:
  /*!
   * \brief Makes a glyph of `width` columns from its columns.
   *
   * \param[in] width The width in columns.
   * \param[in] columns The columns, from the left.
   */
  constexpr FontCharVertical(byte width, std::array<ColumnType, ColumnsSize> columns)
      : FontChar(width), columns_(columns) {}

  /*! \brief Makes an empty glyph: zero columns wide, every column cleared. */
  constexpr FontCharVertical() : FontChar(0), columns_{} {}

  // The same two forms as the horizontal glyph above, for the same reason: a column
  // outside the glyph is empty rather than whatever follows it in memory.

  /*!
   * \brief Returns the column at `index`.
   *
   * \param[in] index The column, counted from 0 at the left.
   * \return The column, or an empty one if `index` lies past the last column.
   */
  ColumnType GetColumn(byte index) const {
    if (!IsIndexValid(index)) {
      return ColumnType{};
    }
    return columns_[index];
  }

  /*!
   * \brief Copies the column at `index` into `column`.
   *
   * \param[in] index The column, counted from 0 at the left.
   * \param[out] column The column.
   * \return kEOk, or kENotOk if `index` lies past the last column; `column` is then left as it was.
   */
  StdReturnType GetColumn(byte index, ColumnType& column) const {
    if (IsIndexValid(index)) {
      column = columns_[index];
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*! \brief Returns all the columns, from the left. */
  ColumnsType GetColumns() const { return columns_; }

  /*!
   * \brief Sets the column at `index` to `column`.
   *
   * \param[in] index The column, counted from 0 at the left.
   * \param[in] column The new column.
   * \return kEOk, or kENotOk if `index` lies past the last column; the glyph is then left as it
   *         was.
   */
  StdReturnType SetColumn(byte index, ColumnType column) {
    if (IsIndexValid(index)) {
      columns_[index] = column;
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*!
   * \brief Sets all the columns.
   *
   * \param[in] columns The new columns, from the left.
   */
  void SetColumns(ColumnsType columns) { columns_ = columns; }

  /*!
   * \brief Returns whether `index` is a column of the glyph.
   *
   * \param[in] index The column.
   * \return True if `index` lies below ColumnsSize.
   */
  static constexpr bool IsIndexValid(byte index) { return index < ColumnsSize; }
};

#endif  // WORDCLOCK_SRC_FONT_FONTCHAR_H_
