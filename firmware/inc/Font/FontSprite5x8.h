// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Sprite font, 5 columns by 8 rows.
 */

#ifndef _FONT_SPRITE_5X8_H_
#define _FONT_SPRITE_5X8_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

#define FONT_SPRITE_5X8_WIDTH                       5u /*!< The width of the font in columns. */
#define FONT_SPRITE_5X8_HEIGHT                      8u /*!< The height of the font in rows. */
#define FONT_SPRITE_5X8_FONT_TABLE_SIZE             103u /*!< The number of glyphs in the table. */

/*!
 * \brief The Sprite font, 5 columns by 8 rows, packed by columns into a `byte` each.
 *
 * Its table holds the glyphs of the ASCII codes 32 to 127 at the indices 0 to 95, then those of
 * the umlauts, Latin-1 0xC4, 0xD6, 0xDC, 0xE4, 0xF6 and 0xFC, at 96 to 101, and that of the degree
 * sign, Latin-1 0xB0, at 102. Text maps a character to its index. docs/fonts.md says how the
 * table is made.
 */
class FontSprite5x8 : public Font<FontCharVertical<byte, FONT_SPRITE_5X8_WIDTH>, FONT_SPRITE_5X8_FONT_TABLE_SIZE>
{
  public:
    /*! \copydoc Font::FontTableElementType */
    using FontCharType = FontCharVertical<byte, FONT_SPRITE_5X8_WIDTH>;

    /*! \brief The 5 columns of a glyph, from the left. */
    using ColumnsType = std::array<byte, FONT_SPRITE_5X8_WIDTH>;

  private:
    static const FontTableType kFontTable;

  public:
    /*! \brief Makes the font, which reads its glyph table. */
    constexpr FontSprite5x8() : Font(kFontTable) { }

    /*! \brief Destroys the font. */
    ~FontSprite5x8() { }

    /*! \brief Returns the width of the font in columns, FONT_SPRITE_5X8_WIDTH. */
    static byte GetWidth() { return FONT_SPRITE_5X8_WIDTH; }

    /*! \brief Returns the height of the font in rows, FONT_SPRITE_5X8_HEIGHT. */
    static byte GetHeight() { return FONT_SPRITE_5X8_HEIGHT; }

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
