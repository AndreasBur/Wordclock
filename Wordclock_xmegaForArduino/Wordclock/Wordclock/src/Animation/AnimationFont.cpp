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
/**     \file       AnimationFont.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_FONT_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationFont.h"


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
  Constructor of AnimationFont
******************************************************************************************************************************************************/
/*! \brief          AnimationFont Constructor
 *  \details        Instantiation of the AnimationFont library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationFont::AnimationFont()
{
    Shift.Char = STD_NULL_CHARACTER;
    Shift.Font = FONT_NONE;
    Shift.State = SHIFT_STATE_IDLE;
	TaskCycle = ANIMATION_FONT_TASK_CYCLE_INIT_VALUE;
    State = STATE_UNINIT;
} /* AnimationFont */


/******************************************************************************************************************************************************
  Destructor of AnimationFont
******************************************************************************************************************************************************/
AnimationFont::~AnimationFont()
{

} /* ~AnimationFont */


/******************************************************************************************************************************************************
  setChar()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType AnimationFont::setChar(byte Column, byte Row, char Char, FontType Font)
{
    byte FontIndex;
    if(convertCharToFontIndex(Char, FontIndex) == E_NOT_OK) return E_NOT_OK;

#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) {
        FontSprite5x8::FontCharType FontChar;
        if(Font5x8.getChar(FontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontVertical(Column, Row, FontChar, FONT_SPRITE_5X8_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar;
        if(Font7x9.getChar(FontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) {
        FontCourierNew7x10::FontCharType FontChar;
        if(Font7x10.getChar(FontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_COURIER_NEW_7X10_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) {
        FontLucidaSans9x10::FontCharType FontChar;
        if(Font9x10.getChar(FontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontVertical(Column, Row, FontChar, FONT_LUCIDA_SANS_9X10_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) {
        FontTahoma10x10::FontCharType FontChar;
        if(Font10x10.getChar(FontIndex, FontChar) == E_NOT_OK) { return E_NOT_OK; }
        return setCharFontHorizontal(Column, Row, FontChar, FONT_TAHOMA_10X10_HEIGHT);
    }
#endif
    return E_NOT_OK;
} /* setChar */


/******************************************************************************************************************************************************
  setCharFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharFast(byte Column, byte Row, char Char, FontType Font)
{
    byte FontIndex = convertCharToFontIndexFast(Char);

#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) {
        FontSprite5x8::FontCharType FontChar = Font5x8.getCharFast(FontIndex);
        setCharFontVerticalFast(Column, Row, FontChar, FONT_SPRITE_5X8_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar = Font7x9.getCharFast(FontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) {
        FontCourierNew7x10::FontCharType FontChar = Font7x10.getCharFast(FontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_COURIER_NEW_7X10_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) {
        FontLucidaSans9x10::FontCharType FontChar = Font9x10.getCharFast(FontIndex);
        setCharFontVertical(Column, Row, FontChar, FONT_LUCIDA_SANS_9X10_HEIGHT);
    }
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) {
        FontTahoma10x10::FontCharType FontChar = Font10x10.getCharFast(FontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_TAHOMA_10X10_HEIGHT);
    }
#endif
} /* setCharFast */


/******************************************************************************************************************************************************
  setCharWithShift()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharWithShift(char Char, FontType Font)
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setTextWithShift(const char* Text, FontType Font)
{
    State = STATE_TEXT_SHIFT;
    Shift.State = SHIFT_STATE_IDLE;
    Shift.Font = Font;
    Shift.Text = Text;
}


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::task()
{
    if(State == STATE_TEXT_SHIFT) stringShiftTask();
    if(State == STATE_CHAR_SHIFT) charShiftTask();
} /* task */


/******************************************************************************************************************************************************
  getFontHeight()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
byte AnimationFont::getFontHeight(FontType Font)
{
#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getHeight();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getHeight();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getHeight();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getHeight();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getHeight();
#endif
    return 0;
} /* getFontHeight */


/******************************************************************************************************************************************************
  getFontWidth()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
byte AnimationFont::getFontWidth(FontType Font)
{
#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getWidth();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getWidth();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getWidth();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getWidth();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getWidth();
#endif
    return 0;
} /* getFontWidth */


/******************************************************************************************************************************************************
  getFontCharWidth()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
byte AnimationFont::getFontCharWidth(FontType Font, char Char)
{
    byte Index = convertCharToFontIndexFast(Char);

#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getCharWidthFast(Index);
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getCharWidthFast(Index);
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getCharWidthFast(Index);
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getCharWidthFast(Index);
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getCharWidthFast(Index);
#endif
    return 0;
} /* getFontCharWidth */


/******************************************************************************************************************************************************
  getFontOrientation()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
Orientation AnimationFont::getFontOrientation(FontType Font)
{
#if(ANIMATION_FONT_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getOrientation();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getOrientation();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getOrientation();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getOrientation();
#endif
#if(ANIMATION_FONT_SUPPORT_FONT_10X10 == STD_ON)
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::stringShiftTask()
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::charShiftTask()
{
    if(Shift.State == SHIFT_STATE_BUSY) {
        wcTransformation.shiftLeft();
        if(Shift.Counter == 0) {
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename RowType, byte RowsSize>
StdReturnType AnimationFont::setCharFontHorizontal(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte FontHeight)
{
    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        RowType CharRow;
        byte RowAbs = Row + FontRow;
        if(FontChar.getRow(FontRow, CharRow) == E_NOT_OK) { return E_NOT_OK; }
        if(setCharRow(CharRow, Column, RowAbs, FontChar.getWidth()) == E_NOT_OK) { return E_NOT_OK; }
    }
    return E_OK;
} /* setCharFontHorizontal */


