// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Lucida Sans font, 9 columns by 10 rows.
 */

#ifndef _FONT_LUCIDA_SANS_9X10_H_
#define _FONT_LUCIDA_SANS_9X10_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

/*! \brief The width of the font in columns. */
#define FONT_LUCIDA_SANS_9X10_WIDTH                      9u
#define FONT_LUCIDA_SANS_9X10_HEIGHT                     10u /*!< The height of the font in rows. */
/*! \brief The number of glyphs in the table. */
#define FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE            103u

/*!
 * \brief The Lucida Sans font, 9 columns by 10 rows, packed by columns into a `uint16_t` each.
 *
 * Its table holds the glyphs of the ASCII codes 32 to 127 at the indices 0 to 95, then those of
 * the umlauts, Latin-1 0xC4, 0xD6, 0xDC, 0xE4, 0xF6 and 0xFC, at 96 to 101, and that of the degree
 * sign, Latin-1 0xB0, at 102. Text maps a character to its index. docs/fonts.md says how the
 * table is made.
 */
class FontLucidaSans9x10 : public Font<FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>, FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE>
{
  public:
    /*! \copydoc Font::FontTableElementType */
    using FontCharType = FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;

    /*! \brief The 9 columns of a glyph, from the left. */
    using ColumnsType = std::array<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;

  private:
    static const FontTableType kFontTable;

  public:
    /*! \brief Makes the font, which reads its glyph table. */
    constexpr FontLucidaSans9x10() : Font(kFontTable) { }

    /*! \brief Destroys the font. */
    ~FontLucidaSans9x10() { }

    /*! \brief Returns the width of the font in columns, FONT_LUCIDA_SANS_9X10_WIDTH. */
    static byte GetWidth() { return FONT_LUCIDA_SANS_9X10_WIDTH; }

    /*! \brief Returns the height of the font in rows, FONT_LUCIDA_SANS_9X10_HEIGHT. */
    static byte GetHeight() { return FONT_LUCIDA_SANS_9X10_HEIGHT; }

    /*! \brief Returns how the table is packed, kOrientationVertical: a word per column. */
    static Orientation GetOrientation() { return Orientation::kOrientationVertical; }

    /*!
     * \brief Returns the glyph table of the font.
     *
     * The table is declared PROGMEM, and Font reads it through memcpy_P.
     */
    static const FontTableType& GetFontTable() { return kFontTable; }
};

#endif
