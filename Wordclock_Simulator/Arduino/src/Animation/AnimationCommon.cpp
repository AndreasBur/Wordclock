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
/**     \file       AnimationCommon.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_COMMON_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationCommon.h"


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
  isPixelPartOfClockWords()
******************************************************************************************************************************************************/
bool AnimationCommon::isPixelPartOfClockWords(const ClockWords::WordsListType ClockWordsTable, byte Column, byte Row) const
{
    DisplayWords words;

    for(uint8_t wordIndex = 0u; wordIndex < ClockWordsTable.size(); wordIndex++) {
        if(ClockWordsTable[wordIndex] == DisplayWords::WORD_NONE) { break; }
        DisplayWord Word = words.getDisplayWordFast(ClockWordsTable[wordIndex]);
        if(Word.getRow() == Row) { if((Column >= Word.getColumn()) && (Column < Word.getColumn() + Word.getLength())) { return true; } }
    }
    return false;
} /* isPixelPartOfWord */

/******************************************************************************************************************************************************
  isPixelPartOfClockWords()
******************************************************************************************************************************************************/
bool AnimationCommon::isPixelPartOfClockWords(const ClockWords::WordsListType ClockWordsTable, byte Index) const
{
    byte row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return isPixelPartOfClockWords(ClockWordsTable, column, row);
} /* isPixelPartOfWord */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 