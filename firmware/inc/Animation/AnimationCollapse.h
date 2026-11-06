// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The collapse animation: the letters slide together, and the new ones slide apart.
 *
 * The letters of the previous time slide to one edge of their row, closing every gap between the
 * words on the way, until each row is one solid block. The letters of the new time start as such a
 * block and slide apart into their words. The edge is selected for each word change.
 *
 * Unlike AnimationSqueeze, which lets the letters disappear one by one, nothing vanishes here: the
 * letters move.
 */

#ifndef _ANIMATION_COLLAPSE_H_
#define _ANIMATION_COLLAPSE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"

/*!
 * \brief The collapse animation: the letters slide to one edge of their row and close every gap,
 *        then the new ones slide apart into their words; the edge is selected for each word change.
 *
 * In the first half, while GetState() returns kClearTime, every word of a row that has a gap
 * before it on the side of the edge slides one column towards the edge per step, until each row
 * is one solid block at the edge. In the second half, while it returns kSetTime, the letters of
 * the new time start as such a block and slide apart into their words, each a share of its way
 * per step, so that all of them arrive in the same step.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationCollapse : public Animation
{
  public:
    /*! \brief The edge the letters collapse against, selected for each word change. */
    enum class DirectionType {
        kToLeft,            /*!< The left edge. */
        kToRight,           /*!< The right edge. */
        kNumberOfDirections /*!< The number of directions, not a direction. */
    };

    /*! \brief The number of directions of DirectionType, as a byte. */
    static constexpr byte kNumberOfDirections{static_cast<byte>(DirectionType::kNumberOfDirections)};

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte step_{0u};
    DirectionType direction_{DirectionType::kToLeft};
    /*
     * Steps the expansion needs, which is the longest way a single letter has to
     * travel. Taking the display width instead would let the letters arrive long
     * before the animation ends, because the gaps between words are only one or two
     * columns wide.
     */
    byte expand_steps_{1u};

    void Reset();
    void ClearTimeTask();
    void SetTimeTask();
    bool CollapseRow(byte row);
    byte CalcColumn(byte final_column, byte block_column) const;
    byte CalcBlockColumn(byte letter_index, byte letters_of_row) const;
    byte CountLettersOfRow(byte row) const;
    byte CalcExpandSteps() const;

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationCollapse() { }

    /*! \brief Destroys the animation. */
    ~AnimationCollapse() { }

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
     * A step of the first half slides every word of a row that has a gap before it one column
     * towards the edge; the first step in which nothing moves starts the second half. A step of
     * the second half clears the display and draws the letters of the new time a share of their
     * way further from the block. It takes one step more than the longest way a letter has to go,
     * in columns, and never fewer than two: the first draws the block, and the last draws the
     * letters in their places and ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
