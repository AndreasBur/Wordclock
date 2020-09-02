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
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.getState();
            break;
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.getState();
            break;
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.getState();
            break;
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.getState();
            break;
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.getState();
            break;
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.getState();
            break;
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.getState();
            break;
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.getState();
            break;
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.getState();
            break;
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.getState();
            break;
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.getState();
            break;
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.getState();
            break;
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
        case ANIMATION_CLOCK_NONE :
            Animation = ANIMATION_CLOCK_NONE;
            break;
        case ANIMATION_CLOCK_CURSOR :
            Animations.Cursor.init();
            break;
        case ANIMATION_CLOCK_TELETYPE :
            Animations.Teletype.init();
            break;
        case ANIMATION_CLOCK_DROP :
            Animations.Drop.init();
            break;
        case ANIMATION_CLOCK_WIPE :
            Animations.Wipe.init();
            break;
        case ANIMATION_CLOCK_SHIFT :
            Animations.Shift.init();
            break;
        case ANIMATION_CLOCK_FADE :
            Animations.Fade.init();
            break;
        case ANIMATION_CLOCK_SNAKE :
            Animations.Snake.init();
            break;
        case ANIMATION_CLOCK_CUBE :
            Animations.Cube.init();
            break;
        case ANIMATION_CLOCK_FLICKER :
            Animations.Flicker.init();
            break;
        case ANIMATION_CLOCK_SQUEEZE :
            Animations.Squeeze.init();
            break;
        case ANIMATION_CLOCK_EXPLODE :
            Animations.Explode.init();
            break;
        case ANIMATION_CLOCK_IMPLODE :
            Animations.Implode.init();
            break;
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
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.setTime(Hour, Minute);
            break;
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.setTime(Hour, Minute);
            break;
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
        case ANIMATION_CLOCK_CURSOR :
            Animations.Cursor.task();
            break;
        case ANIMATION_CLOCK_TELETYPE :
            Animations.Teletype.task();
            break;
        case ANIMATION_CLOCK_DROP :
            Animations.Drop.task();
            break;
        case ANIMATION_CLOCK_WIPE :
            Animations.Wipe.task();
            break;
        case ANIMATION_CLOCK_SHIFT :
            Animations.Shift.task();
            break;
        case ANIMATION_CLOCK_FADE :
            Animations.Fade.task();
            break;
        case ANIMATION_CLOCK_SNAKE :
            Animations.Snake.task();
            break;
        case ANIMATION_CLOCK_CUBE :
            Animations.Cube.task();
            break;
        case ANIMATION_CLOCK_FLICKER :
            Animations.Flicker.task();
            break;
        case ANIMATION_CLOCK_SQUEEZE :
            Animations.Squeeze.task();
            break;
        case ANIMATION_CLOCK_EXPLODE :
            Animations.Explode.task();
            break;
        case ANIMATION_CLOCK_IMPLODE :
            Animations.Implode.task();
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
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.show();
            break;
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.show();
            break;
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.show();
            break;
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.show();
            break;
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.show();
            break;
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.show();
            break;
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.show();
            break;
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.show();
            break;
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.show();
            break;
        case ANIMATION_CLOCK_SQUEEZE :
            return Animations.Squeeze.show();
            break;
        case ANIMATION_CLOCK_EXPLODE :
            return Animations.Explode.show();
            break;
        case ANIMATION_CLOCK_IMPLODE :
            return Animations.Implode.show();
            break;
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
