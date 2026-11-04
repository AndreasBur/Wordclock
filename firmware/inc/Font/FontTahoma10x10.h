// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Tahoma font, 10 columns by 10 rows.
 */

#ifndef _FONT_TAHOMA_10X10_H_
#define _FONT_TAHOMA_10X10_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

/* FontTahoma10x10 configuration parameter */

/* FontTahoma10x10 parameter */
#define FONT_TAHOMA_10X10_WIDTH                    10u
#define FONT_TAHOMA_10X10_HEIGHT                   10u
#define FONT_TAHOMA_10X10_FONT_TABLE_SIZE          103u

class FontTahoma10x10 : public Font<FontCharHorizontal<uint16_t, FONT_TAHOMA_10X10_HEIGHT>, FONT_TAHOMA_10X10_FONT_TABLE_SIZE>
{
  public:
    using FontCharType = FontCharHorizontal<uint16_t, FONT_TAHOMA_10X10_HEIGHT>;
    using RowsType = std::array<uint16_t, FONT_TAHOMA_10X10_HEIGHT>;
  
  private:
    static const FontTableType kFontTable;

  public:
    constexpr FontTahoma10x10() : Font(kFontTable) { }
    ~FontTahoma10x10() { }

    // get methods
    static byte GetWidth() { return FONT_TAHOMA_10X10_WIDTH; }
    static byte GetHeight() { return FONT_TAHOMA_10X10_HEIGHT; }
    static Orientation GetOrientation() { return Orientation::kOrientationHorizontal; }
    static const FontTableType& GetFontTable() { return kFontTable; }

    // set methods

    // methods

};

#endif
