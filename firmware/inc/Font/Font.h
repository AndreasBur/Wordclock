// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A font: a table of glyphs, read by index.
 */

#ifndef _FONT_H_
#define _FONT_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include <array>

/* Font configuration parameter */

/* FontCommon parameter */

enum class Orientation {
    kOrientationHorizontal,
    kOrientationVertical,
    kOrientationNone
};

template <typename FontCharType, size_t FontTableSize> class Font
{
  public:
    using FontTableElementType = FontCharType;
    using FontTableType = std::array<FontCharType, FontTableSize>;
  
  private:
    const FontTableType& font_table_;
  
    // functions
    FontTableElementType GetFontTableElement(byte index) const {
        FontTableElementType font_table_element;
        memcpy_P(&font_table_element, &font_table_[index], sizeof(FontTableElementType));
        return font_table_element;
    }
    
  protected:    

  public:
    constexpr Font(const FontTableType& font_table) : font_table_(font_table) { }
    ~Font() { }

    /*
     * Two forms of every reader, and the argument list says which: the one taking a
     * reference answers whether the glyph is in the table, the one taking none answers the
     * glyph. Both check, and the unchecked read is the private table element above - a
     * glyph outside the table is the default one, which is zero columns wide and therefore
     * draws nothing.
     */
    // get methods
    FontCharType GetChar(byte index) const {
        if(!IsIndexValid(index)) { return FontCharType{}; }
        return GetFontTableElement(index);
    }
    byte GetCharWidth(byte index) const { return GetChar(index).GetWidth(); }

    StdReturnType GetChar(byte index, FontCharType& font_char) const {
        if(IsIndexValid(index)) {
            font_char = GetFontTableElement(index);
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    StdReturnType GetCharWidth(byte index, byte& width) const {
        if(IsIndexValid(index)) {
            width = GetFontTableElement(index).GetWidth();
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    // set methods

    // methods
    static constexpr bool IsIndexValid(byte index) { return index < FontTableSize; }

};

#endif
