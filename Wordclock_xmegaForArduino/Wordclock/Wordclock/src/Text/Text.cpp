/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       Text.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _TEXT_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Text.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  setChar()
******************************************************************************************************************************************************/
StdReturnType Text::setChar(byte Column, byte Row, char Char, FontType Font)
{
    byte fontIndex;
    if(convertCharToFontIndex(Char, fontIndex) == E_NOT_OK) return E_NOT_OK;

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) {
        FontSprite5x8::FontCharType FontChar;
        if(Font5x8.getChar(fontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontVertical(Column, Row, FontChar, FONT_SPRITE_5X8_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar;
        if(Font7x9.getChar(fontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) {
        FontCourierNew7x10::FontCharType FontChar;
        if(Font7x10.getChar(fontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_COURIER_NEW_7X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) {
        FontLucidaSans9x10::FontCharType FontChar;
        if(Font9x10.getChar(fontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontVertical(Column, Row, FontChar, FONT_LUCIDA_SANS_9X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) {
        FontTahoma10x10::FontCharType FontChar;
        if(Font10x10.getChar(fontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_TAHOMA_10X10_HEIGHT);
    }
#endif
    return E_NOT_OK;
} /* setChar */


/******************************************************************************************************************************************************
  setCharFast()
******************************************************************************************************************************************************/
void Text::setCharFast(byte Column, byte Row, char Char, FontType Font)
{
    byte fontIndex = convertCharToFontIndexFast(Char);

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) {
        FontSprite5x8::FontCharType FontChar = Font5x8.getCharFast(fontIndex);
        setCharFontVerticalFast(Column, Row, FontChar, FONT_SPRITE_5X8_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar = Font7x9.getCharFast(fontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) {
        FontCourierNew7x10::FontCharType FontChar = Font7x10.getCharFast(fontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_COURIER_NEW_7X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) {
        FontLucidaSans9x10::FontCharType FontChar = Font9x10.getCharFast(fontIndex);
        setCharFontVertical(Column, Row, FontChar, FONT_LUCIDA_SANS_9X10_HEIGHT);
    }
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) {
        FontTahoma10x10::FontCharType FontChar = Font10x10.getCharFast(fontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_TAHOMA_10X10_HEIGHT);
    }
#endif
} /* setCharFast */


/******************************************************************************************************************************************************
  setCharWithShift()
******************************************************************************************************************************************************/
void Text::setCharWithShift(char Char, FontType Font)
{
    State = STATE_CHAR_SHIFT;
    Shift.State = SHIFT_STATE_BUSY;
    Shift.Font = Font;
    Shift.Char = Char;
    Shift.CharWidth = getFontCharWidth(Font, Char);
    Shift.Counter = Shift.CharWidth;
}


/******************************************************************************************************************************************************
  setTextWithShift()
******************************************************************************************************************************************************/
void Text::setTextWithShift(const char* Text, FontType Font)
{
    State = STATE_TEXT_SHIFT;
    Shift.State = SHIFT_STATE_IDLE;
    Shift.Font = Font;
    Shift.Text = Text;
}


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void Text::task(bool Show)
{
    if(State == STATE_TEXT_SHIFT) {
        stringShiftTask();
        if(Show) { Display::getInstance().show(); }
    }
    if(State == STATE_CHAR_SHIFT) {
        charShiftTask();
        if(Show) { Display::getInstance().show(); }
    }
} /* task */


/******************************************************************************************************************************************************
  stop()
******************************************************************************************************************************************************/
void Text::stop()
{
    State = STATE_IDLE;
    Shift.State = SHIFT_STATE_IDLE;
} /* stop */


/******************************************************************************************************************************************************
  getFontHeight()
******************************************************************************************************************************************************/
byte Text::getFontHeight(FontType Font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getHeight();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getHeight();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getHeight();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getHeight();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getHeight();
#endif
    return 0;
} /* getFontHeight */


/******************************************************************************************************************************************************
  getFontWidth()
******************************************************************************************************************************************************/
byte Text::getFontWidth(FontType Font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getWidth();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getWidth();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getWidth();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getWidth();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getWidth();
#endif
    return 0;
} /* getFontWidth */


/******************************************************************************************************************************************************
  getFontCharWidth()
******************************************************************************************************************************************************/
byte Text::getFontCharWidth(FontType Font, char Char) const
{
    byte index = convertCharToFontIndexFast(Char);

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getCharWidthFast(index);
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getCharWidthFast(index);
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getCharWidthFast(index);
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getCharWidthFast(index);
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getCharWidthFast(index);
#endif
    return 0;
} /* getFontCharWidth */


/******************************************************************************************************************************************************
  getFontOrientation()
******************************************************************************************************************************************************/
Orientation Text::getFontOrientation(FontType Font) const
{
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getOrientation();
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getOrientation();
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getOrientation();
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getOrientation();
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getOrientation();
#endif
    return Orientation::ORIENTATION_NONE;
} /* getFontOrientation */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  stringShiftTask()
******************************************************************************************************************************************************/
void Text::stringShiftTask()
{
    if(Shift.State == SHIFT_STATE_IDLE) {
        /* If end of string is not reached, load next char */
        if(*Shift.Text != STD_NULL_CHARACTER) {
            Shift.Char = *Shift.Text;
            Shift.State = SHIFT_STATE_BUSY;
            Shift.CharWidth = getFontCharWidth(Shift.Font, Shift.Char);
            Shift.Counter = Shift.CharWidth;
            charShiftTask();
            Shift.Text++;
        } else { /* otherwise task has finished */
            if(State == STATE_TEXT_SHIFT) State = STATE_IDLE;
            wcTransformation.shiftLeft();
            //wcTransformation.shiftLeftFast();
        }
    } else {
        /* go on shifting char */
        charShiftTask();
    }
} /* stringShiftTask */


/******************************************************************************************************************************************************
  charShiftTask()
******************************************************************************************************************************************************/
void Text::charShiftTask()
{
    if(Shift.State == SHIFT_STATE_BUSY) {
        wcTransformation.shiftLeft();
        if(Shift.Counter == 0u) {
            Shift.State = SHIFT_STATE_IDLE;
            if(State == STATE_CHAR_SHIFT) State = STATE_IDLE;
        } else {
            Shift.Counter--;
            byte Column = DISPLAY_NUMBER_OF_COLUMNS - Shift.CharWidth + Shift.Counter;
            byte Row = getRowCenter(Shift.Font);
            setChar(Column, Row, Shift.Char, Shift.Font);
            //setCharFast(Column, Row, Shift.Char, Shift.Font);
        }
    }
} /* charShiftTask */


/******************************************************************************************************************************************************
  setCharFontHorizontal()
******************************************************************************************************************************************************/
template <typename RowType, byte RowsSize>
StdReturnType Text::setCharFontHorizontal(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte FontHeight)
{
    for(byte fontRow = 0u; fontRow < FontHeight; fontRow++)
    {
        RowType CharRow;
        byte RowAbs = Row + fontRow;
        if(FontChar.getRow(fontRow, CharRow) == E_NOT_OK) { return E_NOT_OK; }
        if(setCharRow(CharRow, Column, RowAbs, FontChar.getWidth()) == E_NOT_OK) { return E_NOT_OK; }
    }
    return E_OK;
} /* setCharFontHorizontal */


/******************************************************************************************************************************************************
  setCharRow()
******************************************************************************************************************************************************/
template <typename RowType>
StdReturnType Text::setCharRow(RowType CharRow, byte Column, byte RowAbs, byte FontWidth)
{
    StdReturnType ReturnValue{E_OK};

    for(byte fontColumn = 0u; fontColumn < FontWidth; fontColumn++)
    {
        byte ColumnAbs = Column + fontColumn;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::getInstance().writePixel(ColumnAbs, RowAbs, bitRead(CharRow, fontColumn)) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
        } else {
            break;
        }
    }
    return ReturnValue;
} /* setCharRow */


/******************************************************************************************************************************************************
  setCharFontHorizontalFast()
******************************************************************************************************************************************************/
template <typename RowType, byte RowsSize>
void Text::setCharFontHorizontalFast(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte FontHeight)
{
    for(byte fontRow = 0u; fontRow < FontHeight; fontRow++)
    {
        RowType CharRow = FontChar.getRowFast(fontRow);
        byte RowAbs = Row + fontRow;
        setCharRowFast(CharRow, Column, RowAbs, FontChar.getWidth());
    }
} /* setCharFontHorizontalFast */


/******************************************************************************************************************************************************
  setCharRowFast()
******************************************************************************************************************************************************/
template <typename RowType>
void Text::setCharRowFast(RowType CharRow, byte Column, byte RowAbs, byte FontWidth)
{
    for(byte fontColumn = 0u; fontColumn < FontWidth; fontColumn++)
    {
        byte ColumnAbs = Column + fontColumn;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            Display::getInstance().writePixelFast(ColumnAbs, RowAbs, bitRead(CharRow, fontColumn));
        } else {
            break;
        }
    }
} /* setCharRowFast */


/******************************************************************************************************************************************************
  setCharFontVertical()
******************************************************************************************************************************************************/
template <typename ColumnType, byte ColumnsSize>
StdReturnType Text::setCharFontVertical(byte Column, byte Row, const FontCharVertical<ColumnType, ColumnsSize>& FontChar, byte FontHeight)
{
    for(byte fontColumn = 0u; fontColumn < FontChar.getWidth(); fontColumn++)
    {
        ColumnType CharColumn;
        byte ColumnAbs = Column + fontColumn;
        if(FontChar.getColumn(fontColumn, CharColumn) == E_NOT_OK) { return E_NOT_OK; }
        if(setCharColumn(CharColumn, ColumnAbs, Row, FontHeight) == E_NOT_OK) { return E_NOT_OK; }
    }
    return E_OK;
} /* setCharFontVertical */


/******************************************************************************************************************************************************
  setCharColumn()
******************************************************************************************************************************************************/
template <typename ColumnType>
StdReturnType Text::setCharColumn(ColumnType CharColumn, byte ColumnAbs, byte Row, byte FontHeight)
{
    StdReturnType returnValue{E_OK};

    for(byte fontRow = 0u; fontRow < FontHeight; fontRow++)
    {
        byte RowAbs = Row + fontRow;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::getInstance().writePixel(ColumnAbs, RowAbs, bitRead(CharColumn, fontRow)) == E_NOT_OK) { returnValue = E_NOT_OK; }
        } else {
            break;
        }
    }
    return returnValue;
} /* setCharColumn */


/******************************************************************************************************************************************************
  setCharFontVerticalFast()
******************************************************************************************************************************************************/
template <typename ColumnType, byte ColumnsSize>
void Text::setCharFontVerticalFast(byte Column, byte Row, const FontCharVertical<ColumnType, ColumnsSize>& FontChar, byte FontHeight)
{
    for(byte fontColumn = 0u; fontColumn < FontChar.getWidth(); fontColumn++)
    {
        ColumnType CharColumn = FontChar.getColumnFast(fontColumn);
        byte ColumnAbs = Column + fontColumn;
        setCharColumnFast(CharColumn, ColumnAbs, Row, FontHeight);
    }
} /* setCharFontVerticalFast */


/******************************************************************************************************************************************************
  setCharColumnFast()
******************************************************************************************************************************************************/
template <typename ColumnType>
void Text::setCharColumnFast(ColumnType CharColumn, byte ColumnAbs, byte Row, byte FontHeight)
{
    for(byte fontRow = 0u; fontRow < FontHeight; fontRow++)
    {
        byte rowAbs = Row + fontRow;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && rowAbs < DISPLAY_NUMBER_OF_ROWS) {
            Display::getInstance().writePixelFast(ColumnAbs, rowAbs, bitRead(CharColumn, fontRow));
        } else {
            break;
        }
    }
} /* setCharColumnFast */


/******************************************************************************************************************************************************
  convertCharToFontIndex()
******************************************************************************************************************************************************/
/*! \brief          converts char to sprite
 *  \details        this function converts the ASCII char to sprite index
 *
 *  \param[in]      Char            char to convert to sprite
 *  \param[out]     SpriteIndex     appropriate sprite
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
StdReturnType Text::convertCharToFontIndex(char Char, byte& Index) const
{
    StdReturnType returnValue = E_NOT_OK;

    /* for umlauts we need a special treatment */
    if('Ä' == Char) { Index = 96u; returnValue = E_OK; }
    else if('Ö' == Char) { Index = 97u; returnValue = E_OK; }
    else if('Ü' == Char) { Index = 98u; returnValue = E_OK; }
    else if('ä' == Char) { Index = 99u; returnValue = E_OK; }
    else if('ö' == Char) { Index = 100u; returnValue = E_OK; }
    else if('ü' == Char) { Index = 101u; returnValue = E_OK; }
    /* for all others only add offset */
    else if(Char >= TEXT_ASCII_CHAR_MIN) {
        Index = Char + TEXT_ASCII_TABLE_OFFSET; returnValue = E_OK;
    }
    return returnValue;
} /* convertCharToFontIndex */


/******************************************************************************************************************************************************
  convertCharToFontIndex()
******************************************************************************************************************************************************/
/*! \brief          converts char to sprite
 *  \details        this function converts the ASCII char to sprite index
 *
 *  \param[in]      Char            char to convert to sprite
 *  \param[out]     SpriteIndex     appropriate sprite
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
byte Text::convertCharToFontIndexFast(char Char) const
{
    /* for umlauts we need a special treatment */
    if('Ä' == Char)  return 96u;
    else if('Ö' == Char) return 97u;
    else if('Ü' == Char) return 98u;
    else if('ä' == Char) return 99u;
    else if('ö' == Char) return 100u;
    else if('ü' == Char) return 101u;
    /* for all others only add offset */
    else if(Char >= TEXT_ASCII_CHAR_MIN) {
        return Char + TEXT_ASCII_TABLE_OFFSET;
    }
    return 0u;
} /* convertCharToFontIndex */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/

