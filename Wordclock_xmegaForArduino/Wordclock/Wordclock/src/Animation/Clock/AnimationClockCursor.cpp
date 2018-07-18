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
/**     \file       AnimationClockCursor.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_CURSOR_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationClockCursor.h"


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
  Constructor of AnimationClockCursor
******************************************************************************************************************************************************/
/*! \brief          AnimationClockCursor Constructor
 *  \details        Instantiation of the AnimationClockCursor library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockCursor::AnimationClockCursor()
{
    reset();
} /* AnimationClockCursor */


/******************************************************************************************************************************************************
  Destructor of AnimationClockCursor
******************************************************************************************************************************************************/
AnimationClockCursor::~AnimationClockCursor()
{

} /* ~AnimationClockCursor */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockCursor::init(Display* Display, Clock* Clock)
{
    AnimationClockCommon::init(Display, Clock, AnimationClockCommon::STATE_IDLE);
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
stdReturnType AnimationClockCursor::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == AnimationClockCommon::STATE_IDLE) {
        ReturnValue = E_OK;
        CurrentPixelIndex = 0;
        State = AnimationClockCommon::STATE_SET_TIME;
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
void AnimationClockCursor::task()
{
    if(State == AnimationClockCommon::STATE_SET_TIME) {
        pDisplay->setPixelFast(CurrentPixelIndex);
        if(CurrentPixelIndex > 0) {
            if(isPixelPartOfClockWords(ClockWordsTable, CurrentPixelIndex - 1) == false) { 
                pDisplay->clearPixelFast(CurrentPixelIndex - 1);
            }
        }
        if(CurrentPixelIndex >= DISPLAY_NUMBER_OF_PIXELS) State = AnimationClockCommon::STATE_IDLE;
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
void AnimationClockCursor::reset()
{
    for(auto& Word : ClockWordsTable) { Word = DisplayWords::WORD_NONE; }
    CurrentPixelIndex = 0;
} /* reset */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 