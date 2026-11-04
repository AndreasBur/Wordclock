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
/**     \file       AnimationRoll.cpp
 *      \brief      Rolls the previous time out while the new one rolls in
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_ROLL_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationRoll.h"


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
void AnimationRoll::Init()
{
    Animation::Init(StateType::kIdle);
    /* all animations share one union, so nothing may rely on member initializers */
    Reset();
} /* Init */


/******************************************************************************************************************************************************
  SetTime()
******************************************************************************************************************************************************/
StdReturnType AnimationRoll::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == StateType::kIdle) {
        direction_ = CalcDirection(hour, minute);
        step_ = 0u;
        // no clear phase: the previous time is still on the display and rolls out
        // while the new one rolls in behind it
        state_ = StateType::kSetTime;
        return_value = kEOk;
    }
    return return_value;
} /* SetTime */


/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
void AnimationRoll::Task()
{
    if(state_ == StateType::kSetTime) { SetTimeTask(); }
} /* Task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Reset()
******************************************************************************************************************************************************/
void AnimationRoll::Reset()
{
    clock_words_table_.fill(DisplayWords::WordIdType::kNone);
    step_ = 0u;
    direction_ = DirectionType::kRight;
} /* Reset */


/******************************************************************************************************************************************************
  SetTimeTask()
******************************************************************************************************************************************************/
void AnimationRoll::SetTimeTask()
{
    if(step_ < NumberOfSteps()) {
        if(direction_ == DirectionType::kRight || direction_ == DirectionType::kLeft) { RollHorizontal(); }
        else { RollVertical(); }
        step_++;
    } else {
        /* the new time has reached its place, nothing left to shift */
        FinishWithClockWords(clock_words_table_);
        Reset();
    }
} /* SetTimeTask */


/******************************************************************************************************************************************************
  RollHorizontal()
******************************************************************************************************************************************************/
void AnimationRoll::RollHorizontal()
{
    /*
     * Shift everything by one and feed the column of the new time that belongs in at
     * the freed edge. The remaining shifts carry it to its final column, so the
     * column entering in step n is the one that ends up n columns away from the edge
     * it entered at.
     */
    if(direction_ == DirectionType::kRight) {
        wc_transformation_.ShiftRight();
        for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
            if(IsPixelPartOfClockWords(clock_words_table_, DISPLAY_NUMBER_OF_COLUMNS - step_ - 1u, row)) {
                Display::GetInstance().SetPixel(0u, row);
            }
        }
    } else {
        wc_transformation_.ShiftLeft();
        for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
            if(IsPixelPartOfClockWords(clock_words_table_, step_, row)) {
                Display::GetInstance().SetPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row);
            }
        }
    }
} /* RollHorizontal */


/******************************************************************************************************************************************************
  RollVertical()
******************************************************************************************************************************************************/
void AnimationRoll::RollVertical()
{
    /* same as RollHorizontal(), with rows instead of columns */
    if(direction_ == DirectionType::kDown) {
        wc_transformation_.ShiftDown();
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(IsPixelPartOfClockWords(clock_words_table_, column, DISPLAY_NUMBER_OF_ROWS - step_ - 1u)) {
                Display::GetInstance().SetPixel(column, 0u);
            }
        }
    } else {
        wc_transformation_.ShiftUp();
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(IsPixelPartOfClockWords(clock_words_table_, column, step_)) {
                Display::GetInstance().SetPixel(column, DISPLAY_NUMBER_OF_ROWS - 1u);
            }
        }
    }
} /* RollVertical */


/******************************************************************************************************************************************************
  NumberOfSteps()
******************************************************************************************************************************************************/
byte AnimationRoll::NumberOfSteps() const
{
    if(direction_ == DirectionType::kRight || direction_ == DirectionType::kLeft) { return DISPLAY_NUMBER_OF_COLUMNS; }

    return DISPLAY_NUMBER_OF_ROWS;
} /* NumberOfSteps */


/******************************************************************************************************************************************************
  CalcDirection()
******************************************************************************************************************************************************/
AnimationRoll::DirectionType AnimationRoll::CalcDirection(byte hour, byte minute)
{
    return static_cast<DirectionType>(SelectByTime(hour, minute, kNumberOfDirections));
} /* CalcDirection */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
