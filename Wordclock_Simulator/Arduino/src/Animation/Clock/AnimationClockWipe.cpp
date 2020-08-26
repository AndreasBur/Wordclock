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
/**     \file       AnimationClockWipe.cpp
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
#include "AnimationClockWipe.h"


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
void AnimationClockWipe::init()
{
    AnimationClockCommon::init(STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationClockWipe::setTime(byte Hour, byte Minute)
{
    StdReturnType ReturnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        ReturnValue = E_OK;
        State = STATE_CLEAR_TIME;
    }
    return ReturnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationClockWipe::task()
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
void AnimationClockWipe::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    Index = 0u;
    SetPixelState = SET_PIXEL_STATE_DOWN;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationClockWipe::clearTimeTask()
{
    byte Column, Row;

    Display::getInstance().indexToColumnAndRow(Index, Column, Row);

    do {
        if(Display::getInstance().getPixelFast(Column, Row)) {
            if(SetPixelState == SET_PIXEL_STATE_DOWN) setPixelDown(Column, Row);
            else setPixelRight(Column, Row);
        }
    } while(Column-- != 0u && Row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

    if(SetPixelState == SET_PIXEL_STATE_DOWN) SetPixelState = SET_PIXEL_STATE_RIGHT;
    else SetPixelState = SET_PIXEL_STATE_DOWN;

    if(setNextIndex() == E_NOT_OK) {
        State = STATE_SET_TIME;
        reset();
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationClockWipe::setTimeTask()
{
    byte Column, Row;

    Display::getInstance().indexToColumnAndRow(Index, Column, Row);

    do {
        if(isPixelPartOfClockWords(ClockWordsTable, Column, Row)) { Display::getInstance().setPixelFast(Column, Row); }
    } while(Column-- != 0u && Row++ < DISPLAY_NUMBER_OF_ROWS - 1u);

    if(setNextIndex() == E_NOT_OK) {
        State = STATE_IDLE;
        reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  setNextIndex()
******************************************************************************************************************************************************/
bool AnimationClockWipe::setNextIndex()
{
    StdReturnType ReturValue = E_OK;
    byte Column, Row;

    Display::getInstance().indexToColumnAndRow(Index, Column, Row);

    if(Column < DISPLAY_NUMBER_OF_COLUMNS - 1u) Column++;
    else if(Row < DISPLAY_NUMBER_OF_ROWS - 1u) Row++;
    else ReturValue = E_NOT_OK;

    Index = Display::getInstance().columnAndRowToIndex(Column, Row);
    return ReturValue;
} /* setNextIndex */


/******************************************************************************************************************************************************
  setPixelDown()
******************************************************************************************************************************************************/
void AnimationClockWipe::setPixelDown(byte Column, byte Row)
{
    Display::getInstance().clearPixelFast(Column, Row);

    for(byte RowNext = Row + 1u; RowNext < DISPLAY_NUMBER_OF_ROWS; RowNext++) {
        if(Display::getInstance().getPixelFast(Column, RowNext) == false) {
            Display::getInstance().setPixelFast(Column, RowNext);
            break;
        }
    }
} /* setPixelDown */


/******************************************************************************************************************************************************
  setPixelRight()
******************************************************************************************************************************************************/
void AnimationClockWipe::setPixelRight(byte Column, byte Row)
{
    Display::getInstance().clearPixelFast(Column, Row);

    for(byte ColumnNext = Column + 1u; ColumnNext < DISPLAY_NUMBER_OF_COLUMNS; ColumnNext++) {
        if(Display::getInstance().getPixelFast(ColumnNext, Row) == false) {
            Display::getInstance().setPixelFast(ColumnNext, Row);
            break;
        }
    }
} /* setPixelRight */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
