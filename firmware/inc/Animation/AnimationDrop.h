// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The drop animation: the letters of the new time fall down their column into place.
 */

#ifndef _ANIMATION_DROP_H_
#define _ANIMATION_DROP_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/*!
 * \brief The drop animation: the letters of the new time fall down their column into place.
 *
 * In the first half, while GetState() returns kClearTime, the lit pixels of the previous time
 * fall out of the display at the bottom, one after the other, the one with the highest index
 * first. In the second half, while it returns kSetTime, the letters of the new time fall from the
 * top row, one after the other: word by word from the last word of the clock's word list to the
 * first, and each word from left to right.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationDrop : public Animation
{
  public:

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte row_{0u};
    byte column_{0u};
    byte current_word_index_{0u};

    void Reset();
    StdReturnType SetNextActivePixelIndex();
    void ClearTimeTask();
    void SetTimeTask();
    StdReturnType SetNextWordIndex();
    void SetStateToSetTime();
    StdReturnType SetNextRow(byte max_row);
    StdReturnType SetNextColumn(byte max_column);

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationDrop() : words_() { }

    /*! \brief Destroys the animation. */
    ~AnimationDrop() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself, unless the display is dark: it then starts the second half at once
     * and lights the top pixel of the first letter to fall.
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
     * In the first half a step moves the falling pixel one row down, and the step that would take
     * it below the last row turns it off and picks the next. The step that turns the last one off
     * starts the second half and lights the top pixel of the first letter to fall.
     *
     * In the second half a step moves the falling letter one row down, until it reaches the row of
     * its word; the step after that lights the next letter in the top row. The step after the
     * last letter has landed ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
