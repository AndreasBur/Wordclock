// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The roll animation: the old time rolls out while the new one rolls in behind it.
 *
 * Unlike AnimationShift, which first shifts the previous time out of an empty display and only then
 * shifts the new one in, both move together here. The display therefore never goes blank and the
 * animation takes one pass instead of two.
 *
 * The direction is selected for each word change out of right, left, down and up.
 *
 * Ported from the wordclock24h project by Frank Meyer (display_animation_roll in
 * src/display/display.c).
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATIONROLL_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATIONROLL_H_

#include "Animation.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "StandardTypes.h"
#include "Transformation.h"

/*!
 * \brief The roll animation: the previous time rolls out while the new one rolls in behind it,
 *        direction selected for each word change.
 *
 * Each step shifts the whole display one column or one row in the direction, through
 * Transformation, and lights the column or row of the new time that comes in at the freed edge.
 * The previous time leaves at the opposite edge. It runs in one part, while GetState() returns
 * kSetTime.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationRoll : public Animation {
 public:
  /*! \brief The direction the display rolls in. */
  enum class DirectionType {
    kRight,             /*!< To the right; the new time comes in at the left. */
    kLeft,              /*!< To the left; the new time comes in at the right. */
    kDown,              /*!< Down; the new time comes in at the top. */
    kUp,                /*!< Up; the new time comes in at the bottom. */
    kNumberOfDirections /*!< The number of directions, not a direction. */
  };

  /*! \brief The number of directions of DirectionType, as a byte. */
  static constexpr byte kNumberOfDirections{static_cast<byte>(DirectionType::kNumberOfDirections)};

 private:
  Transformation wc_transformation_;
  ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
  byte step_{0u};
  DirectionType direction_{DirectionType::kRight};

  void Reset();
  void SetTimeTask();
  void RollHorizontal();
  void RollVertical();
  byte NumberOfSteps() const;
  static DirectionType CalcDirection(byte hour, byte minute);

 public:
  /*! \brief Makes the animation, with GetState() kNone until Init(). */
  constexpr AnimationRoll() : wc_transformation_() {}

  /*! \brief Destroys the animation. */
  ~AnimationRoll() = default;

  /*! \brief Returns the direction of the roll that runs, or kRight while none runs. */
  DirectionType GetDirection() const { return direction_; }

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
   * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59, and the animation is then
   *         left as it was, or if it is not idle; a running one then runs on, but ends on the words
   *         of `hour`:`minute`.
   */
  StdReturnType SetTime(byte hour, byte minute);

  /*!
   * \brief Takes one step of the animation; does nothing while it is idle.
   *
   * A step shifts the display one column or row and lights the column or row of the new time
   * that the remaining shifts carry to its place. After DISPLAY_NUMBER_OF_COLUMNS steps to the
   * right or left, or DISPLAY_NUMBER_OF_ROWS down or up, the new time stands in its place, and
   * the step after them ends it.
   *
   * It writes the pixels of Display and leaves showing them to Show().
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATIONROLL_H_
