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
stdReturnType Animation::setChar(byte Column, byte Row, char Char, AnimationFontType Font)
{
	stdReturnType ReturnValue = E_NOT_OK;
	byte FontIndex, ColumnAbs, RowAbs;

	//convertCharToFontIndex(Char, &FontIndex);
	FontIndex = Char;

#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
	if(Font == ANIMATION_FONT_4X6)
	{
	    ReturnValue = E_OK;
        for(byte FontColumn = 0; FontColumn < ANIMATION_FONT_4X6_WIDTH; FontColumn++)
        {
			byte Font_4x6_EntryItem = pgm_read_byte(&Font_4x6[FontIndex][FontColumn]);
	        for(byte FontRow = 0; FontRow < ANIMATION_FONT_4X6_HEIGHT; FontRow++)
	        {
		        ColumnAbs = Column + FontColumn;
		        RowAbs = Row + ANIMATION_FONT_4X6_HEIGHT - 1 - FontRow;
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_4x6_EntryItem, FontRow)) == E_NOT_OK) ReturnValue = E_NOT_OK;
	        }
		}
	}
#endif

#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
	if(Font == ANIMATION_FONT_5X8)
	{
	    ReturnValue = E_OK;
        for(byte FontColumn = 0; FontColumn < ANIMATION_FONT_5X8_WIDTH; FontColumn++)
        {
			byte Font_5x8_EntryItem = pgm_read_byte(&Font_5x8[FontIndex][FontColumn]);
	        for(byte FontRow = 0; FontRow < ANIMATION_FONT_5X8_HEIGHT; FontRow++)
	        {
		        ColumnAbs = Column + FontColumn;
		        RowAbs = Row + ANIMATION_FONT_5X8_HEIGHT - 1 - FontRow;
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_5x8_EntryItem, FontRow)) == E_NOT_OK) ReturnValue = E_NOT_OK;
	        }
		}
	}
#endif

#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
	if(Font == ANIMATION_FONT_6X8)
	{
	    ReturnValue = E_OK;
        for(byte FontColumn = 0; FontColumn < ANIMATION_FONT_6X8_WIDTH; FontColumn++)
        {
			byte Font_6x8_EntryItem = pgm_read_byte(&Font_6x8[FontIndex][FontColumn]);
	        for(byte FontRow = 0; FontRow < ANIMATION_FONT_6X8_HEIGHT; FontRow++)
	        {
		        ColumnAbs = Column + FontColumn;
		        RowAbs = Row + ANIMATION_FONT_6X8_HEIGHT - 1 - FontRow;
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_6x8_EntryItem, FontRow)) == E_NOT_OK) ReturnValue = E_NOT_OK;
	        }
		}
	}
#endif

#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
	if(Font == ANIMATION_FONT_8X8)
	{
	    ReturnValue = E_OK;
        for(byte FontColumn = 0; FontColumn < ANIMATION_FONT_8X8_WIDTH; FontColumn++)
        {
			byte Font_8x8_EntryItem = pgm_read_byte(&Font_8x8[FontIndex][FontColumn]);
	        for(byte FontRow = 0; FontRow < ANIMATION_FONT_8X8_HEIGHT; FontRow++)
	        {
		        ColumnAbs = Column + FontColumn;
		        RowAbs = Row + ANIMATION_FONT_8X8_HEIGHT - 1 - FontRow;
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_8x8_EntryItem, FontRow)) == E_NOT_OK) ReturnValue = E_NOT_OK;
	        }
		}
	}
#endif

#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
	if(Font == ANIMATION_FONT_6X10)
	{
	    ReturnValue = E_OK;
	    for(byte FontRow = 0; FontRow < ANIMATION_FONT_6X10_HEIGHT; FontRow++)
        {
			byte Font_6x10_EntryItem = pgm_read_byte(&Font_6x10[FontIndex][FontRow]);
	        for(byte FontColumn = 0; FontColumn < ANIMATION_FONT_6X10_WIDTH; FontColumn++)
	        {
		        ColumnAbs = Column + FontColumn;
		        RowAbs = Row + FontRow;
                if(pDisplay->writePixel(ColumnAbs, RowAbs, bitRead(Font_6x10_EntryItem, FontColumn)) == E_NOT_OK) ReturnValue = E_NOT_OK;
	        }
		}
	}
#endif

	return ReturnValue;
} /* setChar */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

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
stdReturnType Animation::convertCharToFontIndex(char Char, byte* Index)
{
    stdReturnType ReturnValue = E_NOT_OK;

    /* for umlauts we need a special treatment */
    if('Ä' == Char) { *Index = 95; ReturnValue = E_OK; }
    else if('Ö' == Char) { *Index = 153; ReturnValue = E_OK; }
    else if('Ü' == Char) { *Index = 154; ReturnValue = E_OK; }
    else if('ä' == Char) { *Index = 132; ReturnValue = E_OK; }
    else if('ö' == Char) { *Index = 148; ReturnValue = E_OK; }
    else if('ü' == Char) { *Index = 129; ReturnValue = E_OK; }
    /* for all others only add offset */
    else if(Char >= 0 && Char <= 255) { *Index = Char + 0; ReturnValue = E_OK; }

    return ReturnValue;
} /* convertCharToSprite */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 