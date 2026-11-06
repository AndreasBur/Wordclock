// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The explode-words animation: word by word, the old time into the centre, the new one out.
 */

#ifndef _ANIMATION_EXPLODE_WORDS_H_
#define _ANIMATION_EXPLODE_WORDS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"

/*!
 * \brief The explode-words animation: the two halves of AnimationExplodePixels, but word by word:
 *        every word of the previous time travels into the centre on its own, then the words of the
 *        new time come back out one after the other.
 *
 * A word in the centre stands in row kRowCenter, centred on column kColumnCenter. In the first
 * half, while GetState() returns kClearTime, the words of the previous time are the blocks of lit
 * pixels on the display. They travel one after the other, row by row from the centre row
 * outwards, first the centre row, then the row above it, the row below it and so on, and each row
 * from left to right; a word goes out when it has arrived in the centre. In the second half,
 * while it returns kSetTime, the words of the new time start in the centre one after the other,
 * in the order of the clock's word list, and each travels to its place.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationExplodeWords : public Animation
{
  public:
    /*! \brief The column of the centre, DISPLAY_NUMBER_OF_COLUMNS / 2. */
    static constexpr byte kColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    /*! \brief The row of the centre, DISPLAY_NUMBER_OF_ROWS / 2. */
    static constexpr byte kRowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte clock_words_table_index_{0u};
    byte current_word_index_{0u};
    byte current_word_length_{0u};
    byte current_column_{0u};
    byte current_row_{0u};

    void Reset();
    void ClearTimeTask();
    void StartSetTime();
    void SetTimeTask();
    void SetWordToSet(DisplayWords::WordIdType word_id);
    StdReturnType SetNextWordToClear();
    StdReturnType SetNextWordToClearInColumn();
    StdReturnType SetNextRowToClear();
    StdReturnType SetNextWordToSet();
    void SetWordIndex(DisplayWords::WordIdType word_id);
    void SetWordLength();
    StdReturnType ShiftWord(byte final_colum, byte final_row);
    void ToggleWordOnDisplay() const;
    void SetWordOnDisplay() const;

    //byte getFinalIndex() const { return Display::GetInstance().ColumnAndRowToIndex(getFinalColumn(), getFinalRow()); }
    static byte GetClearFinalRow() { return kRowCenter; }
    byte GetSetFinalRow() const { return Display::GetInstance().IndexToRow(current_word_index_); }
    byte GetClearFinalColumn() const { return kColumnCenter - (current_word_length_ / 2u); }
    byte GetSetFinalColumn() const { return Display::GetInstance().IndexToColumn(current_word_index_); }

    byte GetNextColumn(byte final_column) const {
        byte next_column = current_column_;
        if(current_column_ < final_column) { next_column++; }
        if(current_column_ > final_column) { next_column--; }
        return next_column;
    }

    byte GetNextRow(byte final_row) const {
        byte next_row = current_row_;
        if(current_row_ < final_row) { next_row++; }
        if(current_row_ > final_row) { next_row--; }
        return next_row;
    }

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationExplodeWords() : words_() { }

    /*! \brief Destroys the animation. */
    ~AnimationExplodeWords() { }

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself, unless the display is dark: it then starts with the second half at
     * once and lights the first word of the new time in the centre.
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
     * A step moves the word that travels one column and one row towards where it goes, each only
     * as far as needed. In the first half, the step after the word has arrived in the centre turns
     * it off and picks the next; when none is left, the same step lights the first word of the
     * new time in the centre and starts the second half. In the second half, the step after the
     * word has arrived in its place lights the next word in the centre, and the step after the
     * last one has arrived ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif
