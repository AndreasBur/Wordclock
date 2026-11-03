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
/**     \file       AnimationExplodePixels.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_IMPLODE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationExplodePixels.h"


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
void AnimationExplodePixels::Init()
{
    Animation::Init(kStateIdle);
    Reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationExplodePixels::SetTime(byte hour, byte minute)
{
    StdReturnType return_value{kENotOk};

    if(Clock::GetInstance().GetClockWords(hour, minute, clock_words_table_) == kEOk && state_ == kStateIdle) {
        return_value = kEOk;
        Display::GetInstance().SetPixel(kColumnCenter, kRowCenter);
        shift_counter_ = 0u;
        state_ = kStateClearTime;
    }
    return return_value;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationExplodePixels::Task()
{
    if(state_ == kStateClearTime) {
        if(shift_counter_ < kShiftCounterMaxValue) {
            shift_counter_++;
            ClearTimeTask();
        } else {
            SetStateToSetTime();
        }
    }
    if(state_ == kStateSetTime) {
        if(shift_counter_ >= 2u) {
            shift_counter_--;
            SetTimeTask();
        } else {
            FinishWithClockWords(clock_words_table_);
        }
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationExplodePixels::Reset()
{
    shift_counter_ = 0u;
    clock_words_table_.fill(DisplayWords::kWordNone);
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ClearTimeTask()
{
    ShiftQuadrantUpperLeft();
    ShiftQuadrantLowerRight();
    ShiftQuadrantUpperRight();
    ShiftQuadrantLowerLeft();
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationExplodePixels::SetTimeTask()
{
    DisplayPixels pixels;
    Display::GetInstance().Clear();
    SetTime(pixels);
    ShiftQuadrants(pixels, shift_counter_);
    Display::GetInstance().SetPixel(kColumnCenter, kRowCenter);
} /* setTimeTask */

/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationExplodePixels::SetStateToSetTime()
{
    state_ = kStateSetTime;
    Display::GetInstance().Clear();
    shift_counter_ = kShiftCounterMaxValue;
} /* setStateToSetTime */

/******************************************************************************************************************************************************
  shiftQuadrants()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftQuadrants(DisplayPixels& pixels, byte number_of_shifts)
{
    for(uint8_t row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        for(uint8_t column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(pixels.GetPixel(column, row)) {
                byte column_next = column, row_next = row;
                if(column < kColumnCenter) column_next = ShiftRight(column, number_of_shifts);
                if(column > kColumnCenter) column_next = ShiftLeft(column, number_of_shifts);
                if(row < kRowCenter) row_next = ShiftDown(row, number_of_shifts);
                if(row > kRowCenter) row_next = ShiftUp(row, number_of_shifts);
                SetNewPixel(column, row, column_next, row_next);
            }
        }
    }
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperLeft()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftQuadrantUpperLeft()
{
    for(int8_t column = kColumnCenter; column >= 0; column--) {
        for(int8_t row = kRowCenter; row >= 0; row--) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftDownRight(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownRight()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftDownRight(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column < kColumnCenter) column_next++;
    if(row < kColumnCenter) row_next++;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperRight()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftQuadrantUpperRight()
{
    for(uint8_t column = kColumnCenter + 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = kRowCenter - 1u; row >= 0; row--) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftDownLeft(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownLeft()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftDownLeft(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column > kColumnCenter) column_next--;
    if(row < kColumnCenter) row_next++;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerLeft()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftQuadrantLowerLeft()
{
    for(int8_t column = kColumnCenter - 1u; column >= 0; column--) {
        for(int8_t row = kRowCenter + 1u; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftUpRight(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpRight()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftUpRight(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column < kColumnCenter) column_next++;
    if(row > kColumnCenter) row_next--;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerRight()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftQuadrantLowerRight()
{
    for(uint8_t column = kColumnCenter; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = kRowCenter; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::GetInstance().GetPixel(column, row)) { ShiftUpLeft(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpLeft()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ShiftUpLeft(byte column, byte row)
{
    byte column_next = column, row_next = row;

    if(column > kColumnCenter) column_next--;
    if(row > kColumnCenter) row_next--;
    ClearOldAndSetNewPixel(column, row, column_next, row_next);
}

/******************************************************************************************************************************************************
  clearOldAndSetNewPixel()
******************************************************************************************************************************************************/
void AnimationExplodePixels::ClearOldAndSetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new)
{
    if((column_old != column_new) || (row_old != row_new))
    {
        Display::GetInstance().ClearPixel(column_old, row_old);
        Display::GetInstance().SetPixel(column_new, row_new);
    }
}

/******************************************************************************************************************************************************
  setNewPixel()
******************************************************************************************************************************************************/
void AnimationExplodePixels::SetNewPixel(byte column_old, byte row_old, byte column_new, byte row_new)
{
    if((column_old != column_new) || (row_old != row_new))
    {
        Display::GetInstance().SetPixel(column_new, row_new);
    }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
