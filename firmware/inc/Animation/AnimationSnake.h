// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The snake animation: a short snake winds through the rows and leaves the new time behind.
 */

#ifndef _ANIMATION_SNAKE_H_
#define _ANIMATION_SNAKE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

#define ANIMATION_SNAKE_LENGTH              8u /*!< The length of the snake in pixels. */

/*!
 * \brief The snake animation: a short snake travels through the display and leaves the letters of
 *        the new time lit behind it.
 *
 * The snake winds through the rows from the top left: from left to right through the rows counted
 * even from 0, from right to left through the odd ones. It does so whatever
 * DISPLAY_LED_STRIPE_SERPENTINE says, so it follows the LED strip only where the strip is wired
 * as a serpentine. A letter of the previous time goes out as the tail passes it. It runs in one
 * part, while GetState() returns kSetTime.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationSnake : public Animation
{
  public:
    /*!
     * \brief A direction of the snake along a row.
     *
     * The animation keeps one in a member, but its steps do not read it: they wind the snake
     * through the rows by the index of its head and tail.
     */
    enum class SnakeStateType {
        kToRight, /*!< To the right. */
        kToLeft   /*!< To the left. */
    };

  private:
    SnakeStateType snake_state_{SnakeStateType::kToRight};
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte snake_begin_index_{0u};
    byte snake_end_index_{0u};

    void Reset();
    static byte TransformToSerpentine(byte column, byte row);
    static byte TransformToSerpentine(byte index);

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationSnake() { }

    /*! \brief Destroys the animation. */
    ~AnimationSnake() = default;

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
     * A step lights the pixel at the head of the snake and moves the head on. Once the snake is
     * ANIMATION_SNAKE_LENGTH pixels long, the step also turns off the pixel at its tail, unless
     * that one is a letter of the new time, and moves the tail on. The head stops at the last
     * pixel and the tail catches up; the step in which the tail leaves the last pixel ends it,
     * DISPLAY_NUMBER_OF_LEDS + ANIMATION_SNAKE_LENGTH steps in all.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
