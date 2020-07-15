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
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    AnimationClock wcAnimationClock;
    AnimationFont wcAnimationFont;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Animation();
    ~Animation();

    // get methods
    StateType getState();
	byte getFontTaskCycle() const { return wcAnimationFont.getTaskCycle(); }
	byte getClockTaskCycle(AnimationClockType AnimationClock) const { return wcAnimationClock.getTaskCycle(AnimationClock); }

    // set methods
	void setFontTaskCylce(byte Cycle) { wcAnimationFont.setTaskCycle(Cycle); }
	void setClockTaskCylce(AnimationClockType AnimationClock, byte Cycle) { wcAnimationClock.setTaskCycle(AnimationClock, Cycle); }

    // methods
    void init();
    void task();
    void show();

    // AnimationClock functions
    StdReturnType setChar(byte Column, byte Row, char Char, AnimationFont::FontType Font) { return wcAnimationFont.setChar(Column, Row, Char, Font); }
    void setCharFast(byte Column, byte Row, char Char, AnimationFont::FontType Font) { wcAnimationFont.setCharFast(Column, Row, Char, Font); }
    void setCharWithShift(char Char, AnimationFont::FontType Font) { wcAnimationFont.setCharWithShift(Char, Font); }
    void setText(const char* Text, AnimationFont::FontType Font) { wcAnimationFont.setText(Text, Font); }
    void setTextWithShift(const char* Text, AnimationFont::FontType Font) { wcAnimationFont.setTextWithShift(Text, Font); }

    // AnimationFont functions
    void setAnimation(AnimationClock::AnimationType Animation) { wcAnimationClock.setAnimation(Animation); }
    StdReturnType setClock(byte Hour, byte Minute) { return wcAnimationClock.setClock(Hour, Minute); }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
