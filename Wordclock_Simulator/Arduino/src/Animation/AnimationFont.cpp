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
} /* AnimationFont */


/******************************************************************************************************************************************************
  Destructor of AnimationFont
******************************************************************************************************************************************************/
AnimationFont::~AnimationFont()
{

} /* ~AnimationFont */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::init()
{

} /* init */


/******************************************************************************************************************************************************
  setChar()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationFont::setChar(byte Column, byte Row, unsigned char Char, FontType Font)
{
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
    if(Font == FONT_4X6) return setCharFontHorizontal(Column, Row, Char, &Font_4x6[0][0], ANIMATION_FONT_4X6_WIDTH, ANIMATION_FONT_4X6_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == FONT_5X8) return setCharFontHorizontal(Column, Row, Char, &Font_5x8[0][0], ANIMATION_FONT_5X8_WIDTH, ANIMATION_FONT_5X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
    if(Font == FONT_6X8) return setCharFontHorizontal(Column, Row, Char, &Font_6x8[0][0], ANIMATION_FONT_6X8_WIDTH, ANIMATION_FONT_6X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
    if(Font == FONT_8X8) return setCharFontHorizontal(Column, Row, Char, &Font_8x8[0][0], ANIMATION_FONT_8X8_WIDTH, ANIMATION_FONT_8X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
    if(Font == FONT_6X10) return setCharFontVertical(Column, Row, Char, &Font_6x10[0][0], ANIMATION_FONT_6X10_WIDTH, ANIMATION_FONT_6X10_HEIGHT);
#endif
    return E_NOT_OK;
} /* setChar */


/******************************************************************************************************************************************************
  setCharWithShift()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharWithShift(unsigned char Char, FontType Font)
{
    State = STATE_CHAR_SHIFT;
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
void AnimationFont::setTextWithShift(unsigned char* Text, FontType Font)
{
    State = STATE_TEXT_SHIFT;
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
    switch(Font) {
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
        case FONT_4X6:
            return ANIMATION_FONT_4X6_HEIGHT;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
        case FONT_5X8:
            return ANIMATION_FONT_5X8_HEIGHT;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
        case FONT_6X8:
            return ANIMATION_FONT_6X8_HEIGHT;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
        case FONT_6X10:
            return ANIMATION_FONT_6X10_HEIGHT;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
        case FONT_8X8:
            return ANIMATION_FONT_8X8_HEIGHT;
            break;
#endif
        default:
            return 0;
            break;
    }
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
    switch(Font) {
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
        case FONT_4X6:
            return ANIMATION_FONT_4X6_WIDTH;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
        case FONT_5X8:
            return ANIMATION_FONT_5X8_WIDTH;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
        case FONT_6X8:
            return ANIMATION_FONT_6X8_WIDTH;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
        case FONT_6X10:
            return ANIMATION_FONT_6X10_WIDTH;
            break;
#endif
#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
        case FONT_8X8:
            return ANIMATION_FONT_8X8_WIDTH;
            break;
#endif
        default:
            return 0;
            break;
    }
} /* getFontWidth */


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
            Shift.Counter = getFontWidth(Shift.Font);
            charShiftTask();
            Shift.Text++;
        } else { /* otherwise task has finished */
            State = STATE_IDLE;
            wcTransformation.shiftLeft();
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
            setChar(DISPLAY_NUMBER_OF_COLUMNS - (getFontWidth(Shift.Font) - Shift.Counter), getRowCenter(Shift.Font), Shift.Char, Shift.Font);
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
stdReturnType AnimationFont::setCharFontHorizontal(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    stdReturnType ReturnValue{E_OK};
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontWidth) + FontColumn]);
        for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontHeight - 1 - FontRow;
            if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontRow)) == E_NOT_OK) {
                    ReturnValue = E_NOT_OK;
                    break;
                }
            } else {
                break;
            }
        }
    }
    return ReturnValue;
} /* setCharFontHorizontal */


/******************************************************************************************************************************************************
  setCharFontHorizontalFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharFontHorizontalFast(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontWidth) + FontColumn]);
        for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontHeight - 1 - FontRow;
            if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                pDisplay->writePixelFast(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontRow));
            } else {
                break;
            }
        }
    }
} /* setCharFontHorizontalFast */


/******************************************************************************************************************************************************
  setCharFontVertical()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationFont::setCharFontVertical(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    stdReturnType ReturnValue{E_OK};
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontHeight) + FontRow]);
        for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontRow;
            if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontColumn)) == E_NOT_OK) {
                    ReturnValue = E_NOT_OK;
                    break;
                }
            } else {
                break;
            }
        }
    }
    return ReturnValue;
} /* setCharFontVertical */


/******************************************************************************************************************************************************
  setCharFontVerticalFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationFont::setCharFontVerticalFast(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontHeight) + FontRow]);
        for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontRow;
            if(ColumnAbs < DISPLAY_NUMBER_OF_COLUMNS && RowAbs < DISPLAY_NUMBER_OF_ROWS) {
                pDisplay->writePixelFast(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontColumn));
            } else {
                break;
            }
        }
    }
} /* setCharFontVerticalFast */


/******************************************************************************************************************************************************
  convertCharToSprite()
******************************************************************************************************************************************************/
/*! \brief          converts char to sprite
 *  \details        this function converts the ASCII char to sprite index
 *
 *  \param[in]      Char            char to convert to sprite
 *  \param[out]     Index     appropriate sprite
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
stdReturnType AnimationFont::convertCharToFontIndex(unsigned char Char, byte* Index)
{
    stdReturnType ReturnValue = E_NOT_OK;
    *Index = Char + 0;
    /* for umlauts we need a special treatment */
    //if('Ä' == Char) { *Index = 95; ReturnValue = E_OK; }
    //else if('Ö' == Char) { *Index = 153; ReturnValue = E_OK; }
    //else if('Ü' == Char) { *Index = 154; ReturnValue = E_OK; }
    //else if('ä' == Char) { *Index = 132; ReturnValue = E_OK; }
    //else if('ö' == Char) { *Index = 148; ReturnValue = E_OK; }
    //else if('ü' == Char) { *Index = 129; ReturnValue = E_OK; }
    /* for all others only add offset */
    //else if(Char >= 0 && Char <= 255) { *Index = Char + 0; ReturnValue = E_OK; }

    return ReturnValue;
} /* convertCharToSprite */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
