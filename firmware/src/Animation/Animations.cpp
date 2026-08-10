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

    /* A meta animation has nothing of its own to initialize. It leaves the running
       animation at none, so the first setTime() selects one right away. */
    if(isMetaAnimation(AnimationId)) { CurrentAnimationId = ANIMATION_ID_NONE; }
    else { CurrentAnimationId = AnimationId; }

    initCurrentAnimation();
} /* setAnimationFast */


/******************************************************************************************************************************************************
  initCurrentAnimation()
******************************************************************************************************************************************************/
void Animations::initCurrentAnimation()
{
    switch(CurrentAnimationId)
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
        case ANIMATION_ID_EXPLODE :
            return AnimationsRaw.Explode.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        case ANIMATION_ID_MATRIX :
            return AnimationsRaw.Matrix.init();
            break;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        case ANIMATION_ID_ROLL :
            return AnimationsRaw.Roll.init();
            break;
# endif
        default :
            CurrentAnimationId = ANIMATION_ID_NONE;
            break;
    }
} /* initCurrentAnimation */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType Animations::setTime(byte Hour, byte Minute)
{
    /* Only while nothing is running, so a minute change during a still running
       animation is ignored here exactly as it is by the animations themselves. */
    if(isMetaAnimation(AnimationId) && getStateOfCurrentAnimation() == Animation::STATE_IDLE) {
        selectAnimationOfMeta();
    }

    switch(CurrentAnimationId)
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
        case ANIMATION_ID_EXPLODE :
            return AnimationsRaw.Explode.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        case ANIMATION_ID_MATRIX :
            return AnimationsRaw.Matrix.setTime(Hour, Minute);
            break;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        case ANIMATION_ID_ROLL :
            return AnimationsRaw.Roll.setTime(Hour, Minute);
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
    switch(CurrentAnimationId)
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
        case ANIMATION_ID_EXPLODE :
            return AnimationsRaw.Explode.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        case ANIMATION_ID_MATRIX :
            return AnimationsRaw.Matrix.getState();
            break;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        case ANIMATION_ID_ROLL :
            return AnimationsRaw.Roll.getState();
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
    switch(CurrentAnimationId)
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
        case ANIMATION_ID_EXPLODE :
            AnimationsRaw.Explode.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        case ANIMATION_ID_MATRIX :
            AnimationsRaw.Matrix.task();
            break;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        case ANIMATION_ID_ROLL :
            AnimationsRaw.Roll.task();
            break;
# endif
        default :
            break;
    }
} /* taskOfCurrentAnimation */


/******************************************************************************************************************************************************
  showOfCurrentAnimation()
******************************************************************************************************************************************************/
StdReturnType Animations::showOfCurrentAnimation() const
{
    switch(CurrentAnimationId)
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
        case ANIMATION_ID_EXPLODE :
            return AnimationsRaw.Explode.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        case ANIMATION_ID_MATRIX :
            return AnimationsRaw.Matrix.show();
            break;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        case ANIMATION_ID_ROLL :
            return AnimationsRaw.Roll.show();
            break;
# endif
        default :
            return E_NOT_OK;
            break;
    }
} /* showOfCurrentAnimation */


/******************************************************************************************************************************************************
  selectAnimationOfMeta()
******************************************************************************************************************************************************/
void Animations::selectAnimationOfMeta()
{
    switch(AnimationId)
    {
# if(ANIMATIONS_SUPPORT_RANDOM == STD_ON)
        case ANIMATION_ID_RANDOM :
            CurrentAnimationId = calcRandomAnimation();
            break;
# endif
# if(ANIMATIONS_SUPPORT_SEQUENCE == STD_ON)
        case ANIMATION_ID_SEQUENCE :
            CurrentAnimationId = calcNextAnimation();
            break;
# endif
        default :
            break;
    }

    /* the selected animation shares its memory with the previous one, so it has to be
       initialized before it is used */
    initCurrentAnimation();
} /* selectAnimationOfMeta */


/******************************************************************************************************************************************************
  calcRandomAnimation()
******************************************************************************************************************************************************/
Animations::AnimationIdType Animations::calcRandomAnimation()
{
    /* Advancing state instead of deriving it from the time: the same animation would
       otherwise always run at the same minute of every day. Both halves of the state
       go into the hash, otherwise the order of animations would repeat every 256
       minutes instead of every 65536. */
    RandomState++;

    byte hash = static_cast<byte>((RandomState ^ (RandomState >> 8u)) * RandomHashFactor);
    hash ^= static_cast<byte>(hash >> 4u);
    hash = static_cast<byte>(hash * RandomHashFactor);

    /* scaled by multiplication instead of a modulo, which would favour the first
       animations because 256 is not a multiple of their number */
    const byte offset = static_cast<byte>((static_cast<uint16_t>(hash) * NumberOfRealAnimations) >> 8u);

    return static_cast<AnimationIdType>(ANIMATION_ID_NONE + 1u + offset);
} /* calcRandomAnimation */


/******************************************************************************************************************************************************
  calcNextAnimation()
******************************************************************************************************************************************************/
Animations::AnimationIdType Animations::calcNextAnimation() const
{
    /* none as the running animation means nothing ran yet, so start at the first one */
    if(CurrentAnimationId == ANIMATION_ID_NONE || CurrentAnimationId + 1u >= FirstMetaAnimation) {
        return static_cast<AnimationIdType>(ANIMATION_ID_NONE + 1u);
    }

    return static_cast<AnimationIdType>(CurrentAnimationId + 1u);
} /* calcNextAnimation */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
