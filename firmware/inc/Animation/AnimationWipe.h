// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The wipe animation: a diagonal line sweeps across the display and uncovers the new time.
 */

#ifndef _ANIMATION_WIPE_H_
#define _ANIMATION_WIPE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"

/*!
 * \brief The wipe animation: a diagonal line sweeps across the display and uncovers the new
 *        letters.
 *
 * The line runs from top right to bottom left and sweeps from the top left corner to the bottom
 * right one, one pixel further per step: along the top row, then down the rightmost column. It
 * sweeps twice. In the first half, while GetState() returns kClearTime, it pushes the previous
 * time ahead of it and off the display. In the second half, while it returns kSetTime, it lights
 * the letters of the new time.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationWipe : public Animation
{
  public:
    /*! \brief Where the first half moves a lit pixel on the line. */
    enum class SetPixelStateType {
        kRight, /*!< To the first dark pixel right of it in its row. */
        kDown   /*!< To the first dark pixel below it in its column. */
    };

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte index_{0u};
    SetPixelStateType set_pixel_state_{SetPixelStateType::kDown};

    void Reset();
    void ClearTimeTask();
    void SetTimeTask();
    StdReturnType SetNextIndex();
    static void SetPixelDown(byte column, byte row);
    static void SetPixelRight(byte column, byte row);

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationWipe() { }

    /*! \brief Destroys the animation. */
    ~AnimationWipe() = default;

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
     * In the first half a step moves each lit pixel on the line to the first dark pixel below it,
     * or, every other step, right of it, as SetPixelStateType says; a pixel with none left before
     * the edge goes out. In the second half a step lights the pixels of the new time on the line.
     * Each half takes DISPLAY_NUMBER_OF_COLUMNS + DISPLAY_NUMBER_OF_ROWS - 1 steps, one per
     * line; the last step of the first starts the second, and the last of the second ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
