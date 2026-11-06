// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Courier New font, 7 columns by 9 rows.
 */

#ifndef _FONT_COURIER_NEW_7X9_H_
#define _FONT_COURIER_NEW_7X9_H_

#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

/*! \brief The width of the font in columns. */
#define FONT_COURIER_NEW_7X9_WIDTH                       7u
#define FONT_COURIER_NEW_7X9_HEIGHT                      9u /*!< The height of the font in rows. */
/*! \brief The number of glyphs in the table. */
#define FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE             103u

/*!
 * \brief The Courier New font, 7 columns by 9 rows, packed by rows into a `byte` each.
 *
 * Its table holds the glyphs of the ASCII codes 32 to 127 at the indices 0 to 95, then those of
 * the umlauts, Latin-1 0xC4, 0xD6, 0xDC, 0xE4, 0xF6 and 0xFC, at 96 to 101, and that of the degree
 * sign, Latin-1 0xB0, at 102. Text maps a character to its index. docs/fonts.md says how the
 * table is made.
 */
class FontCourierNew7x9 : public Font<FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>, FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE>
{
  public:
    /*! \copydoc Font::FontTableElementType */
    using FontCharType = FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>;

    /*! \brief The 9 rows of a glyph, from the top. */
    using RowsType = std::array<byte, FONT_COURIER_NEW_7X9_HEIGHT>;

  private:
    static const FontTableType kFontTable;

  public:
    /*! \brief Makes the font, which reads its glyph table. */
    constexpr FontCourierNew7x9() : Font(kFontTable) { }

    /*! \brief Destroys the font. */
    ~FontCourierNew7x9() = default;

    /*! \brief Returns the width of the font in columns, FONT_COURIER_NEW_7X9_WIDTH. */
    static byte GetWidth() { return FONT_COURIER_NEW_7X9_WIDTH; }

    /*! \brief Returns the height of the font in rows, FONT_COURIER_NEW_7X9_HEIGHT. */
    static byte GetHeight() { return FONT_COURIER_NEW_7X9_HEIGHT; }

    /*! \brief Returns how the table is packed, kOrientationHorizontal: a word per row. */
    static Orientation GetOrientation() { return Orientation::kOrientationHorizontal; }

    /*!
     * \brief Returns the glyph table of the font.
     *
     * The table is declared PROGMEM, and Font reads it through memcpy_P.
     */
    static const FontTableType& GetFontTable() { return kFontTable; }
};

#endif
