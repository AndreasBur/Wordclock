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
/**     \file       AnimationCube.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CUBE_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationCube.h"
#include "StandardTypes.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
void AnimationCube::Init()
{
    Animation::Init(kStateIdle);
    Reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == kStateIdle) {
        return_value = kEOk;
        SetMaxBorder();
        state_ = kStateClearTime;
    }
    return return_value;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationCube::Task()
{
    if(state_ == kStateClearTime) { ClearTimeTask(); }
    else if(state_ == kStateSetTime) { SetTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationCube::Reset()
{
    clock_words_table_.fill(DisplayWords::kWordNone);
    SetMinBorder();
} /* reset */


/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationCube::SetStateToSetTime()
{
    ClearBorderPixels();
    state_ = kStateSetTime;
    SetMinBorder();
} /* setStateToSetTime */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationCube::ClearTimeTask()
{
    switch(border_state_)
    {
        case kBorderStateMax:
            SetBorderPixels();
            border_state_ = kBorderStateBetween;
            break;
        case kBorderStateBetween:
            ClearBorderPixels();
            if(DecreaseBorder() == kENotOk) { SetStateToSetTime(); } 
            else { SetBorderPixels(); }
            break;
        default:
            break;
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationCube::SetTimeTask()
{
    switch(border_state_)
    {
        case kBorderStateMin:
            SetBorderPixels();
            border_state_ = kBorderStateBetween;
            break;
        case kBorderStateBetween:
            ClearBorderPixelsWithoutClockPixels();
            if(IncreaseBorder() == kENotOk) { FinishWithClockWords(clock_words_table_); }
            else { SetBorderPixels(); }
            break;
        default:
            break;
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  writeBorderPixels()
******************************************************************************************************************************************************/
void AnimationCube::ClearBorderPixelsWithoutClockPixels()
{
    for(byte column = border_.column_start; column <= border_.column_end; column++) {
        if(IsPixelPartOfClockWords(clock_words_table_, column, border_.row_start) == false) {
            // clear border top
            Display::GetInstance().ClearPixel(column, border_.row_start);
        }
        if(IsPixelPartOfClockWords(clock_words_table_, column, border_.row_end) == false) {
            // clear border bottom
            Display::GetInstance().ClearPixel(column, border_.row_end);
        }
    }
    for(byte row = border_.row_start; row <= border_.row_end; row++) {
        if(IsPixelPartOfClockWords(clock_words_table_, border_.column_start, row) == false) {
            // clear border left
            Display::GetInstance().ClearPixel(border_.column_start, row);
        }
        if(IsPixelPartOfClockWords(clock_words_table_, border_.column_end, row) == false) {
            // clear border right
            Display::GetInstance().ClearPixel(border_.column_end, row);
        }
    }
} /* writeBorderPixels */


/******************************************************************************************************************************************************
  writeBorderPixels()
******************************************************************************************************************************************************/
void AnimationCube::WriteBorderPixels(bool value) const
{
    for(byte column = border_.column_start; column <= border_.column_end; column++) {
        // set border top
        Display::GetInstance().WritePixel(column, border_.row_start, value);
        // set border bottom
        Display::GetInstance().WritePixel(column, border_.row_end, value);
    }
    for(byte row = border_.row_start; row <= border_.row_end; row++) {
        // set border left
        Display::GetInstance().WritePixel(border_.column_start, row, value);
        // set border right
        Display::GetInstance().WritePixel(border_.column_end, row, value);
    }
} /* writeBorderPixels */


/******************************************************************************************************************************************************
  increaseBorder()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::IncreaseBorder()
{
    StdReturnType return_value = kEOk;

    if(border_.column_start > 0u) { border_.column_start--; }
    else { return_value = kENotOk; }
    if(border_.column_end < DISPLAY_NUMBER_OF_COLUMNS - 1) { border_.column_end++; }
    else { return_value = kENotOk; }
    if(border_.row_start > 0u) { border_.row_start--; }
    else { return_value = kENotOk; }
    if(border_.row_end < DISPLAY_NUMBER_OF_ROWS - 1) { border_.row_end++; }
    else { return_value = kENotOk; }

    return return_value;
} /* increaseBorder */


/******************************************************************************************************************************************************
  decreaseBorder()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::DecreaseBorder()
{
    StdReturnType return_value = kEOk;

    if(border_.column_start < ANIMATION_CUBE_COLUMN_START_MAX_VALUE) { border_.column_start++; }
    else { return_value = kENotOk; }
    if(border_.column_end > ANIMATION_CUBE_COLUMN_END_MIN_VALUE) {border_. column_end--; }
    else { return_value = kENotOk; }
    if(border_.row_start < ANIMATION_CUBE_ROW_START_MAX_VALUE) { border_.row_start++; }
    else { return_value = kENotOk; }
    if(border_.row_end > ANIMATION_CUBE_ROW_END_MIN_VALUE) { border_.row_end--; }
    else { return_value = kENotOk; }

    return return_value;
} /* decreaseBorder */


/******************************************************************************************************************************************************
  setMaxBorder()
******************************************************************************************************************************************************/
void AnimationCube::SetMaxBorder()
{
    border_.column_start = 0u;
    border_.column_end = DISPLAY_NUMBER_OF_COLUMNS - 1u;
    border_.row_start = 0u;
    border_.row_end = DISPLAY_NUMBER_OF_ROWS - 1u;
    border_state_ = kBorderStateMax;
} /* setMaxBorder */


/******************************************************************************************************************************************************
  setMinBorder()
******************************************************************************************************************************************************/
void AnimationCube::SetMinBorder()
{
    border_.column_start = ANIMATION_CUBE_COLUMN_START_MAX_VALUE;
    border_.column_end = ANIMATION_CUBE_COLUMN_END_MIN_VALUE;
    border_.row_start = ANIMATION_CUBE_ROW_START_MAX_VALUE;
    border_.row_end = ANIMATION_CUBE_ROW_END_MIN_VALUE;
    border_state_ = kBorderStateMin;
} /* setMinBorder */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
