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
/**     \file       Animation.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "AnimationClock.h"
#include "AnimationFont.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Animation configuration parameter*/


/* Animation parameter */


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S  A N I M A T I O N
******************************************************************************************************************************************************/
class Animation
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_NONE,
        STATE_IDLE,
        STATE_CLOCK,
        STATE_FONT
    };

    using AnimationClockType = AnimationClock::AnimationType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    constexpr Animation() : wcAnimationClock(), wcAnimationFont() { }
    ~Animation() { }

    AnimationClock wcAnimationClock;
    AnimationFont wcAnimationFont;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Animation& getInstance() {
        static Animation singletonInstance;
        return singletonInstance;
    }

    // get methods
    StateType getState();
    AnimationClockType getAnimation() const { return wcAnimationClock.getAnimation(); }
    byte getFontTaskCycle() const { return wcAnimationFont.getTaskCycle(); }
    byte getClockTaskCycle(AnimationClockType AnimationClock) const { return wcAnimationClock.getTaskCycle(AnimationClock); }

    // set methods
    void setFontTaskCylce(byte Cycle) { wcAnimationFont.setTaskCycle(Cycle); }
    StdReturnType setClockTaskCycle(AnimationClockType AnimationClock, byte Cycle) { return wcAnimationClock.setTaskCycle(AnimationClock, Cycle); }
    void setClockTaskCycleFast(AnimationClockType AnimationClock, byte Cycle) { wcAnimationClock.setTaskCycleFast(AnimationClock, Cycle); }

    // methods
    void init();
    void task();
    StdReturnType show() const;

    // AnimationClock functions
    StdReturnType setChar(byte Column, byte Row, char Char, AnimationFont::FontType Font) { return wcAnimationFont.setChar(Column, Row, Char, Font); }
    void setCharFast(byte Column, byte Row, char Char, AnimationFont::FontType Font) { wcAnimationFont.setCharFast(Column, Row, Char, Font); }
    void setCharWithShift(char Char, AnimationFont::FontType Font) { wcAnimationFont.setCharWithShift(Char, Font); }
    void setText(const char* Text, AnimationFont::FontType Font) { wcAnimationFont.setText(Text, Font); }
    void setTextWithShift(const char* Text, AnimationFont::FontType Font) { wcAnimationFont.setTextWithShift(Text, Font); }

    // AnimationFont functions
    StdReturnType setAnimation(AnimationClockType Animation) { return wcAnimationClock.setAnimation(Animation); }
    void setAnimationFast(AnimationClockType Animation) { wcAnimationClock.setAnimationFast(Animation); }
    StdReturnType setTime(byte Hour, byte Minute) { return wcAnimationClock.setTime(Hour, Minute); }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
