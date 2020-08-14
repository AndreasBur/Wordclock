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
  Constructor of AnimationClock
******************************************************************************************************************************************************/
AnimationClock::AnimationClock()
{
    CurrentAnimation = ANIMATION_CLOCK_NONE;
} /* AnimationClock */


/******************************************************************************************************************************************************
  Destructor of AnimationClock
******************************************************************************************************************************************************/
AnimationClock::~AnimationClock()
{

} /* ~AnimationClock */


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
    switch(CurrentAnimation)
    {
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
        //case ANIMATION_CLOCK_EXPLODE :
            //return AnimationClockCommon::STATE_NONE;
            //break;
        //case ANIMATION_CLOCK_IMPLODE :
            //return AnimationClockCommon::STATE_NONE;
            //break;
        default :
            return AnimationClockCommon::STATE_NONE;
            break;
    }
} /* getState */


/******************************************************************************************************************************************************
  setAnimation()
******************************************************************************************************************************************************/
void AnimationClock::setAnimation(AnimationType Animation)
{
    CurrentAnimation = Animation;

    switch(Animation)
    {
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
        //case ANIMATION_CLOCK_EXPLODE :
            //break;
        //case ANIMATION_CLOCK_IMPLODE :
            //break;
        default :
            CurrentAnimation = ANIMATION_CLOCK_NONE;
            break;
    }
} /* setAnimation */


/******************************************************************************************************************************************************
  setClock()
******************************************************************************************************************************************************/
StdReturnType AnimationClock::setClock(byte Hour, byte Minute)
{
    switch(CurrentAnimation)
    {
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.setClock(Hour, Minute);
            break;
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.setClock(Hour, Minute);
        break;
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.setClock(Hour, Minute);
        break;
        //case ANIMATION_CLOCK_EXPLODE :
            //break;
        //case ANIMATION_CLOCK_IMPLODE :
            //break;
        default :
            break;
    }
    return E_NOT_OK;
} /* setClock */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClock::task()
{
    switch(CurrentAnimation)
    {
        case ANIMATION_CLOCK_CURSOR :
            return Animations.Cursor.task();
            break;
        case ANIMATION_CLOCK_TELETYPE :
            return Animations.Teletype.task();
            break;
        case ANIMATION_CLOCK_DROP :
            return Animations.Drop.task();
            break;
        case ANIMATION_CLOCK_WIPE :
            return Animations.Wipe.task();
            break;
        case ANIMATION_CLOCK_SHIFT :
            return Animations.Shift.task();
            break;
        case ANIMATION_CLOCK_FADE :
            return Animations.Fade.task();
            break;
        case ANIMATION_CLOCK_SNAKE :
            return Animations.Snake.task();
            break;
        case ANIMATION_CLOCK_CUBE :
            return Animations.Cube.task();
            break;
        case ANIMATION_CLOCK_FLICKER :
            return Animations.Flicker.task();
            break;
        //case ANIMATION_CLOCK_EXPLODE :
            //break;
        //case ANIMATION_CLOCK_IMPLODE :
            //break;
        default :
            break;
    }
} /* task */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
void AnimationClock::show()
{
    switch(CurrentAnimation)
    {
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
        //case ANIMATION_CLOCK_EXPLODE :
            //break;
        //case ANIMATION_CLOCK_IMPLODE :
            //break;
        default :
            break;
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 