// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The base class of all animations, with the state and helpers they share.
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATION_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATION_H_

#include "Arduino.h"
#include "ClockWords.h"
#include "StandardTypes.h"

/*!
 * \brief The base of every animation: its state, and the helpers the animations share.
 *
 * An animation carries the display from the time it shows to a new one, in up to two phases:
 * StateType::kClearTime takes the previous time off, StateType::kSetTime puts the new one on.
 * Each animation derives from this class and has an `Init()`, a `SetTime(byte, byte)` and a
 * `Task()` of its own. They are not virtual: Animations holds every animation and calls them on
 * the one it runs.
 */
class Animation {
 public:
  /*! \brief The state of an animation. */
  enum class StateType {
    kNone,      /*!< Not initialised yet: the state before the first `Init()`. */
    kUninit,    /*!< Not used by any animation. */
    kIdle,      /*!< Initialised and not running: ready to take a new time. */
    kClearTime, /*!< Running, in the phase that takes the previous time off the display. */
    kSetTime    /*!< Running, in the phase that puts the new time on the display. */
  };

 private:
 protected:
  /*! \brief The odd factor of the hash in SelectByTime(). */
  static constexpr byte kSelectHashFactor{181u};

  /*! \brief The state the animation is in; each animation moves it on itself. */
  StateType state_{StateType::kNone};

  /*!
   * \brief Sets the state to `state`.
   *
   * Every animation's own `Init()` calls it with StateType::kIdle.
   *
   * \param[in] state The state.
   */
  void Init(StateType state) { state_ = state; }

  /*!
   * \brief Draws the words of `clock_words_table` on a cleared display and ends the animation.
   *
   * Every animation whose set phase places the letters itself ends through one of these two
   * instead of just going idle: drawing the target once more makes the last frame authoritative,
   * so a pixel the animation skipped or hit twice cannot leave a word of the previous time
   * behind. AnimationFade ends through FinishWithTime() as well, although its set phase only
   * ramps the brightness of letters already in place: a newer time that arrived while it was
   * fading up is then on the display when it goes idle.
   *
   * Every pixel is turned off, the words are lit through Clock::SetTime(), and the state goes
   * to StateType::kIdle. Nothing is shown; Show() does that.
   *
   * \param[in] clock_words_table The words of the new time, as Clock::GetClockWords() returns them.
   */
  void FinishWithClockWords(ClockWords::WordsListType const& clock_words_table);

  /*!
   * \brief Draws the words of `hour`:`minute` on a cleared display and ends the animation.
   *
   * For the animations that kept the time rather than the word list. It does what
   * FinishWithClockWords() does, with the words Clock::SetTime(byte, byte) lights in the
   * clock's current mode.
   *
   * \param[in] hour The hour, 0 to 23.
   * \param[in] minute The minute, 0 to 59.
   * \note If `hour` or `minute` is out of range, nothing is lit and the display is left cleared.
   */
  void FinishWithTime(byte hour, byte minute);

  /*!
   * \brief Returns a value below `number_of_values` that depends on `hour` and `minute` only.
   *
   * The same time always gives the same value, and from one minute to the next the values
   * scatter as a random generator's would. An animation uses it to vary from one word change to
   * the next, such as in its direction.
   *
   * \param[in] hour The hour.
   * \param[in] minute The minute.
   * \param[in] number_of_values The number of values to choose from.
   * \return The value, from 0 to `number_of_values` - 1; 0 if `number_of_values` is 0.
   */
  static byte SelectByTime(byte hour, byte minute, byte number_of_values);

 public:
  /*! \brief Makes an animation in the state StateType::kNone, before its `Init()`. */
  constexpr Animation() : state_{StateType::kNone} {}

  /*! \brief Destroys the animation. */
  ~Animation() = default;

  /*! \brief Returns the state the animation is in. */
  StateType GetState() const { return state_; }

  /*! \copydoc Display::Show() */
  static StdReturnType Show();

  /*!
   * \brief Returns whether the pixel at `column` and `row` belongs to a word of
   *        `clock_words_table`.
   *
   * The list is read up to its first `kNone`; a word behind it is not looked at.
   *
   * \param[in] clock_words_table The words, as Clock::GetClockWords() returns them.
   * \param[in] column The column, counted from 0.
   * \param[in] row The row, counted from 0.
   * \return True if a word of the list stands in `row` and covers `column`.
   */
  static bool IsPixelPartOfClockWords(ClockWords::WordsListType clock_words_table, byte column,
                                      byte row);

  /*!
   * \brief Returns whether the pixel at `index` belongs to a word of `clock_words_table`.
   *
   * The list is read up to its first `kNone`; a word behind it is not looked at.
   *
   * \param[in] clock_words_table The words, as Clock::GetClockWords() returns them.
   * \param[in] index The index of the pixel, row * DISPLAY_NUMBER_OF_COLUMNS + column.
   * \return True if a word of the list covers the pixel.
   */
  static bool IsPixelPartOfClockWords(ClockWords::WordsListType clock_words_table, byte index);
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATION_H_
