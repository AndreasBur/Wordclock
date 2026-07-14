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
/**     \file       Animations.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATIONS_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Animations.h"


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
  getState()
******************************************************************************************************************************************************/
Animations::StateType Animations::getState() const
{
    if(getStateOfCurrentAnimation() == Animation::STATE_CLEAR_TIME ||
       getStateOfCurrentAnimation() == Animation::STATE_SET_TIME)
    {
        return STATE_PENDING;
    }

    return STATE_IDLE;
} /* getState */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void Animations::task(bool Show)
{
    if(getStateOfCurrentAnimation() == Animation::STATE_CLEAR_TIME ||
       getStateOfCurrentAnimation() == Animation::STATE_SET_TIME)
    {
        taskOfCurrentAnimation();
        if(Show) { showOfCurrentAnimation(); }
    }
} /* task */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
StdReturnType Animations::show() const
{
    if(getStateOfCurrentAnimation() == Animation::STATE_CLEAR_TIME ||
       getStateOfCurrentAnimation() == Animation::STATE_SET_TIME)
    {
        return showOfCurrentAnimation();
    }

    return E_NOT_OK;
} /* show */

/******************************************************************************************************************************************************
  setTaskCycle()
******************************************************************************************************************************************************/
StdReturnType Animations::setTaskCycle(AnimationIdType AnimationId, byte Cycle)
{
    if(isAnimationValid(AnimationId)) {
        setTaskCycleFast(AnimationId, Cycle);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

/******************************************************************************************************************************************************
  setAnimation()
******************************************************************************************************************************************************/
void Animations::setAnimationFast(AnimationIdType sAnimationId)
{
    AnimationId = sAnimationId;

    switch(AnimationId)
    {
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_ID_CURSOR :
            return AnimationsRaw.Cursor.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_ID_TELETYPE :
            return AnimationsRaw.Teletype.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        case ANIMATION_ID_DROP :
            return AnimationsRaw.Drop.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_ID_SHIFT :
            return AnimationsRaw.Shift.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        case ANIMATION_ID_FADE :
            return AnimationsRaw.Fade.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_ID_SNAKE :
            return AnimationsRaw.Snake.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        case ANIMATION_ID_WIPE :
            return AnimationsRaw.Wipe.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        case ANIMATION_ID_CUBE :
            return AnimationsRaw.Cube.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_ID_FLICKER :
            return AnimationsRaw.Flicker.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_ID_SQUEEZE :
            return AnimationsRaw.Squeeze.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_ID_IMPLODE :
            return AnimationsRaw.Implode.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_EXPLODE :
            return AnimationsRaw.Explode.init();
            break;
# endif
        default :
            AnimationId = ANIMATION_ID_NONE;
            break;
    }
} /* setAnimationFast */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType Animations::setTime(byte Hour, byte Minute)
{
    switch(AnimationId)
    {
        case ANIMATION_ID_NONE :
            return Clock::getInstance().setTime(Hour, Minute);
            break;
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_ID_CURSOR :
            return AnimationsRaw.Cursor.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_ID_TELETYPE :
            return AnimationsRaw.Teletype.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        case ANIMATION_ID_DROP :
            return AnimationsRaw.Drop.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_ID_SHIFT :
            return AnimationsRaw.Shift.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        case ANIMATION_ID_FADE :
            return AnimationsRaw.Fade.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_ID_SNAKE :
            return AnimationsRaw.Snake.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        case ANIMATION_ID_WIPE :
            return AnimationsRaw.Wipe.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        case ANIMATION_ID_CUBE :
            return AnimationsRaw.Cube.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_ID_FLICKER :
            return AnimationsRaw.Flicker.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_ID_SQUEEZE :
            return AnimationsRaw.Squeeze.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_ID_IMPLODE :
            return AnimationsRaw.Implode.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_EXPLODE :
            return AnimationsRaw.Explode.setTime(Hour, Minute);
            break;
# endif
        default :
            return E_NOT_OK;
            break;
    }
} /* setTime */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getStateOfCurrentAnimation()
******************************************************************************************************************************************************/
Animation::StateType Animations::getStateOfCurrentAnimation() const
{
    switch(AnimationId)
    {
        case ANIMATION_ID_NONE :
            return Animation::STATE_IDLE;
            break;
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_ID_CURSOR :
            return AnimationsRaw.Cursor.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_ID_TELETYPE :
            return AnimationsRaw.Teletype.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        case ANIMATION_ID_DROP :
            return AnimationsRaw.Drop.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_ID_SHIFT :
            return AnimationsRaw.Shift.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        case ANIMATION_ID_FADE :
            return AnimationsRaw.Fade.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_ID_SNAKE :
            return AnimationsRaw.Snake.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        case ANIMATION_ID_WIPE :
            return AnimationsRaw.Wipe.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        case ANIMATION_ID_CUBE :
            return AnimationsRaw.Cube.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_ID_FLICKER :
            return AnimationsRaw.Flicker.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_ID_SQUEEZE :
            return AnimationsRaw.Squeeze.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_ID_IMPLODE :
            return AnimationsRaw.Implode.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_EXPLODE :
            return AnimationsRaw.Explode.getState();
            break;
# endif
        default :
            return Animation::STATE_NONE;
            break;
    }
} /* getStateOfCurrentAnimation */


/******************************************************************************************************************************************************
  taskOfCurrentAnimation()
******************************************************************************************************************************************************/
void Animations::taskOfCurrentAnimation()
{
    switch(AnimationId)
    {
        case ANIMATION_ID_NONE :
            break;
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_ID_CURSOR :
            AnimationsRaw.Cursor.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_ID_TELETYPE :
            AnimationsRaw.Teletype.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        case ANIMATION_ID_DROP :
            AnimationsRaw.Drop.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_ID_SHIFT :
            AnimationsRaw.Shift.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        case ANIMATION_ID_FADE :
            AnimationsRaw.Fade.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_ID_SNAKE :
            AnimationsRaw.Snake.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        case ANIMATION_ID_WIPE :
            AnimationsRaw.Wipe.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        case ANIMATION_ID_CUBE :
            AnimationsRaw.Cube.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_ID_FLICKER :
            AnimationsRaw.Flicker.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_ID_SQUEEZE :
            AnimationsRaw.Squeeze.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_ID_IMPLODE :
            AnimationsRaw.Implode.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_EXPLODE :
            AnimationsRaw.Explode.task();
# endif
            break;
        default :
            break;
    }
} /* taskOfCurrentAnimation */


/******************************************************************************************************************************************************
  showOfCurrentAnimation()
******************************************************************************************************************************************************/
StdReturnType Animations::showOfCurrentAnimation() const
{
    switch(AnimationId)
    {
        case ANIMATION_ID_NONE :
            return Display::getInstance().show();
            break;
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        case ANIMATION_ID_CURSOR :
            return AnimationsRaw.Cursor.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        case ANIMATION_ID_TELETYPE :
            return AnimationsRaw.Teletype.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        case ANIMATION_ID_DROP :
            return AnimationsRaw.Drop.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        case ANIMATION_ID_SHIFT :
            return AnimationsRaw.Shift.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        case ANIMATION_ID_FADE :
            return AnimationsRaw.Fade.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        case ANIMATION_ID_SNAKE :
            return AnimationsRaw.Snake.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        case ANIMATION_ID_WIPE :
            return AnimationsRaw.Wipe.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        case ANIMATION_ID_CUBE :
            return AnimationsRaw.Cube.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        case ANIMATION_ID_FLICKER :
            return AnimationsRaw.Flicker.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        case ANIMATION_ID_SQUEEZE :
            return AnimationsRaw.Squeeze.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        case ANIMATION_ID_IMPLODE :
            return AnimationsRaw.Implode.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        case ANIMATION_EXPLODE :
            return AnimationsRaw.Explode.show();
            break;
# endif
        default :
            return E_NOT_OK;
            break;
    }
} /* showOfCurrentAnimation */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