/******************************************************************************************************************************************************
  setCharRow()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename RowType>
StdReturnType AnimationFont::setCharRow(RowType CharRow, byte Column, byte RowAbs, byte FontWidth)
{
    StdReturnType ReturnValue{E_OK};

    for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
    {
        byte ColumnAbs = Column + FontColumn;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::getInstance().writePixel(ColumnAbs, RowAbs, bitRead(CharRow, FontColumn)) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
        } else {
            break;
        }
    }
    return ReturnValue;
} /* setCharRow */


/******************************************************************************************************************************************************
  setCharFontHorizontalFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename RowType, byte RowsSize>
void AnimationFont::setCharFontHorizontalFast(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte FontHeight)
{
    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        RowType CharRow = FontChar.getRowFast(FontRow);
        byte RowAbs = Row + FontRow;
        setCharRowFast(CharRow, Column, RowAbs, FontChar.getWidth());
    }
} /* setCharFontHorizontalFast */


/******************************************************************************************************************************************************
  setCharRowFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename RowType>
void AnimationFont::setCharRowFast(RowType CharRow, byte Column, byte RowAbs, byte FontWidth)
{
    for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
    {
        byte ColumnAbs = Column + FontColumn;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            Display::getInstance().writePixelFast(ColumnAbs, RowAbs, bitRead(CharRow, FontColumn));
        } else {
            break;
        }
    }
} /* setCharRowFast */


/******************************************************************************************************************************************************
  setCharFontVertical()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename ColumnType, byte ColumnsSize>
StdReturnType AnimationFont::setCharFontVertical(byte Column, byte Row, const FontCharVertical<ColumnType, ColumnsSize>& FontChar, byte FontHeight)
{
    for(byte FontColumn = 0; FontColumn < FontChar.getWidth(); FontColumn++)
    {
        ColumnType CharColumn;
        byte ColumnAbs = Column + FontColumn;
        if(FontChar.getColumn(FontColumn, CharColumn) == E_NOT_OK) { return E_NOT_OK; }
        if(setCharColumn(CharColumn, ColumnAbs, Row, FontHeight) == E_NOT_OK) { return E_NOT_OK; }
    }
    return E_OK;
} /* setCharFontVertical */


/******************************************************************************************************************************************************
  setCharColumn()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename ColumnType>
StdReturnType AnimationFont::setCharColumn(ColumnType CharColumn, byte ColumnAbs, byte Row, byte FontHeight)
{
    StdReturnType ReturnValue{E_OK};

    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte RowAbs = Row + FontRow;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            if(Display::getInstance().writePixel(ColumnAbs, RowAbs, bitRead(CharColumn, FontRow)) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
        } else {
            break;
        }
    }
    return ReturnValue;
} /* setCharColumn */


/******************************************************************************************************************************************************
  setCharFontVerticalFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename ColumnType, byte ColumnsSize>
void AnimationFont::setCharFontVerticalFast(byte Column, byte Row, const FontCharVertical<ColumnType, ColumnsSize>& FontChar, byte FontHeight)
{
    for(byte FontColumn = 0; FontColumn < FontChar.getWidth(); FontColumn++)
    {
        ColumnType CharColumn = FontChar.getColumnFast(FontColumn);
        byte ColumnAbs = Column + FontColumn;
        setCharColumnFast(CharColumn, ColumnAbs, Row, FontHeight);
    }
} /* setCharFontVerticalFast */


/******************************************************************************************************************************************************
  setCharColumnFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
template <typename ColumnType>
void AnimationFont::setCharColumnFast(ColumnType CharColumn, byte ColumnAbs, byte Row, byte FontHeight)
{
    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte RowAbs = Row + FontRow;
        if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            Display::getInstance().writePixelFast(ColumnAbs, RowAbs, bitRead(CharColumn, FontRow));
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
StdReturnType AnimationFont::convertCharToFontIndex(char Char, byte& Index)
{
    StdReturnType ReturnValue = E_NOT_OK;

    /* for umlauts we need a special treatment */
    if('Ä' == Char) { Index = 96; ReturnValue = E_OK; }
    else if('Ö' == Char) { Index = 97; ReturnValue = E_OK; }
    else if('Ü' == Char) { Index = 98; ReturnValue = E_OK; }
    else if('ä' == Char) { Index = 99; ReturnValue = E_OK; }
    else if('ö' == Char) { Index = 100; ReturnValue = E_OK; }
    else if('ü' == Char) { Index = 101; ReturnValue = E_OK; }
    /* for all others only add offset */
    else if(Char >= ANIMATION_FONT_ASCII_CHAR_MIN) {
        Index = Char + ANIMATION_FONT_ASCII_TABLE_OFFSET; ReturnValue = E_OK;
    }
    return ReturnValue;
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
byte AnimationFont::convertCharToFontIndexFast(char Char)
{
    /* for umlauts we need a special treatment */
    if('Ä' == Char)  return 96;
    else if('Ö' == Char) return 97;
    else if('Ü' == Char) return 98;
    else if('ä' == Char) return 99;
    else if('ö' == Char) return 100;
    else if('ü' == Char) return 101;
    /* for all others only add offset */
    else if(Char >= ANIMATION_FONT_ASCII_CHAR_MIN) {
        return Char + ANIMATION_FONT_ASCII_TABLE_OFFSET;
    }
    return 0;
} /* convertCharToFontIndex */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/

