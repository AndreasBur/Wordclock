// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The cursor animation: a single pixel walks across all LEDs and leaves the new time behind.
 */

#ifndef _ANIMATION_CURSOR_H_
#define _ANIMATION_CURSOR_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"

/*!
 * \brief The cursor animation: a single pixel walks across all LEDs and leaves the letters of the
 *        new time lit behind it.
 *
 * The cursor walks the pixels in the order of their index, row by row from the top and each row
 * from left to right. A letter of the previous time goes out as the cursor passes it. It runs in
 * one part, while GetState() returns kSetTime.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationCursor : public Animation
{
  public:

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte current_pixel_index_{0u};

    void Reset();

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationCursor() { }

    /*! \brief Destroys the animation. */
    ~AnimationCursor() { }

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
     * A step lights the pixel the cursor moves onto and turns off the one it leaves, unless that
     * one is a letter of the new time. It takes one step per pixel and one more,
     * DISPLAY_NUMBER_OF_PIXELS + 1 in all; the last one ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
