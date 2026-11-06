// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the text output does itself: glyph drawing and the shifting of characters and text.
 */

#define _TEXT_SOURCE_

#include "Text.h"
#include "Display.h"

StdReturnType Text::SetChar(byte column, byte row, char character, FontType font)
{
    byte font_index;
    if(ConvertCharToFontIndex(character, font_index) == kENotOk) return kENotOk;

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(font == FontType::kFont5x8) {
        FontSprite5x8::FontCharType font_char;
        if(font5x8_.GetChar(font_index, font_char) == kENotOk) { return kENotOk; }
        return SetCharFontVertical(column, row, font_char, FONT_SPRITE_5X8_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(font == FontType::kFont7x9) {
        FontCourierNew7x9::FontCharType font_char;
        if(font7x9_.GetChar(font_index, font_char) == kENotOk) { return kENotOk; }
        return SetCharFontHorizontal(column, row, font_char, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(font == FontType::kFont7x10) {
        FontCourierNew7x10::FontCharType font_char;
        if(font7x10_.GetChar(font_index, font_char) == kENotOk) { return kENotOk; }
        return SetCharFontHorizontal(column, row, font_char, FONT_COURIER_NEW_7X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(font == FontType::kFont9x10) {
        FontLucidaSans9x10::FontCharType font_char;
        if(font9x10_.GetChar(font_index, font_char) == kENotOk) { return kENotOk; }
        return SetCharFontVertical(column, row, font_char, FONT_LUCIDA_SANS_9X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(font == FontType::kFont10x10) {
        FontTahoma10x10::FontCharType font_char;
        if(font10x10_.GetChar(font_index, font_char) == kENotOk) { return kENotOk; }
        return SetCharFontHorizontal(column, row, font_char, FONT_TAHOMA_10X10_HEIGHT);
    }
#endif
    return kENotOk;
}

void Text::SetCharWithShift(char character, FontType font)
{
    state_ = StateType::kCharShift;
    shift_.state = ShiftStateType::kBusy;
    shift_.font = font;
    shift_.character = character;
    shift_.char_width = GetFontCharWidth(font, character);
    shift_.counter = shift_.char_width;
}

void Text::SetTextWithShift(const char* text, FontType font)
{
    state_ = StateType::kTextShift;
    shift_.state = ShiftStateType::kIdle;
    shift_.font = font;
    shift_.text = text;
}

void Text::Task(bool show)
{
    if(state_ == StateType::kTextShift) {
        StringShiftTask();
        if(show) { Display::GetInstance().Show(); }
    }
    if(state_ == StateType::kCharShift) {
        CharShiftTask();
        if(show) { Display::GetInstance().Show(); }
    }
}

void Text::Stop()
{
    state_ = StateType::kIdle;
    shift_.state = ShiftStateType::kIdle;
}

byte Text::GetFontHeight(FontType font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(font == FontType::kFont5x8) return font5x8_.GetHeight();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(font == FontType::kFont7x9) return font7x9_.GetHeight();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(font == FontType::kFont7x10) return font7x10_.GetHeight();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(font == FontType::kFont9x10) return font9x10_.GetHeight();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(font == FontType::kFont10x10) return font10x10_.GetHeight();
#endif
    return 0;
}

byte Text::GetFontWidth(FontType font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(font == FontType::kFont5x8) return font5x8_.GetWidth();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(font == FontType::kFont7x9) return font7x9_.GetWidth();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(font == FontType::kFont7x10) return font7x10_.GetWidth();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(font == FontType::kFont9x10) return font9x10_.GetWidth();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(font == FontType::kFont10x10) return font10x10_.GetWidth();
#endif
    return 0;
}

byte Text::GetFontCharWidth(FontType font, char character) const
{
    byte index = ConvertCharToFontIndex(character);

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(font == FontType::kFont5x8) return font5x8_.GetCharWidth(index);
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(font == FontType::kFont7x9) return font7x9_.GetCharWidth(index);
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(font == FontType::kFont7x10) return font7x10_.GetCharWidth(index);
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(font == FontType::kFont9x10) return font9x10_.GetCharWidth(index);
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(font == FontType::kFont10x10) return font10x10_.GetCharWidth(index);
#endif
    return 0;
}

Orientation Text::GetFontOrientation(FontType font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(font == FontType::kFont5x8) return font5x8_.GetOrientation();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(font == FontType::kFont7x9) return font7x9_.GetOrientation();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(font == FontType::kFont7x10) return font7x10_.GetOrientation();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(font == FontType::kFont9x10) return font9x10_.GetOrientation();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(font == FontType::kFont10x10) return font10x10_.GetOrientation();
#endif
    return Orientation::kOrientationNone;
}

void Text::StringShiftTask()
{
    if(shift_.state == ShiftStateType::kIdle) {
        /* If end of string is not reached, load next char */
        if(*shift_.text != STD_NULL_CHARACTER) {
            shift_.character = *shift_.text;
            shift_.state = ShiftStateType::kBusy;
            shift_.char_width = GetFontCharWidth(shift_.font, shift_.character);
            shift_.counter = shift_.char_width;
            CharShiftTask();
            shift_.text++;
        } else { /* otherwise task has finished */
            if(state_ == StateType::kTextShift) state_ = StateType::kIdle;
            wc_transformation_.ShiftLeft();
        }
    } else {
        /* go on shifting char */
        CharShiftTask();
    }
}

void Text::CharShiftTask()
{
    if(shift_.state == ShiftStateType::kBusy) {
        wc_transformation_.ShiftLeft();
        if(shift_.counter == 0u) {
            shift_.state = ShiftStateType::kIdle;
            if(state_ == StateType::kCharShift) state_ = StateType::kIdle;
        } else {
            shift_.counter--;
            byte column = DISPLAY_NUMBER_OF_COLUMNS - shift_.char_width + shift_.counter;
            byte row = GetRowCenter(shift_.font);
            /*
             * A character SetChar refuses is shifted through as a space, which is the blank
             * glyph and therefore clears the cells the previous step wrote. Leaving it out
             * instead would smear those pixels across the display for as long as the text
             * runs. The width was already taken from the same substitute.
             */
            if(SetChar(column, row, shift_.character, shift_.font) == kENotOk) {
                SetChar(column, row, ' ', shift_.font);
            }
        }
    }
}

template <typename RowType, byte RowsSize>
StdReturnType Text::SetCharFontHorizontal(byte column, byte row, const FontCharHorizontal<RowType, RowsSize>& font_char, byte font_height)
{
    for(byte font_row = 0u; font_row < font_height; font_row++)
    {
        RowType char_row;
        byte row_abs = row + font_row;
        if(font_char.GetRow(font_row, char_row) == kENotOk) { return kENotOk; }
        if(SetCharRow(char_row, column, row_abs, font_char.GetWidth()) == kENotOk) { return kENotOk; }
    }
    return kEOk;
}

template <typename RowType>
StdReturnType Text::SetCharRow(RowType char_row, byte column, byte row_abs, byte font_width)
{
    StdReturnType return_value{kEOk};

    for(byte font_column = 0u; font_column < font_width; font_column++)
    {
        byte column_abs = column + font_column;
        if(column_abs < DISPLAY_NUMBER_OF_COLUMNS && row_abs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::GetInstance().WritePixel(column_abs, row_abs, bitRead(char_row, font_column)) == kENotOk) { return_value = kENotOk; }
        } else {
            break;
        }
    }
    return return_value;
}

template <typename ColumnType, byte ColumnsSize>
StdReturnType Text::SetCharFontVertical(byte column, byte row, const FontCharVertical<ColumnType, ColumnsSize>& font_char, byte font_height)
{
    for(byte font_column = 0u; font_column < font_char.GetWidth(); font_column++)
    {
        ColumnType char_column;
        byte column_abs = column + font_column;
        if(font_char.GetColumn(font_column, char_column) == kENotOk) { return kENotOk; }
        if(SetCharColumn(char_column, column_abs, row, font_height) == kENotOk) { return kENotOk; }
    }
    return kEOk;
}

template <typename ColumnType>
StdReturnType Text::SetCharColumn(ColumnType char_column, byte column_abs, byte row, byte font_height)
{
    StdReturnType return_value{kEOk};

    for(byte font_row = 0u; font_row < font_height; font_row++)
    {
        byte row_abs = row + font_row;
        if(column_abs < DISPLAY_NUMBER_OF_COLUMNS && row_abs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::GetInstance().WritePixel(column_abs, row_abs, bitRead(char_column, font_row)) == kENotOk) { return_value = kENotOk; }
        } else {
            break;
        }
    }
    return return_value;
}

// Converts `character` to the index of its glyph in the font tables. kENotOk for a character
// without a glyph, with `index` left as it was.
StdReturnType Text::ConvertCharToFontIndex(char character, byte& index)
{
    StdReturnType return_value = kENotOk;

    /* for umlauts we need a special treatment */
    if('\xC4' == character) { index = 96u; return_value = kEOk; }
    else if('\xD6' == character) { index = 97u; return_value = kEOk; }
    else if('\xDC' == character) { index = 98u; return_value = kEOk; }
    else if('\xE4' == character) { index = 99u; return_value = kEOk; }
    else if('\xF6' == character) { index = 100u; return_value = kEOk; }
    else if('\xFC' == character) { index = 101u; return_value = kEOk; }
    // The degree sign sits with them for the same reason: Latin-1 0xB0 is nowhere near the
    // ASCII run, so it is appended past its end rather than shifting every index after it.
    else if('\xB0' == character) { index = 102u; return_value = kEOk; }
    /*
     * For all others only add the offset - within both bounds, and compared as a byte.
     * Whether char is signed is the platform's decision: where it is not, and AVR is one
     * of those, every Latin-1 byte above the six umlauts passes a lower bound on its own
     * and indexes past the end of a font table that has 102 entries.
     */
    else {
        const byte value = static_cast<byte>(character);

        if((value >= TEXT_ASCII_CHAR_MIN) && (value <= TEXT_ASCII_CHAR_MAX)) {
            index = static_cast<byte>(value + TEXT_ASCII_TABLE_OFFSET); return_value = kEOk;
        }
    }
    return return_value;
}

// Converts `character` to the index of its glyph in the font tables, or to the space's if it
// has no glyph.
byte Text::ConvertCharToFontIndex(char character)
{
    byte index;

    // The mapping is written once, above. It used to be written here as well, which made
    // adding a glyph a change in two branches that had to agree - see fonts.md.
    if(ConvertCharToFontIndex(character, index) == kENotOk) {
        // The space, which is the first glyph: a character that cannot be drawn leaves a gap
        // rather than whatever byte sits past the end of the table.
        return kSpaceFontIndex;
    }
    return index;
}
