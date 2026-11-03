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
/**     \file       AnimationCollapse.cpp
 *      \brief      Letters slide together and the new ones slide apart again
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_COLLAPSE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationCollapse.h"


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
void AnimationCollapse::Init()
{
    Animation::Init(kStateIdle);
    /* all animations share one union, so nothing may rely on member initializers */
    Reset();
} /* Init */


/******************************************************************************************************************************************************
  SetTime()
******************************************************************************************************************************************************/
StdReturnType AnimationCollapse::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == kStateIdle) {
        step_ = 0u;
        direction_ = static_cast<DirectionType>(SelectByTime(hour, minute, kDirectionNumberOfDirections));
        /* the previous time is on the display and is what collapses first. An empty
           display simply finishes that phase on its first task. */
        state_ = kStateClearTime;
        return_value = kEOk;
    }
    return return_value;
} /* SetTime */


/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
void AnimationCollapse::Task()
{
    if(state_ == kStateClearTime) { ClearTimeTask(); }
    else if(state_ == kStateSetTime) { SetTimeTask(); }
} /* Task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Reset()
******************************************************************************************************************************************************/
void AnimationCollapse::Reset()
{
    clock_words_table_.fill(DisplayWords::kWordNone);
    step_ = 0u;
    expand_steps_ = 1u;
    direction_ = kDirectionToLeft;
} /* Reset */


/******************************************************************************************************************************************************
  ClearTimeTask()
******************************************************************************************************************************************************/
void AnimationCollapse::ClearTimeTask()
{
    bool moved{false};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(CollapseRow(row)) { moved = true; }
    }

    /* nothing moved any more, so every row is a solid block at the left edge */
    if(!moved) {
        step_ = 0u;
        expand_steps_ = CalcExpandSteps();
        state_ = kStateSetTime;
    }
} /* ClearTimeTask */


/******************************************************************************************************************************************************
  CollapseRow()
******************************************************************************************************************************************************/
bool AnimationCollapse::CollapseRow(byte row)
{
    bool moved{false};

    /* Scanned towards the target edge on purpose: a pixel that just moved frees the
       place for the next one behind it, so a whole word slides by one column per frame
       instead of collapsing into a single pixel. */
    if(direction_ == kDirectionToLeft) {
        for(byte column = 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(Display::GetInstance().GetPixel(column, row) &&
               !Display::GetInstance().GetPixel(column - 1u, row))
            {
                Display::GetInstance().ClearPixel(column, row);
                Display::GetInstance().SetPixel(column - 1u, row);
                moved = true;
            }
        }
    } else {
        for(int8_t column = DISPLAY_NUMBER_OF_COLUMNS - 2; column >= 0; column--) {
            if(Display::GetInstance().GetPixel(static_cast<byte>(column), row) &&
               !Display::GetInstance().GetPixel(static_cast<byte>(column + 1), row))
            {
                Display::GetInstance().ClearPixel(static_cast<byte>(column), row);
                Display::GetInstance().SetPixel(static_cast<byte>(column + 1), row);
                moved = true;
            }
        }
    }

    return moved;
} /* CollapseRow */


/******************************************************************************************************************************************************
  SetTimeTask()
******************************************************************************************************************************************************/
void AnimationCollapse::SetTimeTask()
{
    Display::GetInstance().Clear();

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        const byte letters_of_row = CountLettersOfRow(row);
        byte letter_index{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(IsPixelPartOfClockWords(clock_words_table_, column, row)) {
                Display::GetInstance().SetPixel(CalcColumn(column, CalcBlockColumn(letter_index, letters_of_row)), row);
                letter_index++;
            }
        }
    }

    /* the last step draws the letters exactly in place, only then it is done */
    if(step_ < expand_steps_) { step_++; }
    else {
        FinishWithClockWords(clock_words_table_);
        Reset();
    }
} /* SetTimeTask */


/******************************************************************************************************************************************************
  CalcColumn()
******************************************************************************************************************************************************/
byte AnimationCollapse::CalcColumn(byte final_column, byte block_column) const
{
    /* Interpolates between the position in the block and the final one, which needs no
       memory of where a letter currently is. Split by direction to stay unsigned: the
       block is left of the final position in one and right of it in the other. */
    const byte remaining_steps = expand_steps_ - step_;

    if(block_column > final_column) {
        return static_cast<byte>(final_column + (((block_column - final_column) * remaining_steps) / expand_steps_));
    }

    return static_cast<byte>(final_column - (((final_column - block_column) * remaining_steps) / expand_steps_));
} /* CalcColumn */


/******************************************************************************************************************************************************
  CalcBlockColumn()
******************************************************************************************************************************************************/
byte AnimationCollapse::CalcBlockColumn(byte letter_index, byte letters_of_row) const
{
    /* Where a letter sits while the row is one solid block. Counting the letters of the
       row up front keeps this a plain calculation, so both directions come out of one
       ascending loop instead of two mirrored ones. */
    if(direction_ == kDirectionToLeft) { return letter_index; }

    return static_cast<byte>(DISPLAY_NUMBER_OF_COLUMNS - letters_of_row + letter_index);
} /* CalcBlockColumn */


/******************************************************************************************************************************************************
  CountLettersOfRow()
******************************************************************************************************************************************************/
byte AnimationCollapse::CountLettersOfRow(byte row) const
{
    byte letters{0u};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(IsPixelPartOfClockWords(clock_words_table_, column, row)) { letters++; }
    }

    return letters;
} /* CountLettersOfRow */


/******************************************************************************************************************************************************
  CalcExpandSteps()
******************************************************************************************************************************************************/
byte AnimationCollapse::CalcExpandSteps() const
{
    byte longest_way{0u};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        const byte letters_of_row = CountLettersOfRow(row);
        byte letter_index{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(IsPixelPartOfClockWords(clock_words_table_, column, row)) {
                const byte block_column = CalcBlockColumn(letter_index, letters_of_row);
                const byte way = (block_column > column) ? (block_column - column) : (column - block_column);
                if(way > longest_way) { longest_way = way; }
                letter_index++;
            }
        }
    }

    /* never zero: CalcColumn() divides by it, and a picture that needs no movement at
       all is then drawn once and done */
    if(longest_way == 0u) { return 1u; }

    return longest_way;
} /* CalcExpandSteps */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
