// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The animations facade: selects the animation and runs it to change the displayed time.
 */

#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

#include "StandardTypes.h"
#include "Bit.h"
#include "Arduino.h"
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

#define ANIMATIONS_SUPPORT_TELETYPE             STD_ON /*!< Whether Teletype is built in. */
#define ANIMATIONS_SUPPORT_CURSOR               STD_ON /*!< Whether Cursor is built in. */
#define ANIMATIONS_SUPPORT_DROP                 STD_ON /*!< Whether Drop is built in. */
#define ANIMATIONS_SUPPORT_WIPE                 STD_ON /*!< Whether Wipe is built in. */
#define ANIMATIONS_SUPPORT_SNAKE                STD_ON /*!< Whether Snake is built in. */
#define ANIMATIONS_SUPPORT_SHIFT                STD_ON /*!< Whether Shift is built in. */
#define ANIMATIONS_SUPPORT_FADE                 STD_ON /*!< Whether Fade is built in. */
#define ANIMATIONS_SUPPORT_CUBE                 STD_ON /*!< Whether Cube is built in. */
#define ANIMATIONS_SUPPORT_FLICKER              STD_ON /*!< Whether Flicker is built in. */
#define ANIMATIONS_SUPPORT_SQUEEZE              STD_ON /*!< Whether Squeeze is built in. */
#define ANIMATIONS_SUPPORT_EXPLODE_PIXELS       STD_ON /*!< Whether Explode pixels is built in. */
#define ANIMATIONS_SUPPORT_EXPLODE_WORDS        STD_ON /*!< Whether Explode words is built in. */
#define ANIMATIONS_SUPPORT_MATRIX               STD_ON /*!< Whether Matrix is built in. */
#define ANIMATIONS_SUPPORT_ROLL                 STD_ON /*!< Whether Roll is built in. */
#define ANIMATIONS_SUPPORT_COLLAPSE             STD_ON /*!< Whether Collapse is built in. */

/*!
 * \brief The task cycle every animation starts with, in scheduler ticks: the speed 246 of the
 *        animation command.
 */
#define ANIMATIONS_TASK_CYCLE_INIT_VALUE        10u

/*!
 * \brief The animations facade: holds every animation, selects the one that carries a word change,
 *        and runs it.
 *
 * SetTime() starts the current animation on a new time, Task() steps it, and GetState() says
 * whether it still runs. Which animation is current is up to the mode: ModeType::kFixed runs the
 * selected animation, SetAnimation(); ModeType::kRandom and ModeType::kSequence pick one of the
 * favourites, SetFavourite(), on each word change.
 *
 * Every animation keeps its own speed, as a task cycle: the scheduler ticks between two of its
 * steps. The animations share one memory, AnimationsType, so making another one current
 * initialises it.
 */
class Animations
{
  public:
    /*! \brief Whether an animation runs, as GetState() returns it. */
    enum class StateType {
        kNone, /*!< Not used: GetState() never returns it. */
        kIdle, /*!< No animation runs. */
        kPending /*!< An animation runs. */
    };

    /*!
     * \brief The animations, by the id the animation command takes.
     *
     * An animation whose `ANIMATIONS_SUPPORT_*` switch is STD_OFF has no value here, and the ids
     * behind it move down by one.
     */
    enum class AnimationIdType {
        kNone, /*!< No animation: the time is drawn without a transition. */
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        kCursor, /*!< Cursor, AnimationCursor. */
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        kTeletype, /*!< Teletype, AnimationTeletype. */
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        kDrop, /*!< Drop, AnimationDrop. */
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        kShift, /*!< Shift, AnimationShift. */
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        kFade, /*!< Fade, AnimationFade. */
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        kSnake, /*!< Snake, AnimationSnake. */
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        kWipe, /*!< Wipe, AnimationWipe. */
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        kCube, /*!< Cube, AnimationCube. */
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        kFlicker, /*!< Flicker, AnimationFlicker. */
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        kSqueeze, /*!< Squeeze, AnimationSqueeze. */
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
        kExplodePixels, /*!< Explode pixels, AnimationExplodePixels. */
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
        kExplodeWords, /*!< Explode words, AnimationExplodeWords. */
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        kMatrix, /*!< Matrix, AnimationMatrix. */
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        kRoll, /*!< Roll, AnimationRoll. */
# endif
# if(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
        kCollapse, /*!< Collapse, AnimationCollapse. */
# endif
        kNumberOfAnimations /*!< The number of ids, kNone included; not an animation. */
    };

