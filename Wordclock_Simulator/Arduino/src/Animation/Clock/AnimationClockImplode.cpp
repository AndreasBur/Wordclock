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
    if(State == STATE_CLEAR_TIME) { clearTimeTask(); }
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
    if(ShiftCounter < ANIMATION_CLOCK_SHIFT_COUNTER_MAX_VALUE) {
        ShiftCounter++;
        //shiftQuadrants();
        shiftQuadrantUpperLeft();
        shiftQuadrantLowerRight();
        shiftQuadrantUpperRight();
        shiftQuadrantLowerLeft();
    } else {
        State = STATE_SET_TIME;
    }
} /* clearTimeTask */

/******************************************************************************************************************************************************
  shiftQuadrants()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrants()
{
    for(uint8_t Column = 0u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(uint8_t Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
            if(Display::getInstance().getPixelFast(Column, Row)) {
                byte ColumnNext = Column, RowNext = Row;
                Display::getInstance().clearPixelFast(Column, Row);
                if(Column < DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext = shiftRight(Column);
                if(Column > DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext = shiftLeft(Column);
                if(Row < DISPLAY_NUMBER_OF_ROWS / 2u) RowNext = shiftDown(Row);
                if(Row > DISPLAY_NUMBER_OF_ROWS / 2u) RowNext = shiftUp(Row);
                Display::getInstance().setPixelFast(ColumnNext, RowNext);
            }
        }
    }
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantUpperLeft()
{
    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS / 2u; Column >= 0; Column--) {
        for(int8_t Row = DISPLAY_NUMBER_OF_ROWS / 2u; Row >= 0; Row--) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftDownRight(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftDownRight(byte Column, byte Row)
{
    byte ColumnNext = Column;
    byte RowNext = Row;

    Display::getInstance().clearPixelFast(Column, Row);
    if(Column < DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext++;
    if(Row < DISPLAY_NUMBER_OF_COLUMNS / 2u) RowNext++;
    Display::getInstance().setPixelFast(ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantUpperRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantUpperRight()
{
    for(uint8_t Column = (DISPLAY_NUMBER_OF_COLUMNS / 2u) + 1u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = (DISPLAY_NUMBER_OF_ROWS / 2u) - 1u; Row >= 0; Row--) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftDownLeft(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftDownLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftDownLeft(byte Column, byte Row)
{
    byte ColumnNext = Column;
    byte RowNext = Row;

    Display::getInstance().clearPixelFast(Column, Row);
    if(Column > DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext--;
    if(Row < DISPLAY_NUMBER_OF_COLUMNS / 2u) RowNext++;
    Display::getInstance().setPixelFast(ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantLowerLeft()
{
    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS / 2u - 1u; Column >= 0; Column--) {
        for(int8_t Row = DISPLAY_NUMBER_OF_ROWS / 2u + 1u; Row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); Row++) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftUpRight(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftUpRight(byte Column, byte Row)
{
    byte ColumnNext = Column;
    byte RowNext = Row;

    Display::getInstance().clearPixelFast(Column, Row);
    if(Column < DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext++;
    if(Row > DISPLAY_NUMBER_OF_COLUMNS / 2u) RowNext--;
    Display::getInstance().setPixelFast(ColumnNext, RowNext);
}

/******************************************************************************************************************************************************
  shiftQuadrantLowerRight()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftQuadrantLowerRight()
{
    for(uint8_t Column = (DISPLAY_NUMBER_OF_COLUMNS / 2u); Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = DISPLAY_NUMBER_OF_ROWS / 2u; Row < static_cast<int8_t>(DISPLAY_NUMBER_OF_ROWS); Row++) {
            if(Display::getInstance().getPixelFast(Column, Row)) { shiftUpLeft(Column, Row); }
        }
    }
}

/******************************************************************************************************************************************************
  shiftUpLeft()
******************************************************************************************************************************************************/
void AnimationClockImplode::shiftUpLeft(byte Column, byte Row)
{
    byte ColumnNext = Column;
    byte RowNext = Row;

    Display::getInstance().clearPixelFast(Column, Row);
    if(Column > DISPLAY_NUMBER_OF_COLUMNS / 2u) ColumnNext--;
    if(Row > DISPLAY_NUMBER_OF_COLUMNS / 2u) RowNext--;
    Display::getInstance().setPixelFast(ColumnNext, RowNext);
}



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
