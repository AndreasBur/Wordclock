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
#include "AnimationExplodePixels.h"
#include "AnimationExplodeWords.h"
#include "AnimationMatrix.h"
#include "AnimationRoll.h"
#include "AnimationCollapse.h"

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
#define ANIMATIONS_SUPPORT_EXPLODE_PIXELS       STD_ON
#define ANIMATIONS_SUPPORT_EXPLODE_WORDS        STD_ON
#define ANIMATIONS_SUPPORT_MATRIX               STD_ON
#define ANIMATIONS_SUPPORT_ROLL                 STD_ON
#define ANIMATIONS_SUPPORT_COLLAPSE             STD_ON

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
        kStateNone,
        kStateIdle,
        kStatePending
    };
  
    enum AnimationIdType {
        kAnimationIdNone,
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        kAnimationIdCursor,
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        kAnimationIdTeletype,
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        kAnimationIdDrop,
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        kAnimationIdShift,
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        kAnimationIdFade,
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        kAnimationIdSnake,
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        kAnimationIdWipe,
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        kAnimationIdCube,
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        kAnimationIdFlicker,
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        kAnimationIdSqueeze,
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
        kAnimationIdExplodePixels,
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
        kAnimationIdExplodeWords,
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        kAnimationIdMatrix,
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        kAnimationIdRoll,
# endif
# if(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
        kAnimationIdCollapse,
# endif
        kAnimationIdNumberOfAnimations
    };

    /* Which animation runs on a word change. Only kModeFixed uses the selected
       animation, the others pick one themselves, which is why selecting an animation
       and selecting a strategy are two separate settings. */
    enum ModeType {
        kModeFixed,
        kModeRandom,
        kModeSequence,
        kModeNumberOfModes
    };

    union AnimationsType {
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        AnimationCursor cursor;
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        AnimationTeletype teletype;
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        AnimationDrop drop;
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        AnimationShift shift;
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        AnimationWipe wipe;
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        AnimationSnake snake;
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        AnimationFade fade;
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        AnimationCube cube;
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        AnimationFlicker flicker;
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        AnimationSqueeze squeeze;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
        AnimationExplodePixels explode_pixels;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
        AnimationExplodeWords explode_words;
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        AnimationMatrix matrix;
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        AnimationRoll roll;
# endif
# if(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
        AnimationCollapse collapse;
# endif
        constexpr AnimationsType() :
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        cursor()
# elif(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        teletype()
# elif(ANIMATIONS_SUPPORT_DROP == STD_ON)
        drop()
# elif(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        shift()
# elif(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        wipe()
# elif(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        snake()
# elif(ANIMATIONS_SUPPORT_FADE == STD_ON)
        fade()
# elif(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        cube()
# elif(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        flicker()
# elif(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        squeeze()
# elif(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
        explode_pixels()
# elif(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
        explode_words()
# elif(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        matrix()
# elif(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        roll()
# elif(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
        collapse()
# endif
{ }
        ~AnimationsType() { }
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte kTaskCycleInitValue{ANIMATIONS_TASK_CYCLE_INIT_VALUE};

    /* kAnimationIdNone means no animation at all, so the ids a mode may pick start
       behind it */
    static constexpr byte kFirstAnimation{kAnimationIdNone + 1u};
    static constexpr byte kNumberOfAnimations{kAnimationIdNumberOfAnimations - kFirstAnimation};
    /* odd factor of the hash in CalcRandomAnimation(), see there */
    static constexpr byte kRandomHashFactor{181u};

    static_assert(kAnimationIdNumberOfAnimations <= 16u,
                  "Animations: too many animations for the favourites bit mask, please widen it");

    /* one bit per animation id, bit 0 of kAnimationIdNone stays unused.
       Shifted in an unsigned long rather than an unsigned int: there are sixteen
       animations, and on a target whose int is 16 bits wide - every AVR - 1u shifted by
       sixteen is undefined and the mask comes out empty, which silently leaves no
       animation a favourite. */
    static constexpr uint16_t kAllFavourites{
        static_cast<uint16_t>(((1uL << kAnimationIdNumberOfAnimations) - 1uL) & ~((1uL << kFirstAnimation) - 1uL))
    };

    std::array<byte, kAnimationIdNumberOfAnimations> task_cycles_;
    /* what the user selected, used as it is in kModeFixed */
    AnimationIdType animation_id_{kAnimationIdNone};
    /* what is really running, equal to animation_id_ unless a mode selected something */
    AnimationIdType current_animation_id_{kAnimationIdNone};
    ModeType mode_{kModeFixed};
    /* animations the selecting modes may pick, all of them until told otherwise */
    uint16_t favourites_{kAllFavourites};
    uint16_t random_state_{0u};
    AnimationsType animations_raw_;

    // functions
    Animations() : animations_raw_() { task_cycles_.fill(kTaskCycleInitValue); }
    ~Animations() { }

    Animation::StateType GetStateOfCurrentAnimation() const;
    void TaskOfCurrentAnimation();
    StdReturnType ShowOfCurrentAnimation() const;
    void InitCurrentAnimation();
    void SelectAnimationOfMode();
    AnimationIdType CalcRandomAnimation();
    AnimationIdType CalcNextAnimation() const;
    byte NumberOfFavourites() const;

    /* The round a knob walks, which is not the one CalcNextAnimation() walks: that one
       skips everything but the favourites, because kModeSequence picks among those.
       Stepping the selection by hand may reach every animation, favourite or not. */
    static constexpr AnimationIdType ToNextAnimation(AnimationIdType animation_id) {
        return (animation_id + 1u >= kAnimationIdNumberOfAnimations)
             ? kAnimationIdNone : static_cast<AnimationIdType>(animation_id + 1u);
    }
    static constexpr AnimationIdType ToPreviousAnimation(AnimationIdType animation_id) {
        return (animation_id == kAnimationIdNone)
             ? static_cast<AnimationIdType>(kAnimationIdNumberOfAnimations - 1u)
             : static_cast<AnimationIdType>(animation_id - 1u);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Animations& GetInstance() {
        static Animations singleton_instance;
        return singleton_instance;
    }

    // get methods
    StateType GetState() const;
    AnimationIdType GetAnimation() const { return animation_id_; }
    AnimationIdType GetCurrentAnimation() const { return current_animation_id_; }
    ModeType GetMode() const { return mode_; }
    bool IsFavourite(AnimationIdType animation_id) const {
        return (favourites_ & static_cast<uint16_t>(1u << animation_id)) != 0u;
    }

    byte GetTaskCycle(AnimationIdType animation_id) const { return task_cycles_[animation_id]; }
    /* the cycle of the running animation, so every animation keeps its own speed while
       a meta animation switches between them */
    byte GetTaskCycle() const { return task_cycles_[current_animation_id_]; }

    // set methods
    StdReturnType SetTaskCycle(AnimationIdType, byte);
    StdReturnType SetAnimation(AnimationIdType);
    StdReturnType SetMode(ModeType);
    StdReturnType SetFavourite(AnimationIdType animation_id, bool favourite) {
        /* kAnimationIdNone is no animation and cannot be a favourite */
        if(animation_id < kFirstAnimation || !IsAnimationValid(animation_id)) { return kENotOk; }
        /* the last favourite may not be dropped, the selecting modes would be left
           with nothing to pick */
        if(!favourite && IsFavourite(animation_id) && NumberOfFavourites() <= 1u) { return kENotOk; }

        if(favourite) { favourites_ |= static_cast<uint16_t>(1u << animation_id); }
        else { favourites_ &= static_cast<uint16_t>(~(1u << animation_id)); }
        return kEOk;
    }

    // methods
    /* The same state the constructor leaves behind: every speed at its initial cycle,
       every animation a favourite again, and nothing selected. Written out here rather
       than by constructing a second instance, which a singleton cannot do. */
    void ResetToDefaults() {
        task_cycles_.fill(kTaskCycleInitValue);
        favourites_ = kAllFavourites;
        /* Both are valid by construction, so neither answer can say anything a caller of
           ResetToDefaults() could act on. */
        SetAnimation(kAnimationIdNone);
        SetMode(kModeFixed);
    }

    /* One animation further and one back, wrapping at both ends, for a control that has
       no list to pick from. kAnimationIdNone is part of that round rather than skipped:
       "no animation" is a setting like any other, and a knob is the only way somebody
       without a phone gets back to it.

       They step the selection, so they mean what "9 -A<id>" means and not what
       kRpcIdAnimationStart means - the animation runs at the next word change, and in
       a mode that picks its own the selection is what kModeFixed would return to. */
    void NextAnimation() { SetAnimation(ToNextAnimation(animation_id_)); }
    void PreviousAnimation() { SetAnimation(ToPreviousAnimation(animation_id_)); }

    void Task(bool=false);
    StdReturnType Show() const;
    static bool IsAnimationValid(AnimationIdType animation_id) { return animation_id < kAnimationIdNumberOfAnimations; }
    static bool IsModeValid(ModeType mode) { return mode < kModeNumberOfModes; }
    StdReturnType SetTime(byte, byte);

    /* Ends a running animation by initialising it, which is what every animation's own
       Init() already means: back to idle, and its progress reset. The selection and the
       mode stay as they are, so the next word change animates again.

       Whatever half-finished frame the animation left is still on the display, so the
       caller has to draw something over it - DisplayManager::AbortAnimation() is the
       entry point that does both. */
    void Abort() { InitCurrentAnimation(); }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
