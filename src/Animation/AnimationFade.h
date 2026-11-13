// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The fade animation: the display dims to dark, the time changes, and it fades back up.
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATIONFADE_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATIONFADE_H_

#include "Animation.h"
#include "Arduino.h"
#include "StandardTypes.h"

/*!
 * \brief The fade animation: dims the display down to dark, changes the time, and fades it back up.
 *
 * It dims through the display's fade level, Display::SetBrightnessFadeLevel(), which scales
 * whatever brightness the display is set to; the brightness setting stays as it is. In the first
 * half, while GetState() returns kClearTime, the level falls from 255 to 0. The time changes
 * while the display is dark, and in the second half, while GetState() returns kSetTime, the level
 * rises back to 255.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step ends the fade's dimming, through Display::ClearBrightnessFade(), clears the display,
 * lights the words of the new time whole and makes the animation idle.
 */
class AnimationFade : public Animation {
 private:
  /*
   * How far the fade has got, in what Display calls a fade level: 255 is the display as
   * configured, 0 is dark. Counted here rather than read back from the display, which
   * scales the level by the setting, the gamma correction and the automatic.
   */
  static constexpr byte kLevelFull{255u};
  byte level_{kLevelFull};
  byte hour_{0u};
  byte minute_{0u};

  void Reset();
  void ClearTimeTask();
  void SetTimeTask();
  void SetStateToSetTime();

 public:
  /*! \brief Makes the animation, with GetState() kNone until Init(). */
  constexpr AnimationFade() {}

  /*! \brief Destroys the animation. */
  ~AnimationFade() = default;

  /*!
   * \brief Makes the animation idle and resets its progress.
   *
   * A running animation ends where it stands, and the display keeps what its last step drew.
   * It also ends the fade's dimming, through Display::ClearBrightnessFade(): a fade cut off half
   * way would otherwise leave the display dimmed with nothing left to undim it.
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
   * A step of the first half lowers the fade level by one, 255 steps from 255 to 0. The step
   * after them clears the display, lights the words of the new time and starts the second half.
   * A step of the second half raises the level by one, 255 steps back to 255, and the step after
   * them ends it.
   *
   * It writes the pixels of Display and leaves showing them to Show(). It applies the fade
   * level at once.
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATIONFADE_H_
