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
  init()
******************************************************************************************************************************************************/
void AnimationCube::init()
{
    AnimationCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        setMaxBorder();
        State = STATE_CLEAR_TIME;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationCube::task()
{
    if(State == STATE_CLEAR_TIME) { clearTimeTask(); }
    else if(State == STATE_SET_TIME) { setTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationCube::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    setMinBorder();
} /* reset */


/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationCube::setStateToSetTime()
{
    clearBorderPixels();
    State = STATE_SET_TIME;
    setMinBorder();
} /* setStateToSetTime */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationCube::clearTimeTask()
{
    switch(BorderState)
    {
        case BORDER_STATE_MAX:
            setBorderPixels();
            BorderState = BORDER_STATE_BETWEEN;
            break;
        case BORDER_STATE_BETWEEN:
            clearBorderPixels();
            if(decreaseBorder() == E_NOT_OK) { setStateToSetTime(); } 
            else { setBorderPixels(); }
            break;
        default:
            break;
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationCube::setTimeTask()
{
    switch(BorderState)
    {
        case BORDER_STATE_MIN:
            setBorderPixels();
            BorderState = BORDER_STATE_BETWEEN;
            break;
        case BORDER_STATE_BETWEEN:
            clearBorderPixelsWithoutClockPixels();
            if(increaseBorder() == E_NOT_OK) { State = STATE_IDLE; } 
            else { setBorderPixels(); }
            break;
        default:
            break;
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  writeBorderPixels()
******************************************************************************************************************************************************/
void AnimationCube::clearBorderPixelsWithoutClockPixels()
{
    for(byte Column = Border.ColumnStart; Column <= Border.ColumnEnd; Column++) {
        if(isPixelPartOfClockWords(ClockWordsTable, Column, Border.RowStart) == false) {
            // clear border top
            Display::getInstance().clearPixelFast(Column, Border.RowStart);
        }
        if(isPixelPartOfClockWords(ClockWordsTable, Column, Border.RowEnd) == false) {
            // clear border bottom
            Display::getInstance().clearPixelFast(Column, Border.RowEnd);
        }
    }
    for(byte Row = Border.RowStart; Row <= Border.RowEnd; Row++) {
        if(isPixelPartOfClockWords(ClockWordsTable, Border.ColumnStart, Row) == false) {
            // clear border left
            Display::getInstance().clearPixelFast(Border.ColumnStart, Row);
        }
        if(isPixelPartOfClockWords(ClockWordsTable, Border.ColumnEnd, Row) == false) {
            // clear border right
            Display::getInstance().clearPixelFast(Border.ColumnEnd, Row);
        }
    }
} /* writeBorderPixels */


/******************************************************************************************************************************************************
  writeBorderPixels()
******************************************************************************************************************************************************/
void AnimationCube::writeBorderPixels(bool Value)
{
    for(byte Column = Border.ColumnStart; Column <= Border.ColumnEnd; Column++) {
        // set border top
        Display::getInstance().writePixelFast(Column, Border.RowStart, Value);
        // set border bottom
        Display::getInstance().writePixelFast(Column, Border.RowEnd, Value);
    }
    for(byte Row = Border.RowStart; Row <= Border.RowEnd; Row++) {
        // set border left
        Display::getInstance().writePixelFast(Border.ColumnStart, Row, Value);
        // set border right
        Display::getInstance().writePixelFast(Border.ColumnEnd, Row, Value);
    }
} /* writeBorderPixels */


/******************************************************************************************************************************************************
  increaseBorder()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::increaseBorder()
{
    StdReturnType returnValue = E_OK;

    if(Border.ColumnStart > 0u) { Border.ColumnStart--; }
    else { returnValue = E_NOT_OK; }
    if(Border.ColumnEnd < DISPLAY_NUMBER_OF_COLUMNS - 1) { Border.ColumnEnd++; }
    else { returnValue = E_NOT_OK; }
    if(Border.RowStart > 0u) { Border.RowStart--; }
    else { returnValue = E_NOT_OK; }
    if(Border.RowEnd < DISPLAY_NUMBER_OF_ROWS - 1) { Border.RowEnd++; }
    else { returnValue = E_NOT_OK; }

    return returnValue;
} /* increaseBorder */


/******************************************************************************************************************************************************
  decreaseBorder()
******************************************************************************************************************************************************/
StdReturnType AnimationCube::decreaseBorder()
{
    StdReturnType returnValue = E_OK;

    if(Border.ColumnStart < ANIMATION_CUBE_COLUMN_START_MAX_VALUE) { Border.ColumnStart++; }
    else { returnValue = E_NOT_OK; }
    if(Border.ColumnEnd > ANIMATION_CUBE_COLUMN_END_MIN_VALUE) {Border. ColumnEnd--; }
    else { returnValue = E_NOT_OK; }
    if(Border.RowStart < ANIMATION_CUBE_ROW_START_MAX_VALUE) { Border.RowStart++; }
    else { returnValue = E_NOT_OK; }
    if(Border.RowEnd > ANIMATION_CUBE_ROW_END_MIN_VALUE) { Border.RowEnd--; }
    else { returnValue = E_NOT_OK; }

    return returnValue;
} /* decreaseBorder */


/******************************************************************************************************************************************************
  setMaxBorder()
******************************************************************************************************************************************************/
void AnimationCube::setMaxBorder()
{
    Border.ColumnStart = 0u;
    Border.ColumnEnd = DISPLAY_NUMBER_OF_COLUMNS - 1u;
    Border.RowStart = 0u;
    Border.RowEnd = DISPLAY_NUMBER_OF_ROWS - 1u;
    BorderState = BORDER_STATE_MAX;
} /* setMaxBorder */


/******************************************************************************************************************************************************
  setMinBorder()
******************************************************************************************************************************************************/
void AnimationCube::setMinBorder()
{
    Border.ColumnStart = ANIMATION_CUBE_COLUMN_START_MAX_VALUE;
    Border.ColumnEnd = ANIMATION_CUBE_COLUMN_END_MIN_VALUE;
    Border.RowStart = ANIMATION_CUBE_ROW_START_MAX_VALUE;
    Border.RowEnd = ANIMATION_CUBE_ROW_END_MIN_VALUE;
    BorderState = BORDER_STATE_MIN;
} /* setMinBorder */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
