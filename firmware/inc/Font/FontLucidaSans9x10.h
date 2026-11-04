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

/* FontLucidaSans9x10 configuration parameter */

/* FontLucidaSans9x10 parameter */
#define FONT_LUCIDA_SANS_9X10_WIDTH                      9u
#define FONT_LUCIDA_SANS_9X10_HEIGHT                     10u
#define FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE            103u

class FontLucidaSans9x10 : public Font<FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>, FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE>
{
  public:
    using FontCharType = FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;
    using ColumnsType = std::array<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;
  
  private:
    static const FontTableType kFontTable;
  
  public:
    constexpr FontLucidaSans9x10() : Font(kFontTable) { }
    ~FontLucidaSans9x10() { }

    // get methods
    static byte GetWidth() { return FONT_LUCIDA_SANS_9X10_WIDTH; }
    static byte GetHeight() { return FONT_LUCIDA_SANS_9X10_HEIGHT; }
    static Orientation GetOrientation() { return Orientation::kOrientationVertical; }
    static const FontTableType& GetFontTable() { return kFontTable; }
                
    // set methods

    // methods

};

#endif
