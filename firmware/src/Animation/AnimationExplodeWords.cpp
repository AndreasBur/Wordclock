/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       AnimationExplodeWords.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_EXPLODE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationExplodeWords.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
void AnimationExplodeWords::Init()
{
    Animation::Init(kStateIdle);
    Reset();
} /* init */

/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == kStateIdle) {
        return_value = kEOk;
        if(SetNextWordToClear() == kEOk) { state_ = kStateClearTime; }
        else { state_ = kStateSetTime; }
    }
    return return_value;
} /* setTime */

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationExplodeWords::Task()
{
    if(state_ == kStateClearTime) ClearTimeTask();
    else if(state_ == kStateSetTime) SetTimeTask();
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationExplodeWords::Reset()
{
    clock_words_table_.fill(DisplayWords::kWordNone);
    clock_words_table_index_ = 0u;
    current_word_index_ = Display::GetInstance().ColumnAndRowToIndex(0u, kRowCenter);
    current_word_length_ = 0u;
    current_column_ = 0u;
    current_row_ = 0u;
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationExplodeWords::ClearTimeTask()
{
    if(ShiftWord(GetClearFinalColumn(), GetClearFinalRow()) == kENotOk) {
        ToggleWordOnDisplay();
        if(SetNextWordToClear() == kENotOk) {
            current_word_index_ = 0u;
            SetWordToSet(clock_words_table_[current_word_index_]);
            SetWordOnDisplay();
            state_ = kStateSetTime;
        }
    }
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationExplodeWords::SetTimeTask()
{
    if(ShiftWord(GetSetFinalColumn(), GetSetFinalRow()) == kENotOk) {
        if(SetNextWordToSet() == kENotOk) { FinishWithClockWords(clock_words_table_); }
        else { ToggleWordOnDisplay(); }
    }
} /* setTimeTask */

/******************************************************************************************************************************************************
  shiftWord()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::ShiftWord(byte final_colum, byte final_row)
{
    if(current_column_ != GetNextColumn(final_colum) || current_row_ != GetNextRow(final_row)) {
        ToggleWordOnDisplay();
        current_column_ = GetNextColumn(final_colum);
        current_row_ = GetNextRow(final_row);
        ToggleWordOnDisplay();
        return kEOk;
    } else {
        return kENotOk;
    }
} /* shiftWord */

/******************************************************************************************************************************************************
  toggleWordOnDisplay()
******************************************************************************************************************************************************/
void AnimationExplodeWords::ToggleWordOnDisplay() const
{
    for(byte column_index = current_column_; column_index < current_column_ + current_word_length_; column_index++) {
        Display::GetInstance().TogglePixel(column_index, current_row_);
    }
} /* toggleWordOnDisplay */

/******************************************************************************************************************************************************
  setWordOnDisplay()
******************************************************************************************************************************************************/
void AnimationExplodeWords::SetWordOnDisplay() const
{
    for(byte column_index = current_column_; column_index < current_column_ + current_word_length_; column_index++) {
        Display::GetInstance().SetPixel(column_index, current_row_);
    }
} /* setWordOnDisplay */

/******************************************************************************************************************************************************
  setNextWordToClear()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::SetNextWordToClear()
{
    current_column_ = Display::GetInstance().IndexToColumn(current_word_index_) + current_word_length_;
    current_row_ = Display::GetInstance().IndexToRow(current_word_index_);

    do {
        if(SetNextWordToClearInColumn() == kEOk) { return kEOk; }
        current_column_ = 0;
    } while(SetNextRowToClear() == kEOk);

    return kENotOk;
} /* setNextWordToClear */

/******************************************************************************************************************************************************
  setNextWordToClearInColumn()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::SetNextWordToClearInColumn()
{
    for(; current_column_ < DISPLAY_NUMBER_OF_COLUMNS; current_column_++) {
        if(Display::GetInstance().GetPixel(current_column_, current_row_)) {
            current_word_index_ = Display::GetInstance().ColumnAndRowToIndex(current_column_, current_row_);
            SetWordLength();
            return kEOk;
        }
    }
    return kENotOk;
} /* setNextWordToClearInColumn */

/******************************************************************************************************************************************************
  setRowNextToClear()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::SetNextRowToClear()
{
    if(current_row_ > 0 && current_row_ < DISPLAY_NUMBER_OF_ROWS) {
        if(current_row_ >= kRowCenter) { current_row_ = DISPLAY_NUMBER_OF_ROWS - current_row_ - 1u; }
        else { current_row_ = DISPLAY_NUMBER_OF_ROWS - current_row_; }
        return kEOk;
    } else {
        return kENotOk;
    }
} /* setRowNextToClear */

/******************************************************************************************************************************************************
  setNextWordToSet()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodeWords::SetNextWordToSet()
{
    for(byte index = clock_words_table_index_ + 1u; index < clock_words_table_.size(); index++) {
        if(clock_words_table_[index] != DisplayWords::kWordNone) {
            clock_words_table_index_ = index;
            SetWordToSet(clock_words_table_[clock_words_table_index_]);
            return kEOk;
        }
    }
    return kENotOk;
} /* setNextWordToSet */

/******************************************************************************************************************************************************
  setWordToSet()
******************************************************************************************************************************************************/
void AnimationExplodeWords::SetWordToSet(DisplayWords::WordIdType word_id)
{
    SetWordIndex(word_id);
    current_word_length_ = words_.GetDisplayWordLength(word_id);
    current_column_ = GetClearFinalColumn();
    current_row_ = GetClearFinalRow();
} /* setWordToSet */

/******************************************************************************************************************************************************
  setWordIndex()
******************************************************************************************************************************************************/
void AnimationExplodeWords::SetWordIndex(DisplayWords::WordIdType word_id)
{
    byte column = words_.GetDisplayWordColumn(word_id);
    byte row = words_.GetDisplayWordRow(word_id);
    current_word_index_ = Display::GetInstance().ColumnAndRowToIndex(column, row);
} /* setWordIndex */

/******************************************************************************************************************************************************
  setWordLength()
******************************************************************************************************************************************************/
void AnimationExplodeWords::SetWordLength()
{
    current_word_length_ = 1u;

    for(byte index = current_word_index_ + 1u; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
        if(Display::GetInstance().GetPixel(index)) {
            current_word_length_++;
        } else {
            break;
        }
    }
} /* setWordLength */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
