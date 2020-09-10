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
/**     \file       AnimationClockExplode.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_EXPLODE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationClockExplode.h"


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
void AnimationClockExplode::init()
{
    AnimationClockCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationClockExplode::setTime(byte Hour, byte Minute)
{
    StdReturnType ReturnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        ReturnValue = E_OK;
        State = STATE_CLEAR_TIME;
    }
    return ReturnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClockExplode::task()
{
    if(State == STATE_CLEAR_TIME) clearTimeTask();
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationClockExplode::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    CurrentWordIndex = 0u;
    CurrentWordLength = 0u;
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationClockExplode::clearTimeTask()
{
    if(setNextWord() == E_NOT_OK) {
        State = STATE_IDLE;
    }
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setNextWord()
******************************************************************************************************************************************************/
StdReturnType AnimationClockExplode::setNextWord()
{
    byte nextWordIndex = CurrentWordIndex + CurrentWordLength;

    do {
        if(Display::getInstance().getPixelFast(nextWordIndex)) {
            CurrentWordIndex = nextWordIndex;
            break;
        } else {
            nextWordIndex++;
        }
    } while(nextWordIndex < DISPLAY_NUMBER_OF_PIXELS);

    if(nextWordIndex < DISPLAY_NUMBER_OF_PIXELS) {
        setNextWordLength();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setNextWord */

/******************************************************************************************************************************************************
  setNextWordLength()
******************************************************************************************************************************************************/
void AnimationClockExplode::setNextWordLength()
{
    CurrentWordLength = 1u;

    for(byte index = CurrentWordIndex + 1u; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
        if(Display::getInstance().getPixelFast(index)) {
            CurrentWordLength++;
        } else {
            break;
        }
    }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
