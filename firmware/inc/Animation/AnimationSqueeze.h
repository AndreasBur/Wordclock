// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The squeeze animation: the old words are eaten away letter by letter, the new ones grow.
 */

#ifndef _ANIMATION_SQUEEZE_H_
#define _ANIMATION_SQUEEZE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/*!
 * \brief The squeeze animation: the words of the previous time are eaten away letter by letter,
 *        from the left or from the right as selected for each word change; the new ones grow out of
 *        their first letter.
 *
 * In the first half, while GetState() returns kClearTime, every block of lit pixels in a row
 * loses one pixel per step, on the side of the direction, so all words of a row get shorter at
 * the same time. In the second half, while it returns kSetTime, every word of the new time gains
 * one letter per step, from its first letter on.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 *
 * \warning Only Init() resets the count of letters lit and the length of the longest word; the end
 *          of the animation leaves them as they are. Started again without an Init() in between, as
 *          Animations does at every word change in Animations::ModeType::kFixed, the second half
 *          goes on from the length the run before ended at, so the new time does not grow but
 *          appears whole, or nearly whole, in its first step.
 */
class AnimationSqueeze : public Animation
{
  public:

  public:
    /*!
     * \brief The side the letters are eaten away from, selected for each word change.
     *
     * From the right is what the reference project does, from the left is the other half of the
     * coin.
     */
    enum class DirectionType {
        kFromLeft,          /*!< From the left. */
        kFromRight,         /*!< From the right. */
        kNumberOfDirections /*!< The number of directions, not a direction. */
    };

    /*! \brief The number of directions of DirectionType, as a byte. */
    static constexpr byte kNumberOfDirections{static_cast<byte>(DirectionType::kNumberOfDirections)};

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte max_word_length_{0u};
    byte current_length_{1u};
    DirectionType direction_{DirectionType::kFromLeft};

    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

    void ClearRow(byte row);
    void SetMaxWordLength();

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationSqueeze() : words_() { }

    /*! \brief Destroys the animation. */
    ~AnimationSqueeze() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself. It selects the direction from `hour` and `minute`, so that it varies
     * from one word change to the next and the same time always takes the same one.
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
     * A step of the first half turns off, in every block of lit pixels of a row, the pixel at its
     * end on the side of the direction; the step that leaves the display dark also starts the
     * second half. A step of the second half lights one more letter of every word of the new time,
     * from the left; the step that lights the longest word whole also ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
