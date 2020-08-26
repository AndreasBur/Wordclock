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
/**     \file       AnimationClockFade.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_FADE_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationClockFade.h"


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
  init()
******************************************************************************************************************************************************/
void AnimationClockFade::init()
{
    AnimationClockCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationClockFade::setTime(byte sHour, byte sMinute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(sHour < CLOCK_NUMBER_OF_HOURS_PER_DAY && sMinute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        Hour = sHour;
        Minute = sMinute;
        State = STATE_CLEAR_TIME;
        DisplayBrightness = Display::getInstance().getBrightness();
        returnValue = E_OK;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClockFade::task()
{
    if(State == STATE_CLEAR_TIME) { clearTimeTask(); }
    else if(State == STATE_SET_TIME) { setTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationClockFade::reset()
{
    DisplayBrightness = 0u;
    Hour = 0u;
    Minute = 0u;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationClockFade::clearTimeTask()
{
    if(DisplayBrightness > 0u) {
        DisplayBrightness--;
    } else {
        setStateToSetTime();
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationClockFade::setTimeTask()
{
    if(DisplayBrightness < Display::getInstance().getBrightness()) {
        DisplayBrightness++;
    } else {
        State = STATE_IDLE;
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationClockFade::setStateToSetTime()
{
    State = STATE_SET_TIME;
    Clock::getInstance().setTime(Hour, Minute);
    DisplayBrightness = 0u;
} /* setStateToSetTime */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
