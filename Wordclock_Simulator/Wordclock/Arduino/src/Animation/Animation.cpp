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
/**     \file       Animation.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "Animation.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  CONSTRUCTOR OF Animation
******************************************************************************************************************************************************/
/*! \brief          Template Constructor
 *  \details        Instantiation of the Template library
 *
 *  \return         -
******************************************************************************************************************************************************/
Animation::Animation(Display* Display)
{
    pDisplay = Display;
} /* Animation */


/******************************************************************************************************************************************************
  DESTRUCTOR OF Animation
******************************************************************************************************************************************************/
Animation::~Animation()
{

} /* ~Animation */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Animation::init()
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
stdReturnType Animation::setChar(byte Column, byte Row, unsigned char Char, AnimationFontType Font)
{
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
    if(Font == ANIMATION_FONT_4X6) return setCharFontHorizontal(Column, Row, Char, &Font_4x6[0][0], ANIMATION_FONT_4X6_WIDTH, ANIMATION_FONT_4X6_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    if(Font == ANIMATION_FONT_5X8) return setCharFontHorizontal(Column, Row, Char, &Font_5x8[0][0], ANIMATION_FONT_5X8_WIDTH, ANIMATION_FONT_5X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
    if(Font == ANIMATION_FONT_6X8) return setCharFontHorizontal(Column, Row, Char, &Font_6x8[0][0], ANIMATION_FONT_6X8_WIDTH, ANIMATION_FONT_6X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
    if(Font == ANIMATION_FONT_8X8) return setCharFontHorizontal(Column, Row, Char, &Font_8x8[0][0], ANIMATION_FONT_8X8_WIDTH, ANIMATION_FONT_8X8_HEIGHT);
#endif

#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
    if(Font == ANIMATION_FONT_6X10) return setCharFontVertical(Column, Row, Char, &Font_6x10[0][0], ANIMATION_FONT_6X10_WIDTH, ANIMATION_FONT_6X10_HEIGHT);
#endif
    return E_NOT_OK;
} /* setChar */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  setCharFontHorizontal()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Animation::setCharFontHorizontal(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    stdReturnType ReturnValue = E_OK;
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontWidth) + FontColumn]);
        for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontHeight - 1 - FontRow;
            if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontRow)) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    return ReturnValue;
} /* setCharFontHorizontal */


/******************************************************************************************************************************************************
  setCharFontVertical()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Animation::setCharFontVertical(byte Column, byte Row, unsigned char Char, const unsigned char* FontTable, byte FontWidth, byte FontHeight)
{
    stdReturnType ReturnValue = E_OK;
    byte FontIndex, ColumnAbs, RowAbs;

    FontIndex = Char;

    for(byte FontRow = 0; FontRow < FontHeight; FontRow++)
    {
        byte Font_EntryItem = pgm_read_byte(&FontTable[(FontIndex * FontHeight) + FontRow]);
        for(byte FontColumn = 0; FontColumn < FontWidth; FontColumn++)
        {
            ColumnAbs = Column + FontColumn;
            RowAbs = Row + FontRow;
            if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_EntryItem, FontColumn)) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    return ReturnValue;
} /* setCharVertical */


/******************************************************************************************************************************************************
  convertCharToSprite()
******************************************************************************************************************************************************/
/*! \brief          converts char to sprite
 *  \details        this function converts the ASCII char to sprite index
 *
 *  \param[in]      Char            char to convert to sprite
 *  \param[out]     SpriteIndex     appropriate sprite
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
stdReturnType Animation::convertCharToFontIndex(unsigned char Char, byte* Index)
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
