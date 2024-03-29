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
/**     \file       AnimationFlicker.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_FLICKER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationFlicker.h"


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
void AnimationFlicker::init()
{
    Animation::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationFlicker::setTime(byte sHour, byte sMinute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(sHour < CLOCK_NUMBER_OF_HOURS_PER_DAY && sMinute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        Hour = sHour;
        Minute = sMinute;
        State = STATE_CLEAR_TIME;
        returnValue = E_OK;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationFlicker::task()
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
void AnimationFlicker::reset()
{
    Minute = 0u;
    Hour = 0u;
    IsClockSet = false;
    FlickerCounter = ANIMATION_FLICKER_COUNTER_INIT_VALUE;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationFlicker::clearTimeTask()
{

    IsClockSet = !IsClockSet;

    if(IsClockSet) {
        Display::getInstance().disable();
    } else {
        Display::getInstance().enable();
    }

    if(FlickerCounter-- <= 0u) State = STATE_SET_TIME;
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationFlicker::setTimeTask()
{
    Display::getInstance().clear();
    Clock::getInstance().setTime(Hour, Minute);
    State = STATE_IDLE;
    reset();
} /* setTimeTask */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
