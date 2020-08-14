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
/**     \file       Animation.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "Animation.h"


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
  Singleton Instance of Animation
******************************************************************************************************************************************************/
Animation& Animation::getInstance()
{
    static Animation SingletonInstance;
    return SingletonInstance;
}

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
void Animation::init()
{
    wcAnimationClock.init();
    wcAnimationFont.init();
} /* init */


/******************************************************************************************************************************************************
  getState()
******************************************************************************************************************************************************/
Animation::StateType Animation::getState()
{
    if( wcAnimationClock.getState() == AnimationClockCommon::STATE_CLEAR_TIME ||
        wcAnimationClock.getState() == AnimationClockCommon::STATE_SET_TIME)
    {
        return STATE_CLOCK;
    }
    else if( wcAnimationFont.getState() == AnimationFont::STATE_CHAR_SHIFT ||
             wcAnimationFont.getState() == AnimationFont::STATE_TEXT_SHIFT)
    {
        return STATE_FONT;
    }

    return STATE_IDLE;
} /* getState */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void Animation::task()
{
    if( wcAnimationClock.getState() == AnimationClockCommon::STATE_CLEAR_TIME ||
        wcAnimationClock.getState() == AnimationClockCommon::STATE_SET_TIME)
    {
        wcAnimationClock.task();
    }
    else if( wcAnimationFont.getState() == AnimationFont::STATE_CHAR_SHIFT ||
             wcAnimationFont.getState() == AnimationFont::STATE_TEXT_SHIFT)
    {
        wcAnimationFont.task();
    }
} /* task */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
void Animation::show()
{
    if( wcAnimationClock.getState() == AnimationClockCommon::STATE_CLEAR_TIME ||
        wcAnimationClock.getState() == AnimationClockCommon::STATE_SET_TIME)
    {
        wcAnimationClock.show();
    }
    else if( wcAnimationFont.getState() == AnimationFont::STATE_CHAR_SHIFT ||
             wcAnimationFont.getState() == AnimationFont::STATE_TEXT_SHIFT)
    {
        wcAnimationFont.show();
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of Animation
******************************************************************************************************************************************************/
Animation::Animation()
{

} /* Animation */


/******************************************************************************************************************************************************
  Destructor of Animation
******************************************************************************************************************************************************/
Animation::~Animation()
{

} /* ~Animation */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
