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
/**     \file       AnimationTeletype.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_TELETYPE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationTeletype.h"


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
  CONSTRUCTOR OF ANIMATION_TELETYPE
******************************************************************************************************************************************************/
/*! \brief          AnimationTeletype Constructor
 *  \details        Instantiation of the AnimationTeletype library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationTeletype::AnimationTeletype()
{
    pDisplay = nullptr;
    pClock = nullptr;
    State = STATE_UNINIT;
    reset();
} /* AnimationTeletype */


/******************************************************************************************************************************************************
  DESTRUCTOR OF ANIMATION_TELETYPE
******************************************************************************************************************************************************/
AnimationTeletype::~AnimationTeletype()
{

} /* ~AnimationTeletype */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationTeletype::init(Display* Display, Clock* Clock)
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
stdReturnType AnimationTeletype::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        ReturnValue = E_OK;
        CurrentWordIndex = 0;
        CurrentCharIndex = 0;
        CurrentWordLength = Words.getDisplayWordLengthFast(ClockWordsTable[CurrentWordIndex]);
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
void AnimationTeletype::task()
{
    if(State == STATE_WORKING) {
        if(CurrentCharIndex >= CurrentWordLength) {
            if(setNextWordIndex() == E_NOT_OK) {
                State = STATE_IDLE;
                return;
            }
            CurrentCharIndex = 0;
            CurrentWordLength = Words.getDisplayWordLengthFast(ClockWordsTable[CurrentWordIndex]);
        }
        CurrentCharIndex++;
        pDisplay->setWordFast(ClockWordsTable[CurrentWordIndex], CurrentCharIndex);
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
void AnimationTeletype::reset()
{
    for(auto& Word : ClockWordsTable) { Word = DisplayWords::WORD_NONE; }
    CurrentWordIndex = 0;
    CurrentWordLength = 0;
    CurrentCharIndex = 0;
} /* reset */


/******************************************************************************************************************************************************
  setNextWordIndex()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationTeletype::setNextWordIndex()
{
    if(CurrentWordIndex + 1 < CLOCK_WORDS_TABLE_TYPE_SIZE) {
        CurrentWordIndex++;
        return E_OK;
    }
    return E_NOT_OK;
} /* setNextWordIndex */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 