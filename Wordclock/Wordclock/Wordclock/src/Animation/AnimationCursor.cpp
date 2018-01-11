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
/**     \file       AnimationCursor.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CURSOR_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationCursor.h"


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
  Constructor of AnimationCursor
******************************************************************************************************************************************************/
/*! \brief          AnimationCursor Constructor
 *  \details        Instantiation of the AnimationCursor library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationCursor::AnimationCursor()
{
    pDisplay = nullptr;
    pClock = nullptr;
    State = STATE_UNINIT;
    reset();
} /* AnimationCursor */


/******************************************************************************************************************************************************
  Destructor of AnimationCursor
******************************************************************************************************************************************************/
AnimationCursor::~AnimationCursor()
{

} /* ~AnimationCursor */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationCursor::init(Display* Display, Clock* Clock)
{
    pDisplay = Display;
    pClock = Clock;
    State = STATE_IDLE;
    reset();
} /* init */


/******************************************************************************************************************************************************
  setClock()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationCursor::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        ReturnValue = E_OK;
        CurrentPixelIndex = 0;
        State = STATE_WORKING;
    }
    return ReturnValue;
} /* setClock */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationCursor::task()
{
    if(State == STATE_WORKING) {
        pDisplay->setPixelFast(CurrentPixelIndex);
        if(CurrentPixelIndex > 0) {
            if(isPixelPartOfClockWords(CurrentPixelIndex - 1) == false) { 
                pDisplay->clearPixelFast(CurrentPixelIndex - 1);
            }
        }
        if(CurrentPixelIndex >= DISPLAY_NUMBER_OF_PIXELS) State = STATE_IDLE;
        CurrentPixelIndex++;
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationCursor::reset()
{
    for(auto& Word : ClockWordsTable) { Word = DisplayWords::WORD_NONE; }
    CurrentPixelIndex = 0;
} /* reset */


/******************************************************************************************************************************************************
  isPixelPartOfWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
boolean AnimationCursor::isPixelPartOfClockWords(byte Index)
{
    byte Row, Column;
    pDisplay->indexToColumnAndRow(Index, Column, Row);

    for(uint8_t WordIndex = 0; WordIndex < CLOCK_WORDS_TABLE_TYPE_SIZE; WordIndex++) {
        if(ClockWordsTable[WordIndex] == DisplayWords::WORD_NONE) { break; }
        DisplayWords::Word Word = Words.getDisplayWordFast(ClockWordsTable[WordIndex]);
        if(Word.Row == Row) {
            if(Column >= Word.Column && Column < Word.Column + Word.Length) { 
                return true;   
            }
        }
    }
    return false;
} /* isPixelPartOfWord */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 