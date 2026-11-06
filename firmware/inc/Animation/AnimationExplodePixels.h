// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The explode-pixels animation: the old time is pulled into the centre, the new one expands.
 */

#ifndef _ANIMATION_EXPLODE_PIXELS_H_
#define _ANIMATION_EXPLODE_PIXELS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"
#include "DisplayPixels.h"
#include "Display.h"

/*!
 * \brief The explode-pixels animation: the previous time is pulled into the centre, all of its
 *        pixels at once, until only the centre is lit; then the new time expands out of the centre
 *        the same way.
 *
 * The centre is the pixel at column kColumnCenter and row kRowCenter. In the first half, while
 * GetState() returns kClearTime, every lit pixel moves one column and one row towards it per step,
 * each only as far as the centre column and the centre row; after kShiftCounterMaxValue steps
 * every pixel of the grid has arrived. In the second half, while it returns kSetTime, each step
 * draws the new time anew, one column and one row closer to its place: every pixel the same count
 * of columns and rows short of it, towards the centre, and the centre lit.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationExplodePixels : public Animation
{
  public:
    /*! \brief The steps of the first half, each one column and one row towards the centre. */
    static constexpr byte kShiftCounterMaxValue{5u};
    /*! \brief The column of the centre, DISPLAY_NUMBER_OF_COLUMNS / 2. */
    static constexpr byte kColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    /*! \brief The row of the centre, DISPLAY_NUMBER_OF_ROWS / 2. */
    static constexpr byte kRowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    DisplayWords words_;
    byte shift_counter_{0u};

    void Reset();
    static void ClearTimeTask();
    void SetTimeTask();
    void SetStateToSetTime();

    static void ShiftQuadrants(DisplayPixels& pixels, byte number_of_shifts);

    static void ShiftQuadrantUpperLeft();
    static void ShiftQuadrantUpperRight();
    static void ShiftQuadrantLowerLeft();
    static void ShiftQuadrantLowerRight();

    static void ShiftDownRight(byte column, byte row);
    static void ShiftDownLeft(byte column, byte row);
    static void ShiftUpRight(byte column, byte row);
    static void ShiftUpLeft(byte column, byte row);

    static void ShiftIfLit(byte column, byte row, void (*shift)(byte column, byte row));
    static void ClearOldAndSetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new);

    // The quadrant loops count down past 0 to end, so their variables are signed. Inside the
    // loop they never are negative, which is what makes this exact.
    static constexpr byte ToCoordinate(int8_t value) { return static_cast<byte>(value); }
    static void SetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new);

    static byte ShiftLeft(byte column, byte number_of_shifts) {
         return (static_cast<int16_t>(column) - number_of_shifts > kColumnCenter) ? static_cast<byte>(column - number_of_shifts) : kColumnCenter;
    }
    static byte ShiftRight(byte column, byte number_of_shifts) {
         return (static_cast<uint16_t>(column) + number_of_shifts < kColumnCenter) ? static_cast<byte>(column + number_of_shifts) : kColumnCenter;
    }
    static byte ShiftUp(byte row, byte number_of_shifts) {
         return (static_cast<int16_t>(row) - number_of_shifts > kRowCenter) ? static_cast<byte>(row - number_of_shifts) : kRowCenter;
    }
    static byte ShiftDown(byte row, byte number_of_shifts) {
         return (static_cast<uint16_t>(row) + number_of_shifts < kRowCenter) ? static_cast<byte>(row + number_of_shifts) : kRowCenter;
    }

    void SetTime(DisplayPixels& pixels) {
        for(byte index  = 0u; index < clock_words_table_.size(); index++) {
            pixels.SetWord(clock_words_table_[index]);
        }
    }

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationExplodePixels() : words_() { }

    /*! \brief Destroys the animation. */
    ~AnimationExplodePixels() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It lights the centre pixel.
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
     * A step of the first half moves every lit pixel one column and one row towards the centre;
     * kShiftCounterMaxValue steps in all. The step after them clears the display, starts the
     * second half and draws the new time kShiftCounterMaxValue - 1 columns and rows short of its
     * place. Each further step draws it one column and one row closer, and the step after the one
     * that draws it a column and a row short ends it: 2 * kShiftCounterMaxValue steps in all.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
