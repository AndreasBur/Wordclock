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
/**     \file       AnimationClockCommon.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_COMMON_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationClockCommon.h"


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
  Constructor of AnimationClockCommon
******************************************************************************************************************************************************/
/*! \brief          AnimationClockCommon Constructor
 *  \details        Instantiation of the AnimationClockCommon library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockCommon::AnimationClockCommon()
{
    pDisplay = nullptr;
    pClock = nullptr;
    State = STATE_NONE;
} /* AnimationClockCommon */


/******************************************************************************************************************************************************
  Destructor of AnimationClockCommon
******************************************************************************************************************************************************/
AnimationClockCommon::~AnimationClockCommon()
{

} /* ~AnimationClockCommon */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockCommon::init(Display* Display, Clock* Clock, StateType sState)
{
    pDisplay = Display;
    pClock = Clock;
    State = sState;
} /* init */


/******************************************************************************************************************************************************
  isPixelPartOfWord()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean AnimationClockCommon::isPixelPartOfClockWords(const ClockWords::WordsListType ClockWordsTable, byte Column, byte Row) const
{
    DisplayWords Words;

    for(uint8_t WordIndex = 0; WordIndex < ClockWordsTable.size(); WordIndex++) {
        if(ClockWordsTable[WordIndex] == DisplayWords::WORD_NONE) { break; }
        DisplayWord Word = Words.getDisplayWordFast(ClockWordsTable[WordIndex]);
        if(Word.getRow() == Row) { if(Column >= Word.getColumn() && Column < Word.getColumn() + Word.getLength()) { return true; } }
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
boolean AnimationClockCommon::isPixelPartOfClockWords(const ClockWords::WordsListType ClockWordsTable, byte Index) const
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return isPixelPartOfClockWords(ClockWordsTable, Column, Row);
} /* isPixelPartOfWord */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 