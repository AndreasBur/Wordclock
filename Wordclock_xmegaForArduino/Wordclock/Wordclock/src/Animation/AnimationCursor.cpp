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
#define _ANIMATION_CLOCK_CURSOR_SOURCE_

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
  init()
******************************************************************************************************************************************************/
void AnimationCursor::init()
{
    Animation::init(STATE_IDLE);
    reset();
} /* init */

/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationCursor::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        CurrentPixelIndex = 0u;
        State = STATE_SET_TIME;
    }
    return returnValue;
} /* setTime */

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationCursor::task()
{
    if(State == STATE_SET_TIME) {
        if(CurrentPixelIndex < DISPLAY_NUMBER_OF_PIXELS) { Display::getInstance().setPixelFast(CurrentPixelIndex); }
        if(CurrentPixelIndex > 0u) {
            if(isPixelPartOfClockWords(ClockWordsTable, CurrentPixelIndex - 1u) == false) {
                Display::getInstance().clearPixelFast(CurrentPixelIndex - 1u);
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
void AnimationCursor::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    CurrentPixelIndex = 0u;
} /* reset */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