    /*! \brief The number of animation ids, AnimationIdType::kNone included. */
    static constexpr byte kNumberOfAnimations{static_cast<byte>(AnimationIdType::kNumberOfAnimations)};

    /*!
     * \brief The mode: which animation runs on a word change.
     *
     * Only ModeType::kFixed uses the selected animation, the others pick one themselves, which is
     * why selecting an animation and selecting a strategy are two separate settings. They pick
     * among the favourites only, and only while no animation runs.
     */
    enum class ModeType {
        kFixed, /*!< The selected animation, SetAnimation(), on every word change. */
        kRandom, /*!< A favourite drawn anew on each word change. */
        kSequence, /*!< The favourites in turn, in the order of their ids, one per word change. */
        kNumberOfModes /*!< The number of modes; not a mode. */
    };

    /*! \brief The number of modes. */
    static constexpr byte kNumberOfModes{static_cast<byte>(ModeType::kNumberOfModes)};

    /*!
     * \brief The memory of the animations: one at a time, as only one runs at a time.
     *
     * Its constructor constructs the first animation that is built in. Animations initialises
     * each animation whenever it makes it the current one.
     */
    union AnimationsType {
# if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
        AnimationCursor cursor; /*!< Cursor, AnimationIdType::kCursor. */
# endif
# if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
        AnimationTeletype teletype; /*!< Teletype, AnimationIdType::kTeletype. */
# endif
# if(ANIMATIONS_SUPPORT_DROP == STD_ON)
        AnimationDrop drop; /*!< Drop, AnimationIdType::kDrop. */
# endif
# if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
        AnimationShift shift; /*!< Shift, AnimationIdType::kShift. */
# endif
# if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
        AnimationWipe wipe; /*!< Wipe, AnimationIdType::kWipe. */
# endif
# if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
        AnimationSnake snake; /*!< Snake, AnimationIdType::kSnake. */
# endif
# if(ANIMATIONS_SUPPORT_FADE == STD_ON)
        AnimationFade fade; /*!< Fade, AnimationIdType::kFade. */
# endif
# if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
        AnimationCube cube; /*!< Cube, AnimationIdType::kCube. */
# endif
# if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
        AnimationFlicker flicker; /*!< Flicker, AnimationIdType::kFlicker. */
# endif
# if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
        AnimationSqueeze squeeze; /*!< Squeeze, AnimationIdType::kSqueeze. */
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
        /*! \brief Explode pixels, AnimationIdType::kExplodePixels. */
        AnimationExplodePixels explode_pixels;
# endif
# if(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
        AnimationExplodeWords explode_words; /*!< Explode words, AnimationIdType::kExplodeWords. */
# endif
# if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
        AnimationMatrix matrix; /*!< Matrix, AnimationIdType::kMatrix. */
# endif
# if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
        AnimationRoll roll; /*!< Roll, AnimationIdType::kRoll. */
# endif
# if(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
        AnimationCollapse collapse; /*!< Collapse, AnimationIdType::kCollapse. */
# endif
        /*! \brief Constructs the first animation that is built in, before its `Init()`. */
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
        /*! \brief Destroys the memory without destroying the animation in it. */
        ~AnimationsType() = default;
    };

  private:
    static constexpr byte kTaskCycleInitValue{ANIMATIONS_TASK_CYCLE_INIT_VALUE};

    // AnimationIdType::kNone means no animation at all, so the ids a mode may pick start
    // behind it
    static constexpr byte kFirstAnimation{ToUnderlying(AnimationIdType::kNone) + 1u};
    static constexpr byte kNumberOfSelectableAnimations{kNumberOfAnimations - kFirstAnimation};
    /* odd factor of the hash in CalcRandomAnimation(), see there */
    static constexpr byte kRandomHashFactor{181u};

