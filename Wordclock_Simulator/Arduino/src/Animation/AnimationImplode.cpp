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
/**     \file       AnimationImplode.cpp
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
#include "AnimationImplode.h"


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
  init()
******************************************************************************************************************************************************/
void AnimationImplode::init()
{
    Animation::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationImplode::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        Display::getInstance().setPixel(ColumnCenter, RowCenter);
        ShiftCounter = 0u;
        State = STATE_CLEAR_TIME;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationImplode::task()
{
    if(State == STATE_CLEAR_TIME) {
        if(ShiftCounter < ShiftCounterMaxValue) {
            ShiftCounter++;
            clearTimeTask();
        } else {
            setStateToSetTime();
        }
    }
    if(State == STATE_SET_TIME) {
        if(ShiftCounter >= 2u) {
            ShiftCounter--;
            setTimeTask();
        } else {
            Display::getInstance().clear();
            Clock::getInstance().setTime(ClockWordsTable);
            State = STATE_IDLE;
        }
    }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationImplode::reset()
{
    ShiftCounter = 0u;
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationImplode::clearTimeTask()
{
    shiftQuadrantUpperLeft();
    shiftQuadrantLowerRight();
    shiftQuadrantUpperRight();
    shiftQuadrantLowerLeft();
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationImplode::setTimeTask()
{
    DisplayPixels pixels;
    Display::getInstance().clear();
    setTime(pixels);
    shiftQuadrants(pixels, ShiftCounter);
    Display::getInstance().setPixel(ColumnCenter, RowCenter);
} /* setTimeTask */

/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationImplode::setStateToSetTime()
{
    State = STATE_SET_TIME;
    Display::getInstance().clear();
    ShiftCounter = ShiftCounterMaxValue;
} /* setStateToSetTime */

/******************************************************************************************************************************************************
  shiftQuadrants()
******************************************************************************************************************************************************/
void AnimationImplode::shiftQuadrants(DisplayPixels& Pixels, byte NumberOfShifts)
{
    for(uint8_t row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        for(uint8_t column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(Pixels.getPixel(column, row)) {
                byte ColumnNext = column, RowNext = row;
                if(column < ColumnCenter) ColumnNext = shiftRight(column, NumberOfShifts);
                if(column > ColumnCenter) ColumnNext = shiftLeft(column, NumberOfShifts);
                if(row < RowCenter) RowNext = shiftDown(row, NumberOfShifts);
                if(row > RowCenter) RowNext = shiftUp(row, NumberOfShifts);
                setNewPixel(column, row, ColumnNext, RowNext);
            }
        }
    }
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperLeft()
******************************************************************************************************************************************************/
void AnimationImplode::shiftQuadrantUpperLeft()
{
    for(int8_t column = ColumnCenter; column >= 0; column--) {
        for(int8_t row = RowCenter; row >= 0; row--) {
            if(Display::getInstance().getPixelFast(column, row)) { shiftDownRight(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownRight()
******************************************************************************************************************************************************/
void AnimationImplode::shiftDownRight(byte Column, byte Row)
{
    byte columnNext = Column, rowNext = Row;

    if(Column < ColumnCenter) columnNext++;
    if(Row < ColumnCenter) rowNext++;
    clearOldAndSetNewPixel(Column, Row, columnNext, rowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperRight()
******************************************************************************************************************************************************/
void AnimationImplode::shiftQuadrantUpperRight()
{
    for(uint8_t column = ColumnCenter + 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = RowCenter - 1u; row >= 0; row--) {
            if(Display::getInstance().getPixelFast(column, row)) { shiftDownLeft(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownLeft()
******************************************************************************************************************************************************/
void AnimationImplode::shiftDownLeft(byte Column, byte Row)
{
    byte columnNext = Column, rowNext = Row;

    if(Column > ColumnCenter) columnNext--;
    if(Row < ColumnCenter) rowNext++;
    clearOldAndSetNewPixel(Column, Row, columnNext, rowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerLeft()
******************************************************************************************************************************************************/
void AnimationImplode::shiftQuadrantLowerLeft()
{
    for(int8_t column = ColumnCenter - 1u; column >= 0; column--) {
        for(int8_t row = RowCenter + 1u; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::getInstance().getPixelFast(column, row)) { shiftUpRight(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpRight()
******************************************************************************************************************************************************/
void AnimationImplode::shiftUpRight(byte Column, byte Row)
{
    byte columnNext = Column, rowNext = Row;

    if(Column < ColumnCenter) columnNext++;
    if(Row > ColumnCenter) rowNext--;
    clearOldAndSetNewPixel(Column, Row, columnNext, rowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerRight()
******************************************************************************************************************************************************/
void AnimationImplode::shiftQuadrantLowerRight()
{
    for(uint8_t column = ColumnCenter; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        for(int8_t row = RowCenter; row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); row++) {
            if(Display::getInstance().getPixelFast(column, row)) { shiftUpLeft(column, row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpLeft()
******************************************************************************************************************************************************/
void AnimationImplode::shiftUpLeft(byte Column, byte Row)
{
    byte columnNext = Column, rowNext = Row;

    if(Column > ColumnCenter) columnNext--;
    if(Row > ColumnCenter) rowNext--;
    clearOldAndSetNewPixel(Column, Row, columnNext, rowNext);
}

/******************************************************************************************************************************************************
  clearOldAndSetNewPixel()
******************************************************************************************************************************************************/
void AnimationImplode::clearOldAndSetNewPixel(byte ColumnOld, byte RowOld, byte ColumnNew, byte RowNew)
{
    if((ColumnOld != ColumnNew) || (RowOld != RowNew))
    {
        Display::getInstance().clearPixelFast(ColumnOld, RowOld);
        Display::getInstance().setPixelFast(ColumnNew, RowNew);
    }
}

/******************************************************************************************************************************************************
  setNewPixel()
******************************************************************************************************************************************************/
void AnimationImplode::setNewPixel(byte ColumnOld, byte RowOld, byte ColumnNew, byte RowNew)
{
    if((ColumnOld != ColumnNew) || (RowOld != RowNew))
    {
        Display::getInstance().setPixel(ColumnNew, RowNew);
    }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
