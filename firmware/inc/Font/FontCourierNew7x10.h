// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Courier New font, 7 columns by 10 rows.
 */

#ifndef _FONT_COURIER_NEW_7X10_H_
#define _FONT_COURIER_NEW_7X10_H_

#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

/* FontCourierNew7x10 configuration parameter */

/* FontCourierNew7x10 parameter */
#define FONT_COURIER_NEW_7X10_WIDTH                     7u
#define FONT_COURIER_NEW_7X10_HEIGHT                    10u
#define FONT_COURIER_NEW_7X10_FONT_TABLE_SIZE           103u

class FontCourierNew7x10 : public Font<FontCharHorizontal<byte, FONT_COURIER_NEW_7X10_HEIGHT>, FONT_COURIER_NEW_7X10_FONT_TABLE_SIZE>
{
  public:
    using FontCharType = FontCharHorizontal<byte, FONT_COURIER_NEW_7X10_HEIGHT>;
    using RowsType = std::array<byte, FONT_COURIER_NEW_7X10_HEIGHT>;
  
  private:
    static const FontTableType kFontTable;
  
  public:
    constexpr FontCourierNew7x10() : Font(kFontTable) { }
    ~FontCourierNew7x10() { }

    // get methods
    static byte GetWidth() { return FONT_COURIER_NEW_7X10_WIDTH; }
    static byte GetHeight() { return FONT_COURIER_NEW_7X10_HEIGHT; }
    static Orientation GetOrientation() { return Orientation::kOrientationHorizontal; }
    static const FontTableType& GetFontTable() { return kFontTable; }
        
    // set methods

    // methods

};

#endif
