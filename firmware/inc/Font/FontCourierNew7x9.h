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

/* FontCourierNew7x9 configuration parameter */

/* FontCourierNew7x9 parameter */
#define FONT_COURIER_NEW_7X9_WIDTH                       7u
#define FONT_COURIER_NEW_7X9_HEIGHT                      9u
#define FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE             103u

class FontCourierNew7x9 : public Font<FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>, FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE>
{
  public:
    using FontCharType = FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>;
    using RowsType = std::array<byte, FONT_COURIER_NEW_7X9_HEIGHT>;
  
  private:
    static const FontTableType kFontTable;
  
  public:
    constexpr FontCourierNew7x9() : Font(kFontTable) { }
    ~FontCourierNew7x9() { }

    // get methods
    static byte GetWidth() { return FONT_COURIER_NEW_7X9_WIDTH; }
    static byte GetHeight() { return FONT_COURIER_NEW_7X9_HEIGHT; }
    static Orientation GetOrientation() { return Orientation::kOrientationHorizontal; }
    static const FontTableType& GetFontTable() { return kFontTable; }
        
    // set methods

    // methods

};

#endif
