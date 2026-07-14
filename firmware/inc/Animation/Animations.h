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
/**     \file       Animations.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Animation.h"
#include "AnimationTeletype.h"
#include "AnimationCursor.h"
#include "AnimationDrop.h"
#include "AnimationWipe.h"
#include "AnimationSnake.h"
#include "AnimationShift.h"
#include "AnimationFade.h"
#include "AnimationCube.h"
#include "AnimationFlicker.h"
#include "AnimationSqueeze.h"
#include "AnimationImplode.h"
#include "AnimationExplode.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Animations configuration parameter */
#define ANIMATIONS_SUPPORT_TELETYPE             STD_ON
#define ANIMATIONS_SUPPORT_CURSOR               STD_ON
#define ANIMATIONS_SUPPORT_DROP                 STD_ON
#define ANIMATIONS_SUPPORT_WIPE                 STD_ON
#define ANIMATIONS_SUPPORT_SNAKE                STD_ON
#define ANIMATIONS_SUPPORT_SHIFT                STD_ON
#define ANIMATIONS_SUPPORT_FADE                 STD_ON
#define ANIMATIONS_SUPPORT_CUBE                 STD_ON
#define ANIMATIONS_SUPPORT_FLICKER              STD_ON
#define ANIMATIONS_SUPPORT_SQUEEZE              STD_ON
#define ANIMATIONS_SUPPORT_IMPLODE              STD_ON
#define ANIMATIONS_SUPPORT_EXPLODE              STD_ON

#define ANIMATIONS_TASK_CYCLE_INIT_VALUE        10u

/* Animations parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N S
******************************************************************************************************************************************************/
class Animations
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_NONE,
        STATE_IDLE,
        STATE_PENDING
    };
  
    enum AnimationIdType {
        ANIMATION_ID_NONE,
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        ANIMATION_ID_CURSOR,
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        ANIMATION_ID_TELETYPE,
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        ANIMATION_ID_DROP,
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        ANIMATION_ID_SHIFT,
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        ANIMATION_ID_FADE,
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        ANIMATION_ID_SNAKE,
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        ANIMATION_ID_WIPE,
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        ANIMATION_ID_CUBE,
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        ANIMATION_ID_FLICKER,
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        ANIMATION_ID_SQUEEZE,
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        ANIMATION_ID_IMPLODE,
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        ANIMATION_EXPLODE,
# endif
        //ANIMATION_MATRIX,
        ANIMATION_ID_NUMBER_OF_ANIMATIONS
    };

    union AnimationsType {
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        AnimationCursor Cursor;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        AnimationTeletype Teletype;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        AnimationDrop Drop;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        AnimationShift Shift;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        AnimationWipe Wipe;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        AnimationSnake Snake;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        AnimationFade Fade;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        AnimationCube Cube;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        AnimationFlicker Flicker;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        AnimationSqueeze Squeeze;
# endif
# if(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        AnimationImplode Implode;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        AnimationExplode Explode;
# endif
        constexpr AnimationsType() :
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        Cursor()
# elif(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        Teletype()
# elif(ANIMATIONS_SUPPORT_DROP == STD_ON)
        Drop()
# elif(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        Shift()
# elif(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        Wipe()
# elif(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        Snake()
# elif(ANIMATIONS_SUPPORT_FADE == STD_ON)
        Fade()
# elif(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        Cube()
# elif(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        Flicker()
# elif(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        Squeeze()
# elif(ANIMATIONS_SUPPORT_IMPLODE == STD_ON)
        Implode()
# elif(ANIMATIONS_SUPPORT_EXPLODE == STD_ON)
        Explode()
# endif
{ }
        ~AnimationsType() { }
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycleInitValue{ANIMATIONS_TASK_CYCLE_INIT_VALUE};
    std::array<byte, ANIMATION_ID_NUMBER_OF_ANIMATIONS> TaskCycles;
    AnimationIdType AnimationId{ANIMATION_ID_NONE};
    AnimationsType AnimationsRaw;

    // functions
    Animations() : AnimationsRaw() { TaskCycles.fill(TaskCycleInitValue); }
    ~Animations() { }

    Animation::StateType getStateOfCurrentAnimation() const;
    void taskOfCurrentAnimation();
    StdReturnType showOfCurrentAnimation() const;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Animations& getInstance() {
        static Animations singletonInstance;
        return singletonInstance;
    }

    // get methods
    StateType getState() const;
    AnimationIdType getAnimation() const { return AnimationId; }

    byte getTaskCycle(AnimationIdType sAnimationId) const { return TaskCycles[sAnimationId]; }
    byte getTaskCycle() const { return TaskCycles[AnimationId]; }

    // set methods
    void setTaskCycleFast(AnimationIdType AnimationId, byte Cycle) { TaskCycles[AnimationId] = Cycle; }
    StdReturnType setTaskCycle(AnimationIdType, byte);
    void setAnimationFast(AnimationIdType);
    StdReturnType setAnimation(AnimationIdType AnimationId) {
        if(isAnimationValid(AnimationId)) {
            setAnimationFast(AnimationId);
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }

    // methods
    void task(bool=false);
    StdReturnType show() const;
    bool isAnimationValid(AnimationIdType AnimationId) const { return AnimationId < ANIMATION_ID_NUMBER_OF_ANIMATIONS; }
    StdReturnType setTime(byte, byte);

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
