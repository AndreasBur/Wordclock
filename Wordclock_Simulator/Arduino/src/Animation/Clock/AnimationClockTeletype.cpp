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
/**     \file       AnimationClockTeletype.cpp
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
#include "AnimationClockTeletype.h"


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
  Constructor of AnimationClockTeletype
******************************************************************************************************************************************************/
/*! \brief          AnimationClockTeletype Constructor
 *  \details        Instantiation of the AnimationClockTeletype library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockTeletype::AnimationClockTeletype()
{
    pDisplay = nullptr;
    pClock = nullptr;
    reset();
} /* AnimationClockTeletype */


/******************************************************************************************************************************************************
  Destructor of AnimationClockTeletype
******************************************************************************************************************************************************/
AnimationClockTeletype::~AnimationClockTeletype()
{

} /* ~AnimationClockTeletype */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockTeletype::init(Display* Display, Clock* Clock)
{
    pDisplay = Display;
    pClock = Clock;
    setState(AnimationClockCommon::STATE_IDLE);
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
stdReturnType AnimationClockTeletype::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && getState() == AnimationClockCommon::STATE_IDLE) {
        ReturnValue = E_OK;
        CurrentWordIndex = 0;
        CurrentCharIndex = 0;
        CurrentWordLength = Words.getDisplayWordLengthFast(ClockWordsTable[CurrentWordIndex]);
        setState(AnimationClockCommon::STATE_SET_TIME);
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
void AnimationClockTeletype::task()
{
    if(getState() == AnimationClockCommon::STATE_SET_TIME) {
        if(CurrentCharIndex >= CurrentWordLength) {
            if(setNextWordIndex() == E_NOT_OK) {
                setState(AnimationClockCommon::STATE_IDLE);
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
void AnimationClockTeletype::reset()
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
stdReturnType AnimationClockTeletype::setNextWordIndex()
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
 