    static_assert(kNumberOfAnimations <= 16u,
                  "Animations: too many animations for the favourites bit mask, please widen it");

    /*
     * one bit per animation id, bit 0 of AnimationIdType::kNone stays unused.
     * Shifted in an unsigned long rather than an unsigned int: there are sixteen
     * animations, and on a target whose int is 16 bits wide - every AVR - 1u shifted by
     * sixteen is undefined and the mask comes out empty, which silently leaves no
     * animation a favourite.
     */
    static constexpr uint16_t kAllFavourites{
        static_cast<uint16_t>(((1uL << kNumberOfAnimations) - 1uL) & ~((1uL << kFirstAnimation) - 1uL))
    };

    std::array<byte, kNumberOfAnimations> task_cycles_;
    /* what the user selected, used as it is in ModeType::kFixed */
    AnimationIdType animation_id_{AnimationIdType::kNone};
    /* what is really running, equal to animation_id_ unless a mode selected something */
    AnimationIdType current_animation_id_{AnimationIdType::kNone};
    ModeType mode_{ModeType::kFixed};
    /* animations the selecting modes may pick, all of them until told otherwise */
    Bit<uint16_t> favourites_{kAllFavourites};
    uint16_t random_state_{0u};
    AnimationsType animations_raw_;

    Animations() : animations_raw_() { task_cycles_.fill(kTaskCycleInitValue); }
    ~Animations() = default;

    Animation::StateType GetStateOfCurrentAnimation() const;
    void TaskOfCurrentAnimation();
    StdReturnType ShowOfCurrentAnimation() const;
    void InitCurrentAnimation();
    void SwitchCurrentAnimation(AnimationIdType animation_id);
    void SelectAnimationOfMode();
    AnimationIdType CalcRandomAnimation();
    AnimationIdType CalcNextAnimation() const;
    byte NumberOfFavourites() const;

    /*
     * The round a knob walks, which is not the one CalcNextAnimation() walks: that one
     * skips everything but the favourites, because ModeType::kSequence picks among those.
     * Stepping the selection by hand may reach every animation, favourite or not.
     */
    static constexpr AnimationIdType ToNextAnimation(AnimationIdType animation_id) {
        return (ToUnderlying(animation_id) + 1u >= kNumberOfAnimations)
             ? AnimationIdType::kNone : static_cast<AnimationIdType>(ToUnderlying(animation_id) + 1u);
    }
    static constexpr AnimationIdType ToPreviousAnimation(AnimationIdType animation_id) {
        return (animation_id == AnimationIdType::kNone)
             ? static_cast<AnimationIdType>(kNumberOfAnimations - 1u)
             : static_cast<AnimationIdType>(ToUnderlying(animation_id) - 1u);
    }

