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
  Constructor of AnimationCommon
******************************************************************************************************************************************************/
/*! \brief          AnimationCommon Constructor
 *  \details        Instantiation of the AnimationCommon library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationCommon::AnimationCommon()
{

} /* AnimationCommon */


/******************************************************************************************************************************************************
  Destructor of AnimationCommon
******************************************************************************************************************************************************/
AnimationCommon::~AnimationCommon()
{

} /* ~AnimationCommon */


/******************************************************************************************************************************************************
  isPixelPartOfWord()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean AnimationCommon::isPixelPartOfClockWords(Clock::ClockWordsTableType ClockWordsTable, byte Column, byte Row)
{
    DisplayWords Words;

    for(uint8_t WordIndex = 0; WordIndex < CLOCK_WORDS_TABLE_TYPE_SIZE; WordIndex++) {
        if(ClockWordsTable[WordIndex] == DisplayWords::WORD_NONE) { break; }
        DisplayWords::Word Word = Words.getDisplayWordFast(ClockWordsTable[WordIndex]);
        if(Word.Row == Row) { if(Column >= Word.Column && Column < Word.Column + Word.Length) { return true; } }
    }
    return false;
} /* isPixelPartOfWord */

/******************************************************************************************************************************************************
  isPixelPartOfWord()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean AnimationCommon::isPixelPartOfClockWords(Clock::ClockWordsTableType ClockWordsTable, byte Index)
{
    byte Row = Index % DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index / DISPLAY_NUMBER_OF_COLUMNS;

    return isPixelPartOfClockWords(ClockWordsTable, Column, Row);
} /* isPixelPartOfWord */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 