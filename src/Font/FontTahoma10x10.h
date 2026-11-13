// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Tahoma font, 10 columns by 10 rows.
 */

#ifndef WORDCLOCK_SRC_FONT_FONTTAHOMA10X10_H_
#define WORDCLOCK_SRC_FONT_FONTTAHOMA10X10_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

#define FONT_TAHOMA_10X10_WIDTH 10u            /*!< The width of the font in columns. */
#define FONT_TAHOMA_10X10_HEIGHT 10u           /*!< The height of the font in rows. */
#define FONT_TAHOMA_10X10_FONT_TABLE_SIZE 103u /*!< The number of glyphs in the table. */

/*!
 * \brief The Tahoma font, 10 columns by 10 rows, packed by rows into a `uint16_t` each.
 *
 * Its table holds the glyphs of the ASCII codes 32 to 127 at the indices 0 to 95, then those of
 * the umlauts, Latin-1 0xC4, 0xD6, 0xDC, 0xE4, 0xF6 and 0xFC, at 96 to 101, and that of the degree
 * sign, Latin-1 0xB0, at 102. Text maps a character to its index. docs/fonts.md says how the
 * table is made.
 */
class FontTahoma10x10 : public Font<FontCharHorizontal<uint16_t, FONT_TAHOMA_10X10_HEIGHT>,
                                    FONT_TAHOMA_10X10_FONT_TABLE_SIZE> {
 public:
  /*! \copydoc Font::FontTableElementType */
  using FontCharType = FontCharHorizontal<uint16_t, FONT_TAHOMA_10X10_HEIGHT>;

  /*! \brief The 10 rows of a glyph, from the top. */
  using RowsType = std::array<uint16_t, FONT_TAHOMA_10X10_HEIGHT>;

 private:
  static FontTableType const kFontTable;

 public:
  /*! \brief Makes the font, which reads its glyph table. */
  constexpr FontTahoma10x10() : Font(kFontTable) {}

  /*! \brief Destroys the font. */
  ~FontTahoma10x10() = default;

  /*! \brief Returns the width of the font in columns, FONT_TAHOMA_10X10_WIDTH. */
  static byte GetWidth() { return FONT_TAHOMA_10X10_WIDTH; }

  /*! \brief Returns the height of the font in rows, FONT_TAHOMA_10X10_HEIGHT. */
  static byte GetHeight() { return FONT_TAHOMA_10X10_HEIGHT; }

  /*! \brief Returns how the table is packed, kOrientationHorizontal: a word per row. */
  static Orientation GetOrientation() { return Orientation::kOrientationHorizontal; }

  /*!
   * \brief Returns the glyph table of the font.
   *
   * The table is declared PROGMEM, and Font reads it through memcpy_P.
   */
  static FontTableType const& GetFontTable() { return kFontTable; }
};

#endif  // WORDCLOCK_SRC_FONT_FONTTAHOMA10X10_H_
