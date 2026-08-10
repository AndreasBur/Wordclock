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
#include "AnimationMatrix.h"
#include "AnimationRoll.h"

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
#define ANIMATIONS_SUPPORT_MATRIX               STD_ON
#define ANIMATIONS_SUPPORT_ROLL                 STD_ON

/* These two do not animate themselves, they pick one of the animations above per
   minute. They have to stay the last ids, see FirstMetaAnimation. */
#define ANIMATIONS_SUPPORT_RANDOM               STD_ON
#define ANIMATIONS_SUPPORT_SEQUENCE             STD_ON

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
        ANIMATION_ID_EXPLODE,
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        ANIMATION_ID_MATRIX,
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        ANIMATION_ID_ROLL,
# endif
        /* meta animations, they select one of the ids above. Everything below the
           first of them is a real animation, which is how they know what they may
           pick, so they have to stay at the end of this list. */
# if(ANIMATIONS_SUPPORT_RANDOM == STD_ON)
        ANIMATION_ID_RANDOM,
# endif
# if(ANIMATIONS_SUPPORT_SEQUENCE == STD_ON)
        ANIMATION_ID_SEQUENCE,
# endif
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
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        AnimationMatrix Matrix;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        AnimationRoll Roll;
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
# elif(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        Matrix()
# elif(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        Roll()
# endif
{ }
        ~AnimationsType() { }
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycleInitValue{ANIMATIONS_TASK_CYCLE_INIT_VALUE};

    /* first id that selects instead of animates, everything from ANIMATION_ID_NONE + 1
       up to it is a real animation that the meta animations may pick */
# if(ANIMATIONS_SUPPORT_RANDOM == STD_ON)
    static constexpr AnimationIdType FirstMetaAnimation{ANIMATION_ID_RANDOM};
# elif(ANIMATIONS_SUPPORT_SEQUENCE == STD_ON)
    static constexpr AnimationIdType FirstMetaAnimation{ANIMATION_ID_SEQUENCE};
# else
    static constexpr AnimationIdType FirstMetaAnimation{ANIMATION_ID_NUMBER_OF_ANIMATIONS};
# endif
    static constexpr byte NumberOfRealAnimations{FirstMetaAnimation - ANIMATION_ID_NONE - 1u};
    /* odd factor of the hash in calcRandomAnimation(), see there */
    static constexpr byte RandomHashFactor{181u};

    std::array<byte, ANIMATION_ID_NUMBER_OF_ANIMATIONS> TaskCycles;
    /* what the user selected, may be a meta animation */
    AnimationIdType AnimationId{ANIMATION_ID_NONE};
    /* what is really running, never a meta animation. Equal to AnimationId unless a
       meta animation selected something. */
    AnimationIdType CurrentAnimationId{ANIMATION_ID_NONE};
    uint16_t RandomState{0u};
    AnimationsType AnimationsRaw;

    // functions
    Animations() : AnimationsRaw() { TaskCycles.fill(TaskCycleInitValue); }
    ~Animations() { }

    Animation::StateType getStateOfCurrentAnimation() const;
    void taskOfCurrentAnimation();
    StdReturnType showOfCurrentAnimation() const;
    void initCurrentAnimation();
    void selectAnimationOfMeta();
    AnimationIdType calcRandomAnimation();
    AnimationIdType calcNextAnimation() const;

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
    AnimationIdType getCurrentAnimation() const { return CurrentAnimationId; }
    bool isMetaAnimation(AnimationIdType sAnimationId) const { return sAnimationId >= FirstMetaAnimation; }

    byte getTaskCycle(AnimationIdType sAnimationId) const { return TaskCycles[sAnimationId]; }
    /* the cycle of the running animation, so every animation keeps its own speed while
       a meta animation switches between them */
    byte getTaskCycle() const { return TaskCycles[CurrentAnimationId]; }

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
