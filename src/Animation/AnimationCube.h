// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The cube animation: a rectangle shrinks to the centre and grows back over the new time.
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATIONCUBE_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATIONCUBE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Animation.h"
#include "ClockWords.h"

/*! \brief The left column of the smallest frame. */
#define ANIMATION_CUBE_COLUMN_START_MAX_VALUE 4u
/*! \brief The right column of the smallest frame. */
#define ANIMATION_CUBE_COLUMN_END_MIN_VALUE 6u

/*! \brief The top row of the smallest frame. */
#define ANIMATION_CUBE_ROW_START_MAX_VALUE 4u
/*! \brief The bottom row of the smallest frame. */
#define ANIMATION_CUBE_ROW_END_MIN_VALUE 5u

/*!
 * \brief The cube animation: a rectangle shrinks towards the centre and grows back out, leaving the
 *        new letters behind.
 *
 * The rectangle is a lit frame, one pixel wide. In the first half, while GetState() returns
 * kClearTime, it starts along the edge of the display and shrinks one pixel per side and step to
 * the smallest frame the ANIMATION_CUBE macros give; the previous time goes out as the frame
 * passes over it. In the second half, while it returns kSetTime, it grows back from the smallest
 * frame to the edge, and the letters of the new time it passes over stay lit.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationCube : public Animation {
 public:
  /*! \brief A frame: its first and last column and its first and last row, counted from 0. */
  struct BorderType {
    byte column_start; /*!< The left column. */
    byte column_end;   /*!< The right column. */
    byte row_start;    /*!< The top row. */
    byte row_end;      /*!< The bottom row. */
  };

  /*! \brief Where the frame stands in its half. */
  enum class BorderStateType {
    kMin,    /*!< At the smallest frame, not lit yet. */
    kMax,    /*!< Along the edge of the display, not lit yet. */
    kBetween /*!< Lit, on its way. */
  };

 private:
  ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
  BorderType border_{ANIMATION_CUBE_COLUMN_START_MAX_VALUE, ANIMATION_CUBE_COLUMN_END_MIN_VALUE,
                     ANIMATION_CUBE_ROW_START_MAX_VALUE, ANIMATION_CUBE_ROW_END_MIN_VALUE};
  BorderStateType border_state_{BorderStateType::kMin};

  void Reset();
  void SetStateToSetTime();
  void SetBorderPixels() { WriteBorderPixels(true); }
  void ClearBorderPixels() { WriteBorderPixels(false); }
  void ClearBorderPixelsWithoutClockPixels();
  void WriteBorderPixels(bool value) const;
  StdReturnType IncreaseBorder();
  StdReturnType DecreaseBorder();
  void ClearTimeTask();
  void SetTimeTask();
  void SetMinBorder();
  void SetMaxBorder();

 public:
  /*! \brief Makes the animation, with GetState() kNone until Init(). */
  constexpr AnimationCube() {}

  /*! \brief Destroys the animation. */
  ~AnimationCube() = default;

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
   * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59, and the animation is then
   *         left as it was, or if it is not idle; a running one then runs on, but ends on the words
   *         of `hour`:`minute`.
   */
  StdReturnType SetTime(byte hour, byte minute);

  /*!
   * \brief Takes one step of the animation; does nothing while it is idle.
   *
   * The first step of the first half lights the frame along the edge of the display. Each
   * further step turns the frame off and lights it one pixel further in on every side; the step
   * that cannot shrink it any further turns it off and starts the second half.
   *
   * The first step of the second half lights the smallest frame. Each further step turns the
   * frame off, but for the letters of the new time on it, and lights it one pixel further out on
   * every side; the step that cannot grow it any further ends it.
   *
   * It writes the pixels of Display and leaves showing them to Show().
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATIONCUBE_H_
