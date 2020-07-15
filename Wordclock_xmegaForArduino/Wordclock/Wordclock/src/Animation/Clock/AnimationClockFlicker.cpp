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
/**     \file       AnimationClockFlicker.cpp
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
#include "AnimationClockFlicker.h"


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
  Constructor of AnimationClockFlicker
******************************************************************************************************************************************************/
/*! \brief          AnimationClockFlicker Constructor
 *  \details        Instantiation of the AnimationClockFlicker library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockFlicker::AnimationClockFlicker()
{
    reset();
} /* AnimationClockFlicker */


/******************************************************************************************************************************************************
  Destructor of AnimationClockFlicker
******************************************************************************************************************************************************/
AnimationClockFlicker::~AnimationClockFlicker()
{

} /* ~AnimationClockFlicker */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockFlicker::init()
{
    AnimationClockCommon::init(STATE_IDLE);
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
StdReturnType AnimationClockFlicker::setClock(byte sHour, byte sMinute)
{
    StdReturnType ReturnValue{E_NOT_OK};

    if(sHour < CLOCK_NUMBER_OF_HOURS_PER_DAY && sMinute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        Hour = sHour;
        Minute = sMinute;
        State = STATE_CLEAR_TIME;
        ReturnValue = E_OK;
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
void AnimationClockFlicker::task()
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockFlicker::reset()
{
    Minute = 0u;
    Hour = 0u;
    isClockSet = false;
    FlickerCounter = ANIMATION_CLOCK_FLICKER_COUNTER_INIT_VALUE;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockFlicker::clearTimeTask()
{

    isClockSet = !isClockSet;

    if(isClockSet) {
        Display::getInstance().disable();
    } else {
        Display::getInstance().enable();
    }

    if(FlickerCounter-- <= 0u) State = STATE_SET_TIME;
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockFlicker::setTimeTask()
{
    Display::getInstance().clear();
    Clock::getInstance().setClockFast(Hour, Minute);
    State = STATE_IDLE;
    reset();
} /* setTimeTask */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 