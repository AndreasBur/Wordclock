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
/**     \file       AnimationClock.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_H_
#define _ANIMATION_CLOCK_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "AnimationClockCommon.h"
#include "AnimationClockTeletype.h"
#include "AnimationClockCursor.h"
#include "AnimationClockDrop.h"
#include "AnimationClockWipe.h"
#include "AnimationClockSnake.h"
#include "AnimationClockShift.h"
#include "AnimationClockFade.h"
#include "AnimationClockCube.h"
#include "AnimationClockFlicker.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClock configuration parameter */


/* AnimationClock parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N C L O C K
******************************************************************************************************************************************************/
class AnimationClock
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum AnimationType {
		ANIMATION_CLOCK_NONE,
        ANIMATION_CLOCK_CURSOR,
        ANIMATION_CLOCK_TELETYPE,
        ANIMATION_CLOCK_DROP,
        ANIMATION_CLOCK_SHIFT,
        ANIMATION_CLOCK_FADE,
        ANIMATION_CLOCK_SNAKE,
        ANIMATION_CLOCK_WIPE,
        ANIMATION_CLOCK_CUBE,
        ANIMATION_CLOCK_FLICKER,
        //ANIMATION_CLOCK_EXPLODE,
        //ANIMATION_CLOCK_IMPLODE,
        //ANIMATION_CLOCK_MATRIX,
		ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS
    };

    union AnimationsType {
        AnimationClockCursor Cursor;
        AnimationClockTeletype Teletype;
        AnimationClockDrop Drop;
        AnimationClockWipe Wipe;
        AnimationClockShift Shift;
        AnimationClockSnake Snake;
        AnimationClockFade Fade;
        AnimationClockCube Cube;
        AnimationClockFlicker Flicker;

        AnimationsType() {}
        ~AnimationsType() {}
    };

    using StateType = AnimationClockCommon::StateType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
  	std::array<byte, ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS> TaskCycles;
    AnimationType CurrentAnimation;
    AnimationsType Animations;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClock();
    ~AnimationClock();

	// get methods
    AnimationType getAnimation() const { return CurrentAnimation; }
    StateType getState() const;
	byte getTaskCycle(AnimationType Animation) const { return TaskCycles[Animation]; }

	// set methods
	void setTaskCycle(AnimationType Animation, byte Cycle) { TaskCycles[Animation] = Cycle; }
    void setAnimation(AnimationType);

	// methods
    void init();
    void task();
    stdReturnType setClock(byte, byte);
    void show();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