  public:
    /*! \brief Returns the one animations facade. */
    static Animations& GetInstance() {
        static Animations singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Returns whether an animation runs.
     *
     * \return StateType::kPending while the current animation is in either of its phases,
     *         StateType::kIdle otherwise and while there is no current animation.
     */
    StateType GetState() const;

    /*! \brief Returns the selected animation, the one ModeType::kFixed runs. */
    AnimationIdType GetAnimation() const { return animation_id_; }

    /*! \brief Returns the current animation: the selected one, or the one the mode picked. */
    AnimationIdType GetCurrentAnimation() const { return current_animation_id_; }

    /*! \brief Returns the mode. */
    ModeType GetMode() const { return mode_; }

    /*!
     * \brief Returns whether `animation_id` is a favourite, one that ModeType::kRandom and
     *        ModeType::kSequence may pick.
     *
     * \param[in] animation_id The animation.
     * \return True if it is a favourite; false for AnimationIdType::kNone.
     * \pre `animation_id` is valid, IsAnimationValid(); it is not checked.
     */
    bool IsFavourite(AnimationIdType animation_id) const {
        return favourites_[ToUnderlying(animation_id)];
    }

    /*!
     * \brief Returns the task cycle of `animation_id`: the scheduler ticks between two of its
     *        steps.
     *
     * \param[in] animation_id The animation.
     * \return The task cycle in ticks, 0 for an animation that is stopped.
     * \pre `animation_id` is valid, IsAnimationValid(); it is not checked.
     */
    byte GetTaskCycle(AnimationIdType animation_id) const { return task_cycles_[ToUnderlying(animation_id)]; }

    /*!
     * \brief Returns the task cycle of the current animation, the one the scheduler runs Task() at.
     *
     * The cycle of the running animation, so every animation keeps its own speed while a mode
     * switches between them.
     *
     * \return The task cycle in ticks, 0 for an animation that is stopped.
     */
    byte GetTaskCycle() const { return task_cycles_[ToUnderlying(current_animation_id_)]; }

    /*!
     * \brief Sets the task cycle of `animation_id`: the scheduler ticks between two of its steps.
     *
     * Scheduler::ConvertSpeedToTaskCycle() turns the speed of the animation command into it. While
     * the current animation has a cycle of 0, the scheduler does not run Task(), so the animation
     * stops where it is.
     *
     * \param[in] animation_id The animation; AnimationIdType::kNone is taken too.
     * \param[in] cycle The task cycle in ticks, 0 to stop the animation.
     * \return kEOk, or kENotOk if `animation_id` is not valid, IsAnimationValid(); the task cycles
     *         are then left as they were.
     */
    StdReturnType SetTaskCycle(AnimationIdType animation_id, byte cycle);

    /*!
     * \brief Selects `animation_id`, the animation ModeType::kFixed runs, and makes it the current
     *        animation at once.
     *
     * The animation that was current is initialised first, which ends it where it stands and
     * puts back what it changed on the display, a fade's dimming or a flicker's switching off.
     * Then `animation_id` is initialised; nothing is drawn. In ModeType::kFixed it runs on the
     * next SetTime(). A mode that picks its own picks again on the next SetTime(),
     * ModeType::kSequence going on with the favourite after `animation_id`.
     *
     * \param[in] animation_id The animation, or AnimationIdType::kNone for none.
     * \return kEOk, or kENotOk if `animation_id` is not valid, IsAnimationValid(); the selection
     *         and the current animation are then left as they were.
     */
    StdReturnType SetAnimation(AnimationIdType animation_id);

    /*!
     * \brief Sets the mode, which decides which animation runs on a word change.
     *
     * ModeType::kFixed makes the selected animation the current one again as SetAnimation()
     * does, even if the mode was ModeType::kFixed already. The selecting modes leave the current
     * animation alone and pick on the next SetTime().
     *
     * \param[in] mode The mode.
     * \return kEOk, or kENotOk if `mode` is not valid, IsModeValid(); the mode is then left as it
     *         was.
     */
    StdReturnType SetMode(ModeType mode);

    /*!
     * \brief Sets whether `animation_id` is a favourite, one that ModeType::kRandom and
     *        ModeType::kSequence may pick.
     *
     * Every animation is one by default. A running animation is not affected.
     *
     * \param[in] animation_id The animation.
     * \param[in] favourite True to make it a favourite, false to leave it out.
     * \return kEOk, or kENotOk if `animation_id` is AnimationIdType::kNone or not valid, or if
     *         `favourite` is false and it is the last favourite; the favourites are then left as
     *         they were.
     */
    StdReturnType SetFavourite(AnimationIdType animation_id, bool favourite) {
        /* AnimationIdType::kNone is no animation and cannot be a favourite */
        if(ToUnderlying(animation_id) < kFirstAnimation || !IsAnimationValid(animation_id)) { return kENotOk; }
        // the last favourite may not be dropped, the selecting modes would be left
        // with nothing to pick
        if(!favourite && IsFavourite(animation_id) && NumberOfFavourites() <= 1u) { return kENotOk; }

        favourites_[ToUnderlying(animation_id)] = favourite;
        return kEOk;
    }

    /*!
     * \brief Puts the animations back to how a new clock has them: every speed at its initial
     *        cycle, every animation a favourite again, nothing selected, and ModeType::kFixed.
     *
     * The same settings the constructor leaves behind. Written out here rather than by
     * constructing a second instance, which a singleton cannot do.
     */
    void ResetToDefaults() {
        task_cycles_.fill(kTaskCycleInitValue);
        favourites_.SetValue(kAllFavourites);
        // Both are valid by construction, so neither answer can say anything a caller of
        // ResetToDefaults() could act on.
        SetAnimation(AnimationIdType::kNone);
        SetMode(ModeType::kFixed);
    }

    /*!
     * \brief Selects the animation after the selected one, through SetAnimation(); after the last
     *        comes AnimationIdType::kNone.
     *
     * NextAnimation() and PreviousAnimation() step one animation further and one back, wrapping at
     * both ends, for a control that has no list to pick from. AnimationIdType::kNone is part of
     * that round rather than skipped: "no animation" is a setting like any other, and a knob is
     * the only way somebody without a phone gets back to it. The round reaches every animation,
     * favourite or not.
     *
     * They step the selection, so they mean what `9 -A<id>` means and not what
     * `RpcIdType::kAnimationStart` means - the animation runs at the next word change, and in a
     * mode that picks its own the selection is what ModeType::kFixed would return to. Like
     * SetAnimation(), they end a running animation through its own `Init()` first.
     */
    void NextAnimation() { SetAnimation(ToNextAnimation(animation_id_)); }

    /*!
     * \brief Selects the animation before the selected one, through SetAnimation(); before
     *        AnimationIdType::kNone comes the last.
     *
     * It is NextAnimation() the other way round.
     */
    void PreviousAnimation() { SetAnimation(ToPreviousAnimation(animation_id_)); }

    /*!
     * \brief Runs one step of the current animation, while it runs.
     *
     * It does nothing while no animation runs. The scheduler calls it every GetTaskCycle() ticks,
     * with `show` true.
     *
     * \param[in] show True to show the display after the step, the step that ends the animation
     *                 included; false to leave that to the caller.
     */
    void Task(bool show=false);

    /*!
     * \brief Shows the display while an animation runs, through Display::Show().
     *
     * \return What Display::Show() returns, kEOk on every backend, or kENotOk while no animation
     *         runs; nothing is shown then.
     */
    StdReturnType Show() const;

    /*!
     * \brief Returns whether `animation_id` is an id of AnimationIdType, AnimationIdType::kNone
     *        included.
     *
     * \param[in] animation_id The id.
     * \return True if `animation_id` lies below AnimationIdType::kNumberOfAnimations.
     */
    static bool IsAnimationValid(AnimationIdType animation_id) { return animation_id < AnimationIdType::kNumberOfAnimations; }

    /*!
     * \brief Returns whether `mode` is a mode.
     *
     * \param[in] mode The mode.
     * \return True if `mode` lies below ModeType::kNumberOfModes.
     */
    static bool IsModeValid(ModeType mode) { return mode < ModeType::kNumberOfModes; }

    /*!
     * \brief Starts the current animation on the time `hour`:`minute`.
     *
     * If none runs, the animation is initialised first, so that every run starts as the first
     * did; in ModeType::kRandom and ModeType::kSequence the mode picks it before. While one runs,
     * the mode picks nothing, and the time goes to the running animation, which runs on and ends
     * on the newer time.
     *
     * With AnimationIdType::kNone current, the words of the time are lit through
     * Clock::SetTime(byte, byte), over what is lit already. Nothing is shown either way; Task()
     * shows the steps.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return What the `SetTime()` of the current animation returns, or with none what
     *         Clock::SetTime(byte, byte) returns. The pick of the mode stands either way.
     */
    StdReturnType SetTime(byte hour, byte minute);

    /*!
     * \brief Ends a running animation by initialising it.
     *
     * That is what every animation's own `Init()` already means: back to idle, and its progress
     * reset. The selection and the mode stay as they are, so the next word change animates again.
     *
     * Whatever half-finished frame the animation left is still on the display, so the caller has
     * to draw something over it - DisplayManager::AbortAnimation() is the entry point that does
     * both.
     */
    void Abort() { InitCurrentAnimation(); }

};

#endif
