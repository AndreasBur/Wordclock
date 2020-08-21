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

        constexpr AnimationsType() : Cursor() { }
        ~AnimationsType() { }
    };

    using StateType = AnimationClockCommon::StateType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    std::array<byte, ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS> TaskCycles{{0}};
    AnimationType Animation{ANIMATION_CLOCK_NONE};
    AnimationsType Animations;

    // functions
    bool isAnimationValid(AnimationType sAnimation) {
        if(sAnimation < ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS) { return true; }
        else { return false; }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClock() : TaskCycles{0}, Animation(ANIMATION_CLOCK_NONE), Animations() { }
    ~AnimationClock() { }

    // get methods
    AnimationType getAnimation() const { return Animation; }
    StateType getState() const;
    byte getTaskCycle(AnimationType Animation) const { return TaskCycles[Animation]; }

    // set methods
    void setTaskCycleFast(AnimationType Animation, byte Cycle) { TaskCycles[Animation] = Cycle; }
    StdReturnType setTaskCycle(AnimationType, byte);
    void setAnimationFast(AnimationType);
    StdReturnType setAnimation(AnimationType Animation) {
        if(isAnimationValid(Animation)) {
            setAnimationFast(Animation);
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }

    // methods
    void init();
    void task();
    StdReturnType setClock(byte, byte);
    StdReturnType show();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
