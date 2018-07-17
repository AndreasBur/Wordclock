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
AnimationFont::AnimationFont(Display* Display) : wcTransformation(Display)
{
    pDisplay = Display;
    Shift.Char = STD_NULL_CHARACTER;
    Shift.Font = FONT_NONE;
    Shift.State = SHIFT_STATE_IDLE;
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
stdReturnType AnimationFont::setChar(byte Column, byte Row, char Char, FontType Font)
{
    byte FontIndex;
    if(convertCharToFontIndex(Char, FontIndex) == E_NOT_OK) return E_NOT_OK;

#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    //if(FONT_5X8) return
#endif
#if(ANIMATION_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar;
        if(Font7x9.getChar(FontIndex, FontChar) == E_OK) {
            return setCharFontHorizontal(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
        } else {
            return E_NOT_OK;
        }
    }
#endif
#if(ANIMATION_SUPPORT_FONT_7X10 == STD_ON)
    //if(FONT_7X10) return Font7x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_9X10 == STD_ON)
    //if(FONT_9X10) return Font9x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_10X10 == STD_ON)
    //if(FONT_10X10) return Font10x10.getHeight();
#endif
    if(FONT_NONE) return E_NOT_OK;
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

#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    //if(Font == FONT_5X8) return
#endif
#if(ANIMATION_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) {
        FontCourierNew7x9::FontCharType FontChar = Font7x9.getCharFast(FontIndex);
        setCharFontHorizontalFast(Column, Row, FontChar, FONT_COURIER_NEW_7X9_HEIGHT);
    }
#endif
#if(ANIMATION_SUPPORT_FONT_7X10 == STD_ON)
    //if(Font == FONT_7X10) return Font7x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_9X10 == STD_ON)
    //if(Font == FONT_9X10) return Font9x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_10X10 == STD_ON)
    //if(Font == FONT_10X10) return Font10x10.getHeight();
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
    Shift.Counter = getFontWidth(Font);
}


/******************************************************************************************************************************************************
  setTextWithShift()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setTextWithShift(char* Text, FontType Font)
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
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getHeight();
#endif
#if(ANIMATION_SUPPORT_FONT_10X10 == STD_ON)
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
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getWidth();
#endif
#if(ANIMATION_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getWidth();
#endif
#if(ANIMATION_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) Font7x10.getWidth();
#endif
#if(ANIMATION_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getWidth();
#endif
#if(ANIMATION_SUPPORT_FONT_10X10 == STD_ON)
    if(Font == FONT_10X10) return Font10x10.getWidth();
#endif
    return 0;
} /* getFontWidth */


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
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return Font5x8.getOrientation();
#endif
#if(ANIMATION_SUPPORT_FONT_7X9 == STD_ON)
    if(Font == FONT_7X9) return Font7x9.getOrientation();
#endif
#if(ANIMATION_SUPPORT_FONT_7X10 == STD_ON)
    if(Font == FONT_7X10) return Font7x10.getOrientation();
#endif
#if(ANIMATION_SUPPORT_FONT_9X10 == STD_ON)
    if(Font == FONT_9X10) return Font9x10.getOrientation();
#endif
#if(ANIMATION_SUPPORT_FONT_10X10 == STD_ON)
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
    if(Shift.State == SHIFT_STATE_BUSY) {
        /* If end of string is not reached, load next char */
        if(*Shift.Text != STD_NULL_CHARACTER) {
            Shift.Char = *Shift.Text;
            Shift.State = SHIFT_STATE_BUSY;
            Shift.Counter = getFontWidth(Shift.Font);
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
            //setChar(DISPLAY_NUMBER_OF_COLUMNS - (getFontWidth(Shift.Font) - Shift.Counter), getRowCenter(Shift.Font), Shift.Char, Shift.Font);
            setCharFast(DISPLAY_NUMBER_OF_COLUMNS - (getFontWidth(Shift.Font) - Shift.Counter), getRowCenter(Shift.Font), Shift.Char, Shift.Font);
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
stdReturnType AnimationFont::setCharFontHorizontal(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte Height)
{
    stdReturnType ReturnValue{E_OK};

    for(byte FontColumn = 0; FontColumn < FontChar.getWidth(); FontColumn++)
    {
        RowType CharRow;
        byte ColumnAbs = Column + FontColumn;
        if(FontChar.getRow(FontColumn, CharRow) == E_NOT_OK) {
            ReturnValue = E_NOT_OK;
            break;
        }
        setCharRow(CharRow, ColumnAbs, Row, Height);
    }
    return ReturnValue;
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
stdReturnType AnimationFont::setCharRow(RowType CharRow, byte Column, byte Row, byte Height)
{
    stdReturnType ReturnValue{E_OK};

    for(byte FontRow = 0; FontRow < Height; FontRow++)
    {
        byte RowAbs = Row + Height - 1 - FontRow;
        if(Column < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            if(pDisplay->writePixel(Column, RowAbs, bitRead(CharRow, FontRow)) == E_NOT_OK) {
                ReturnValue = E_NOT_OK;
                break;
            }
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
void AnimationFont::setCharFontHorizontalFast(byte Column, byte Row, const FontCharHorizontal<RowType, RowsSize>& FontChar, byte Height)
{
    for(byte FontColumn = 0; FontColumn < FontChar.getWidth(); FontColumn++)
    {
        RowType CharRow = FontChar.getRowFast(FontColumn);
        byte ColumnAbs = Column + FontColumn;
        setCharRowFast(CharRow, ColumnAbs, Row, Height);
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
void AnimationFont::setCharRowFast(RowType CharRow, byte Column, byte Row, byte Height)
{
    for(byte FontRow = 0; FontRow < Height; FontRow++)
    {
        byte RowAbs = Row + Height - 1 - FontRow;
        if(Column < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
            pDisplay->writePixelFast(Column, RowAbs, bitRead(CharRow, FontRow));
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
//stdReturnType AnimationFont::setCharFontVertical(byte Column, byte Row, char Char, const byte* FontTable, byte FontWidth, byte FontHeight)
//{
    //stdReturnType ReturnValue{E_OK};
    //byte FontIndex = Char;
//
    //for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    //{
        //byte CharColumn = pgm_read_byte(&FontTable[(FontIndex * FontHeight) + FontRow]);
        //for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
        //{
            //byte ColumnAbs = Column + FontColumn;
            //byte RowAbs = Row + FontRow;
            //if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                //if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(CharColumn, FontColumn)) == E_NOT_OK) {
                    //ReturnValue = E_NOT_OK;
                    //break;
                //}
            //} else {
                //break;
            //}
        //}
    //}
    //return ReturnValue;
//} /* setCharFontVertical */


/******************************************************************************************************************************************************
  setCharFontVerticalFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharFontVerticalFast(byte Column, byte Row, char Char, const byte* FontTable, byte FontWidth, byte FontHeight)
{
    byte FontIndex = Char;

    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte CharColumn = pgm_read_byte(&FontTable[(FontIndex * FontHeight) + FontRow]);
        for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
        {
            byte ColumnAbs = Column + FontColumn;
            byte RowAbs = Row + FontRow;
            if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                pDisplay->writePixelFast(ColumnAbs, RowAbs, bitRead(CharColumn, FontColumn));
            } else {
                break;
            }
        }
    }
} /* setCharFontVerticalFast */


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
stdReturnType AnimationFont::convertCharToFontIndex(char Char, byte& Index)
{
    stdReturnType ReturnValue = E_NOT_OK;

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
 
