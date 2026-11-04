// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The flicker animation: the display flickers a few times, then the new time is there.
 */

#ifndef _ANIMATION_FLICKER_H_
#define _ANIMATION_FLICKER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Animation.h"

/*!
 * \brief One less than the switches of the flicker; the display is switched off and on this many
 *        times and once more, starting with off.
 */
#define ANIMATION_FLICKER_COUNTER_INIT_VALUE          4u

/*!
 * \brief The flicker animation: the display flickers a few times, then the new time is there.
 *
 * In the first half, while GetState() returns kClearTime, it switches the display off and on,
 * through Display::Disable() and Display::Enable(), while the display keeps the previous time.
 * The second half, while it returns kSetTime, is one step.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle; it also enables the display.
 *
 * \warning It enables the display in Init(), at every other step of the flicker and at its end,
 *          whether the display was enabled before or not. A display that was switched off, by hand
 *          or by NightSwitch, lights up again when the flicker is selected or runs at a word
 *          change.
 */
class AnimationFlicker : public Animation
{
  public:

  private:
    byte minute_{0u};
    byte hour_{0u};
    bool is_clock_set_{false};
    byte flicker_counter_{ANIMATION_FLICKER_COUNTER_INIT_VALUE};

    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationFlicker() { }

    /*! \brief Destroys the animation. */
    ~AnimationFlicker() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     * It also enables the display, so that a flicker cut short does not leave it switched off.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself. It does not refuse while it runs: a running flicker goes on with
     * the switches it has left, and ends on the new time.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59; the animation is then
     *         left as it was.
     */
    StdReturnType SetTime(byte hour, byte minute);

    /*!
     * \brief Takes one step of the animation; does nothing while it is idle.
     *
     * A step of the first half switches the display off or on, the first one off;
     * ANIMATION_FLICKER_COUNTER_INIT_VALUE + 1 steps in all. The step after them ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();

};

#endif
