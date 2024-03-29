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
#define _ANIMATION_CLOCK_TELETYPE_SOURCE_

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
  init()
******************************************************************************************************************************************************/
void AnimationTeletype::init()
{
    Animation::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationTeletype::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        CurrentWordIndex = 0u;
        CurrentCharIndex = 0u;
        CurrentWordLength = Words.getDisplayWordLengthFast(ClockWordsTable[CurrentWordIndex]);
        State = STATE_SET_TIME;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationTeletype::task()
{
    if(State == STATE_SET_TIME) {
        if(CurrentCharIndex >= CurrentWordLength) {
            if(setNextWordIndex() == E_NOT_OK) {
                State = STATE_IDLE;
                return;
            }
            CurrentCharIndex = 0u;
            CurrentWordLength = Words.getDisplayWordLengthFast(ClockWordsTable[CurrentWordIndex]);
        }
        CurrentCharIndex++;
        Display::getInstance().setWordFast(ClockWordsTable[CurrentWordIndex], CurrentCharIndex);
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationTeletype::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    CurrentWordIndex = 0u;
    CurrentWordLength = 0u;
    CurrentCharIndex = 0u;
} /* reset */


/******************************************************************************************************************************************************
  setNextWordIndex()
******************************************************************************************************************************************************/
StdReturnType AnimationTeletype::setNextWordIndex()
{
    if(CurrentWordIndex + 1u < static_cast<byte>(ClockWordsTable.size())) {
        CurrentWordIndex++;
        return E_OK;
    }
    return E_NOT_OK;
} /* setNextWordIndex */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 