// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The flicker animation: the display flickers a few times, then the new time is there.
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATIONFLICKER_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATIONFLICKER_H_

#include "Animation.h"
#include "Arduino.h"
#include "StandardTypes.h"

/*!
 * \brief One less than the switches of the flicker; the display is switched off and on this many
 *        times and once more, starting with off.
 */
#define ANIMATION_FLICKER_COUNTER_INIT_VALUE 4u

/*!
 * \brief The flicker animation: the display flickers a few times, then the new time is there.
 *
 * In the first half, while GetState() returns kClearTime, it switches a display that was on when
 * the run started off and on, through Display::Disable() and Display::Enable(), while the display
 * keeps the previous time. The second half, while it returns kSetTime, is one step.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle; it also switches the display back on if the flicker had switched it off.
 *
 * A display that is switched off when the run starts, by hand or by NightSwitch, is not
 * flickered and stays off: the run only counts its steps and then lights the words.
 */
class AnimationFlicker : public Animation {
 public:
 private:
  byte minute_{0u};
  byte hour_{0u};
  bool is_clock_set_{false};
  // Whether the display was on when the run started; a display that was off is not flickered.
  bool display_was_enabled_{true};
  byte flicker_counter_{ANIMATION_FLICKER_COUNTER_INIT_VALUE};

  void Reset();
  void ClearTimeTask();
  void SetTimeTask();

 public:
  /*! \brief Makes the animation, with GetState() kNone until Init(). */
  constexpr AnimationFlicker() {}

  /*! \brief Destroys the animation. */
  ~AnimationFlicker() = default;

  /*!
   * \brief Makes the animation idle and resets its progress.
   *
   * A running animation ends where it stands, and the display keeps what its last step drew.
   * A display the flicker switched off is switched on again, so that a flicker cut short does
   * not leave it dark; one switched off by hand or for the night stays off.
   */
  void Init();

  /*!
   * \brief Starts the animation towards the time `hour`:`minute`.
   *
   * It draws nothing itself. A display that is switched off now stays off through the run.
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
   * A step of the first half switches the display off or on, the first one off, unless the
   * display was already off when the run started; ANIMATION_FLICKER_COUNTER_INIT_VALUE + 1 steps
   * in all. The step after them ends it.
   *
   * It writes the pixels of Display and leaves showing them to Show().
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATIONFLICKER_H_
