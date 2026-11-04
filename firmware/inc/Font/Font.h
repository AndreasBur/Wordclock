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

/*! \brief How a font's table is packed: one word per pixel row, or one per pixel column. */
enum class Orientation {
    kOrientationHorizontal, /*!< One word per pixel row, as FontCharHorizontal holds it. */
    kOrientationVertical,   /*!< One word per pixel column, as FontCharVertical holds it. */
    kOrientationNone        /*!< No packing, for a font that is not built in. */
};

/*!
 * \brief A font: a table of glyphs, read by index.
 *
 * The table belongs to the font class that derives from this one, such as FontSprite5x8; the font
 * keeps a reference to it. Each read copies the glyph out of the table with memcpy_P, as the
 * tables are declared PROGMEM.
 *
 * \tparam FontCharType The type of a glyph, a FontCharHorizontal or a FontCharVertical.
 * \tparam FontTableSize The number of glyphs in the table.
 */
template <typename FontCharType, size_t FontTableSize> class Font
{
  public:
    /*! \brief A glyph of the font, as the table holds it. */
    using FontTableElementType = FontCharType;

    /*! \brief The glyph table: one glyph per index, from 0. */
    using FontTableType = std::array<FontCharType, FontTableSize>;
  
  private:
    const FontTableType& font_table_;
  
    FontTableElementType GetFontTableElement(byte index) const {
        FontTableElementType font_table_element;
        memcpy_P(&font_table_element, &font_table_[index], sizeof(FontTableElementType));
        return font_table_element;
    }
    
  protected:    

  public:
    /*!
     * \brief Makes a font that reads `font_table`.
     *
     * \param[in] font_table The glyph table. The font keeps a reference to it, not a copy.
     */
    constexpr Font(const FontTableType& font_table) : font_table_(font_table) { }

    /*! \brief Destroys the font. */
    ~Font() { }

    /*
     * Two forms of every reader, and the argument list says which: the one taking a
     * reference answers whether the glyph is in the table, the one taking none answers the
     * glyph. Both check, and the unchecked read is the private table element above - a
     * glyph outside the table is the default one, which is zero columns wide and therefore
     * draws nothing.
     */

    /*!
     * \brief Returns the glyph at `index`.
     *
     * \param[in] index The index of the glyph in the table.
     * \return The glyph, or an empty one, zero columns wide, if `index` lies past the end of the
     *         table.
     */
    FontCharType GetChar(byte index) const {
        if(!IsIndexValid(index)) { return FontCharType{}; }
        return GetFontTableElement(index);
    }

    /*!
     * \brief Returns the width of the glyph at `index`, in columns.
     *
     * \param[in] index The index of the glyph in the table.
     * \return The width, or 0 if `index` lies past the end of the table.
     */
    byte GetCharWidth(byte index) const { return GetChar(index).GetWidth(); }

    /*!
     * \brief Copies the glyph at `index` into `font_char`.
     *
     * \param[in] index The index of the glyph in the table.
     * \param[out] font_char The glyph.
     * \return kEOk, or kENotOk if `index` lies past the end of the table; `font_char` is then left
     *         as it was.
     */
    StdReturnType GetChar(byte index, FontCharType& font_char) const {
        if(IsIndexValid(index)) {
            font_char = GetFontTableElement(index);
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Copies the width of the glyph at `index`, in columns, into `width`.
     *
     * \param[in] index The index of the glyph in the table.
     * \param[out] width The width.
     * \return kEOk, or kENotOk if `index` lies past the end of the table; `width` is then left as
     *         it was.
     */
    StdReturnType GetCharWidth(byte index, byte& width) const {
        if(IsIndexValid(index)) {
            width = GetFontTableElement(index).GetWidth();
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    /*!
     * \brief Returns whether `index` is the index of a glyph of the table.
     *
     * \param[in] index The index.
     * \return True if `index` lies below FontTableSize.
     */
    static constexpr bool IsIndexValid(byte index) { return index < FontTableSize; }

};

#endif
