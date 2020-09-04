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
/**     \file       AnimationClock.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationClock.h"


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
void AnimationClock::init()
{

} /* init */


/******************************************************************************************************************************************************
  getState()
******************************************************************************************************************************************************/
AnimationClock::StateType AnimationClock::getState() const
{
    switch(Animation)
    {
        case ANIMATION_CLOCK_NONE :
            return AnimationClockCommon::STATE_IDLE;
            break;
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.getState();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.getState();
            break;
# endif
        default :
            return AnimationClockCommon::STATE_NONE;
            break;
    }
} /* getState */

/******************************************************************************************************************************************************
  setTaskCycle()
******************************************************************************************************************************************************/
StdReturnType AnimationClock::setTaskCycle(AnimationType Animation, byte Cycle)
{
    if(isAnimationValid(Animation)) {
        setTaskCycleFast(Animation, Cycle);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

/******************************************************************************************************************************************************
  setAnimation()
******************************************************************************************************************************************************/
void AnimationClock::setAnimationFast(AnimationType sAnimation)
{
    Animation = sAnimation;

    switch(Animation)
    {
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.init();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.init();
            break;
# endif
        default :
            Animation = ANIMATION_CLOCK_NONE;
            break;
    }
} /* setAnimationFast */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationClock::setTime(byte Hour, byte Minute)
{
    switch(Animation)
    {
        case ANIMATION_CLOCK_NONE :
            return Clock::getInstance().setTime(Hour, Minute);
            break;
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.setTime(Hour, Minute);
            break;
# endif
        default :
            return E_NOT_OK;
            break;
    }
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClock::task()
{
    switch(Animation)
    {
        case ANIMATION_CLOCK_NONE :
            break;
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_CLOCK_CURSOR :
            Animations.Cursor.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_CLOCK_TELETYPE :
            Animations.Teletype.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        case ANIMATION_CLOCK_DROP :
            Animations.Drop.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_CLOCK_SHIFT :
            Animations.Shift.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        case ANIMATION_CLOCK_FADE :
            Animations.Fade.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_CLOCK_SNAKE :
            Animations.Snake.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        case ANIMATION_CLOCK_WIPE :
            Animations.Wipe.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        case ANIMATION_CLOCK_CUBE :
            Animations.Cube.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_CLOCK_FLICKER :
            Animations.Flicker.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_CLOCK_SQUEEZE :
            Animations.Squeeze.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_CLOCK_IMPLODE :
            Animations.Implode.task();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_CLOCK_EXPLODE :
            Animations.Explode.task();
# endif
            break;
        default :
            break;
    }
} /* task */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
StdReturnType AnimationClock::show() const
{
    switch(Animation)
    {
        case ANIMATION_CLOCK_NONE :
            return Display::getInstance().show();
            break;
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.show();
            break;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.show();
            break;
# endif
        default :
            return E_NOT_OK;
            break;
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
