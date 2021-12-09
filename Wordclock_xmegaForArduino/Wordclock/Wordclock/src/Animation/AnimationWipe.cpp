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
/**     \file       AnimationWipe.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_WIPE_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationWipe.h"


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
void AnimationWipe::init()
{
    AnimationCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationWipe::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        State = STATE_CLEAR_TIME;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationWipe::task()
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
void AnimationWipe::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    Index = 0u;
    SetPixelState = SET_PIXEL_STATE_DOWN;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationWipe::clearTimeTask()
{
    byte column, row;

    Display::getInstance().indexToColumnAndRow(Index, column, row);

    do {
        if(Display::getInstance().getPixelFast(column, row)) {
            if(SetPixelState == SET_PIXEL_STATE_DOWN) { setPixelDown(column, row); }
            else { setPixelRight(column, row); }
        }
    } while(column-- != 0u && row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

    if(SetPixelState == SET_PIXEL_STATE_DOWN) { SetPixelState = SET_PIXEL_STATE_RIGHT; }
    else { SetPixelState = SET_PIXEL_STATE_DOWN; }

    if(setNextIndex() == E_NOT_OK) {
        Index = 0u;
        State = STATE_SET_TIME;
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationWipe::setTimeTask()
{
    byte column, row;

    Display::getInstance().indexToColumnAndRow(Index, column, row);

    do {
        if(isPixelPartOfClockWords(ClockWordsTable, column, row)) {
            Display::getInstance().setPixelFast(column, row);
        }
    } while(column-- != 0u && row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

    if(setNextIndex() == E_NOT_OK) {
        State = STATE_IDLE;
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  setNextIndex()
******************************************************************************************************************************************************/
StdReturnType AnimationWipe::setNextIndex()
{
    StdReturnType returnValue = E_OK;
    byte column, row;

    Display::getInstance().indexToColumnAndRow(Index, column, row);

    if(column < DISPLAY_NUMBER_OF_COLUMNS - 1u) column++;
    else if(row < DISPLAY_NUMBER_OF_ROWS - 1u) row++;
    else returnValue = E_NOT_OK;

    Index = Display::getInstance().columnAndRowToIndex(column, row);
    return returnValue;
} /* setNextIndex */


/******************************************************************************************************************************************************
  setPixelDown()
******************************************************************************************************************************************************/
void AnimationWipe::setPixelDown(byte Column, byte Row)
{
    Display::getInstance().clearPixelFast(Column, Row);

    for(byte rowNext = Row + 1u; rowNext < DISPLAY_NUMBER_OF_ROWS; rowNext++) {
        if(Display::getInstance().getPixelFast(Column, rowNext) == false) {
            Display::getInstance().setPixelFast(Column, rowNext);
            break;
        }
    }
} /* setPixelDown */


/******************************************************************************************************************************************************
  setPixelRight()
******************************************************************************************************************************************************/
void AnimationWipe::setPixelRight(byte Column, byte Row)
{
    Display::getInstance().clearPixelFast(Column, Row);

    for(byte columnNext = Column + 1u; columnNext < DISPLAY_NUMBER_OF_COLUMNS; columnNext++) {
        if(Display::getInstance().getPixelFast(columnNext, Row) == false) {
            Display::getInstance().setPixelFast(columnNext, Row);
            break;
        }
    }
} /* setPixelRight */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/