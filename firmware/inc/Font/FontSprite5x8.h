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

/* FontSprite5x8 configuration parameter */

/* FontSprite5x8 parameter */
#define FONT_SPRITE_5X8_WIDTH                       5u
#define FONT_SPRITE_5X8_HEIGHT                      8u
#define FONT_SPRITE_5X8_FONT_TABLE_SIZE             103u

class FontSprite5x8 : public Font<FontCharVertical<byte, FONT_SPRITE_5X8_WIDTH>, FONT_SPRITE_5X8_FONT_TABLE_SIZE>
{
  public:
    using FontCharType = FontCharVertical<byte, FONT_SPRITE_5X8_WIDTH>;
    using ColumnsType = std::array<byte, FONT_SPRITE_5X8_WIDTH>;

  private:
    static const FontTableType kFontTable;
  
  public:
    constexpr FontSprite5x8() : Font(kFontTable) { }
    ~FontSprite5x8() { }

    // get methods
    static byte GetWidth() { return FONT_SPRITE_5X8_WIDTH; }
    static byte GetHeight() { return FONT_SPRITE_5X8_HEIGHT; }
    static Orientation GetOrientation() { return Orientation::kOrientationVertical; }
    static const FontTableType& GetFontTable() { return kFontTable; }
        
    // set methods

    // methods

};

#endif
