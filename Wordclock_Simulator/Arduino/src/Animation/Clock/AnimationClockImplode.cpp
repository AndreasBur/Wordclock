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
/**     \file       AnimationClockImplode.cpp
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
#include "AnimationClockImplode.h"


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
void AnimationClockImplode::init()
{
    AnimationClockCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationClockImplode::setTime(byte sHour, byte sMinute)
{
    StdReturnType ReturnValue{E_NOT_OK};

    if(State == STATE_IDLE) {
        ReturnValue = E_OK;
        Display::getInstance().setPixel(ColumnCenter, RowCenter);
        ShiftCounter = 0u;
        Hour = sHour;
        Minute = sMinute;
        State = STATE_CLEAR_TIME;
    }
    return ReturnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClockImplode::task()
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
            Clock::getInstance().setTimeFast(Hour, Minute);
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
void AnimationClockImplode::reset()
{
    ShiftCounter = 0u;
    Hour = 0u;
    Minute = 0u;
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationClockImplode::clearTimeTask()
{
    shiftQuadrantUpperLeft();
    shiftQuadrantLowerRight();
    shiftQuadrantUpperRight();
    shiftQuadrantLowerLeft();
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationClockImplode::setTimeTask()
{
    DisplayPixels pixels;
    Display::getInstance().clear();
    Clock::getInstance().setTimeFast(Hour, Minute);
    pixels.getPixelsFromDisplay();
    Display::getInstance().clear();
    shiftQuadrants(pixels, ShiftCounter);
    Display::getInstance().setPixel(ColumnCenter, RowCenter);
} /* setTimeTask */

/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
void AnimationClockImplode::setStateToSetTime()
{
    State = STATE_SET_TIME;
    Display::getInstance().clear();
    ShiftCounter = ShiftCounterMaxValue;
} /* setStateToSetTime */

/******************************************************************************************************************************************************
  shiftQuadrants()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrants(DisplayPixels& Pixels, byte NumberOfShifts)
{
    for(uint8_t Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(uint8_t Column = 0u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            if(Pixels.getPixel(Column, Row)) {
                byte ColumnNext = Column, RowNext = Row;
                if(Column < ColumnCenter) ColumnNext = shiftRight(Column, NumberOfShifts);
                if(Column > ColumnCenter) ColumnNext = shiftLeft(Column, NumberOfShifts);
                if(Row < RowCenter) RowNext = shiftDown(Row, NumberOfShifts);
                if(Row > RowCenter) RowNext = shiftUp(Row, NumberOfShifts);
                setNewPixel(Column, Row, ColumnNext, RowNext);
            }
        }
    }
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantUpperLeft()
{
    for(int8_t Column = ColumnCenter; Column >= 0; Column--) {
        for(int8_t Row = RowCenter; Row >= 0; Row--) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftDownRight(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftDownRight(byte Column, byte Row)
{
    byte ColumnNext = Column, RowNext = Row;

    if(Column < ColumnCenter) ColumnNext++;
    if(Row < ColumnCenter) RowNext++;
    clearOldAndSetNewPixel(Column, Row, ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantUpperRight()
{
    for(uint8_t Column = ColumnCenter + 1u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = RowCenter - 1u; Row >= 0; Row--) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftDownLeft(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftDownLeft(byte Column, byte Row)
{
    byte ColumnNext = Column, RowNext = Row;

    if(Column > ColumnCenter) ColumnNext--;
    if(Row < ColumnCenter) RowNext++;
    clearOldAndSetNewPixel(Column, Row, ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantLowerLeft()
{
    for(int8_t Column = ColumnCenter - 1u; Column >= 0; Column--) {
        for(int8_t Row = RowCenter + 1u; Row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); Row++) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftUpRight(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftUpRight(byte Column, byte Row)
{
    byte ColumnNext = Column, RowNext = Row;

    if(Column < ColumnCenter) ColumnNext++;
    if(Row > ColumnCenter) RowNext--;
    clearOldAndSetNewPixel(Column, Row, ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantLowerRight()
{
    for(uint8_t Column = ColumnCenter; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = RowCenter; Row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); Row++) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftUpLeft(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftUpLeft(byte Column, byte Row)
{
    byte ColumnNext = Column, RowNext = Row;

    if(Column > ColumnCenter) ColumnNext--;
    if(Row > ColumnCenter) RowNext--;
    clearOldAndSetNewPixel(Column, Row, ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  clearOldAndSetNewPixel()
******************************************************************************************************************************************************/
void AnimationClockImplode::clearOldAndSetNewPixel(byte ColumnOld, byte RowOld, byte ColumnNew, byte RowNew)
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
void AnimationClockImplode::setNewPixel(byte ColumnOld, byte RowOld, byte ColumnNew, byte RowNew)
{
    if((ColumnOld != ColumnNew) || (RowOld != RowNew))
    {
        Display::getInstance().setPixel(ColumnNew, RowNew);
    }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
