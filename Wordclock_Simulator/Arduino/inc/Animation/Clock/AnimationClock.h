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
#include "AnimationClockSqueeze.h"
#include "AnimationClockImplode.h"
#include "AnimationClockExplode.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClock configuration parameter */
#define ANIMATION_CLOCK_SUPPORT_TELETYPE             STD_ON
#define ANIMATION_CLOCK_SUPPORT_CURSOR               STD_ON
#define ANIMATION_CLOCK_SUPPORT_DROP                 STD_ON
#define ANIMATION_CLOCK_SUPPORT_WIPE                 STD_ON
#define ANIMATION_CLOCK_SUPPORT_SNAKE                STD_ON
#define ANIMATION_CLOCK_SUPPORT_SHIFT                STD_ON
#define ANIMATION_CLOCK_SUPPORT_FADE                 STD_ON
#define ANIMATION_CLOCK_SUPPORT_CUBE                 STD_ON
#define ANIMATION_CLOCK_SUPPORT_FLICKER              STD_ON
#define ANIMATION_CLOCK_SUPPORT_SQUEEZE              STD_ON
#define ANIMATION_CLOCK_SUPPORT_IMPLODE              STD_ON
#define ANIMATION_CLOCK_SUPPORT_EXPLODE              STD_ON

/* AnimationClock parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   C L O C K
******************************************************************************************************************************************************/
class AnimationClock
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum AnimationType {
        ANIMATION_CLOCK_NONE,
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        ANIMATION_CLOCK_CURSOR,
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        ANIMATION_CLOCK_TELETYPE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        ANIMATION_CLOCK_DROP,
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        ANIMATION_CLOCK_SHIFT,
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        ANIMATION_CLOCK_FADE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        ANIMATION_CLOCK_SNAKE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        ANIMATION_CLOCK_WIPE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        ANIMATION_CLOCK_CUBE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        ANIMATION_CLOCK_FLICKER,
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        ANIMATION_CLOCK_SQUEEZE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        ANIMATION_CLOCK_IMPLODE,
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        ANIMATION_CLOCK_EXPLODE,
# endif
        //ANIMATION_CLOCK_MATRIX,
        ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS
    };

    union AnimationsType {
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        AnimationClockCursor Cursor;
# endif
# if(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        AnimationClockTeletype Teletype;
# endif
# if(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        AnimationClockDrop Drop;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        AnimationClockShift Shift;
# endif
# if(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        AnimationClockWipe Wipe;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        AnimationClockSnake Snake;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        AnimationClockFade Fade;
# endif
# if(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        AnimationClockCube Cube;
# endif
# if(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        AnimationClockFlicker Flicker;
# endif
# if(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        AnimationClockSqueeze Squeeze;
# endif
# if(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        AnimationClockImplode Implode;
# endif
# if(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        AnimationClockExplode Explode;
# endif
        constexpr AnimationsType() :
# if(ANIMATION_CLOCK_SUPPORT_CURSOR == STD_ON)
        Cursor()
# elif(ANIMATION_CLOCK_SUPPORT_TELETYPE == STD_ON)
        Teletype()
# elif(ANIMATION_CLOCK_SUPPORT_DROP == STD_ON)
        Drop()
# elif(ANIMATION_CLOCK_SUPPORT_SHIFT == STD_ON)
        Shift()
# elif(ANIMATION_CLOCK_SUPPORT_WIPE == STD_ON)
        Wipe()
# elif(ANIMATION_CLOCK_SUPPORT_SNAKE == STD_ON)
        Snake()
# elif(ANIMATION_CLOCK_SUPPORT_FADE == STD_ON)
        Fade()
# elif(ANIMATION_CLOCK_SUPPORT_CUBE == STD_ON)
        Cube()
# elif(ANIMATION_CLOCK_SUPPORT_FLICKER == STD_ON)
        Flicker()
# elif(ANIMATION_CLOCK_SUPPORT_SQUEEZE == STD_ON)
        Squeeze()
# elif(ANIMATION_CLOCK_SUPPORT_IMPLODE == STD_ON)
        Implode()
# elif(ANIMATION_CLOCK_SUPPORT_EXPLODE == STD_ON)
        Explode()
# endif
{ }
        ~AnimationsType() { }
    };

    using StateType = AnimationClockCommon::StateType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    std::array<byte, ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS> TaskCycles{{0u}};
    AnimationType Animation{ANIMATION_CLOCK_NONE};
    AnimationsType Animations;

    // functions

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClock() : TaskCycles{255u}, Animation(ANIMATION_CLOCK_NONE), Animations() { }
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
    bool isAnimationValid(AnimationType sAnimation) const { return sAnimation < ANIMATION_CLOCK_NUMBER_OF_ANIMATIONS; }
    StdReturnType setTime(byte, byte);
    StdReturnType show() const;
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
