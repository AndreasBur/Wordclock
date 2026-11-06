// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The teletype animation: the words of the new time are written letter by letter.
 */

#ifndef _ANIMATION_TELETYPE_H_
#define _ANIMATION_TELETYPE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"

/*!
 * \brief The teletype animation: writes the words of the new time letter by letter, one word after
 *        the other.
 *
 * The words come in the order of the clock's word list: ES IST where it stands, then the minute
 * words, then the hour words. The previous time stays lit until the last step. It runs in one
 * part, while GetState() returns kSetTime.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationTeletype : public Animation
{
  public:

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte current_word_index_{0u};
    byte current_word_length_{0u};
    byte current_char_index_{0u};

    void Reset();
    StdReturnType SetNextWordIndex();

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationTeletype() : words_() { }

    /*! \brief Destroys the animation. */
    ~AnimationTeletype() = default;

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
     * A step lights one more letter of the word it writes, from the left. It takes one step per
     * letter of the new time, one that lights nothing for each place of the word list that holds
     * no word, and a last one that ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
