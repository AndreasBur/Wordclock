// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The shift animation: the old time shifts out of the display, then the new one shifts in.
 */

#ifndef _ANIMATION_SHIFT_H_
#define _ANIMATION_SHIFT_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Transformation.h"
#include "Animation.h"

/*! \brief STD_ON to shift by columns, to the right; exactly one of the two switches is STD_ON. */
#define ANIMATION_SHIFT_HORIZONTAL              STD_ON
/*! \brief STD_ON to shift by rows, downwards; exactly one of the two switches is STD_ON. */
#define ANIMATION_SHIFT_VERTICAL                STD_OFF

#if (ANIMATION_SHIFT_HORIZONTAL == STD_OFF && ANIMATION_SHIFT_VERTICAL == STD_OFF)
# error "AnimationShift: Exactly one switch must be set to STD_ON"
#endif

#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON && ANIMATION_SHIFT_VERTICAL == STD_ON)
# error "AnimationShift: ANIMATION_SHIFT_HORIZONTAL or ANIMATION_SHIFT_VERTICAL has to be STD_ON, but not both"
#endif

/*!
 * \brief The shift animation: shifts the previous time out of the display, then shifts the new one
 *        in, so the display is empty in between.
 *
 * With ANIMATION_SHIFT_HORIZONTAL it shifts the whole display to the right, one column per step,
 * through Transformation::ShiftRight(). In the first half, while GetState() returns kClearTime,
 * the previous time leaves at the right. In the second half, while it returns kSetTime, the new
 * time comes in at the left, its rightmost column first. With ANIMATION_SHIFT_VERTICAL it is the
 * same with rows, shifted down.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationShift : public Animation
{
  public:

  private:
    Transformation wc_transformation_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};

#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON)
    byte current_column_{0u};
#endif

#if (ANIMATION_SHIFT_VERTICAL == STD_ON)
    byte current_row_{0u};
#endif

    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationShift() : wc_transformation_() { }

    /*! \brief Destroys the animation. */
    ~AnimationShift() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59, and the animation is
     *         then left as it was, or if it is not idle; a running one then runs on, but ends on
     *         the words of `hour`:`minute`.
     */
    StdReturnType SetTime(byte hour, byte minute);

    /*!
     * \brief Takes one step of the animation; does nothing while it is idle.
     *
     * With ANIMATION_SHIFT_HORIZONTAL a step of the first half shifts the display one column to
     * the right; the step after DISPLAY_NUMBER_OF_COLUMNS of them only starts the second half. A
     * step of the second half shifts the display one column to the right and lights, in the
     * leftmost column, the column of the new time that the remaining shifts carry to its place;
     * the step after DISPLAY_NUMBER_OF_COLUMNS of them ends it. With ANIMATION_SHIFT_VERTICAL it
     * is the same with rows, DISPLAY_NUMBER_OF_ROWS of them in each half.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
