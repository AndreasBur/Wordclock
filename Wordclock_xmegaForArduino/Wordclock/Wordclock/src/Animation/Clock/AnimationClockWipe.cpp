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
  Constructor of AnimationClockWipe
******************************************************************************************************************************************************/
/*! \brief          AnimationClockWipe Constructor
 *  \details        Instantiation of the AnimationClockWipe library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockWipe::AnimationClockWipe()
{
    reset();
} /* AnimationClockWipe */


/******************************************************************************************************************************************************
  Destructor of AnimationClockWipe
******************************************************************************************************************************************************/
AnimationClockWipe::~AnimationClockWipe()
{

} /* ~AnimationClockWipe */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::init(Display* Display, Clock* Clock)
{
    AnimationClockCommon::init(Display, Clock, AnimationClockCommon::STATE_IDLE);
    reset();
} /* init */


/******************************************************************************************************************************************************
  setClock()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationClockWipe::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == AnimationClockCommon::STATE_IDLE) {
        ReturnValue = E_OK;
        State = AnimationClockCommon::STATE_CLEAR_TIME;
    }
    return ReturnValue;
} /* setClock */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::task()
{
    if(State == AnimationClockCommon::STATE_CLEAR_TIME) { clearTimeTask(); }
    if(State == AnimationClockCommon::STATE_SET_TIME) { setTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::reset()
{
    Index = 0;
    SetPixelState = SET_PIXEL_STATE_DOWN;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::clearTimeTask()
{
    byte Column, Row;

    pDisplay->indexToColumnAndRow(Index, Column, Row);

    do {
        if(pDisplay->getPixelFast(Column, Row)) {
            if(SetPixelState == SET_PIXEL_STATE_DOWN) setPixelDown(Column, Row);
            else setPixelRight(Column, Row);
        }
    } while(Column-- != 0 && Row++ < DISPLAY_NUMBER_OF_ROWS - 1);

    if(SetPixelState == SET_PIXEL_STATE_DOWN) SetPixelState = SET_PIXEL_STATE_RIGHT;
    else SetPixelState = SET_PIXEL_STATE_DOWN;

    if(setNextIndex() == E_NOT_OK) {
        State = AnimationClockCommon::STATE_SET_TIME;
        reset();
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::setTimeTask()
{
    byte Column, Row;

    pDisplay->indexToColumnAndRow(Index, Column, Row);

    do {
        if(isPixelPartOfClockWords(ClockWordsTable, Column, Row)) { pDisplay->setPixelFast(Column, Row); }
    } while(Column-- != 0 && Row++ < DISPLAY_NUMBER_OF_ROWS - 1);

    if(setNextIndex() == E_NOT_OK) State = AnimationClockCommon::STATE_IDLE;
} /* setTimeTask */


/******************************************************************************************************************************************************
  setNextIndex()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean AnimationClockWipe::setNextIndex()
{
    stdReturnType ReturValue = E_OK;
    byte Column, Row;

    pDisplay->indexToColumnAndRow(Index, Column, Row);

    if(Column < DISPLAY_NUMBER_OF_COLUMNS - 1) Column++;
    else if(Row < DISPLAY_NUMBER_OF_ROWS - 1) Row++;
    else ReturValue = E_NOT_OK;

    Index = pDisplay->columnAndRowToIndex(Column, Row);
    return ReturValue;
} /* setNextIndex */


/******************************************************************************************************************************************************
  setPixelDown()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::setPixelDown(byte Column, byte Row)
{
    pDisplay->clearPixelFast(Column, Row);

    for(byte RowNext = Row + 1; RowNext < DISPLAY_NUMBER_OF_ROWS; RowNext++) {
        if(pDisplay->getPixelFast(Column, RowNext) == false) {
            pDisplay->setPixelFast(Column, RowNext);
            break;
        }
    }
} /* setPixelDown */


/******************************************************************************************************************************************************
  setPixelRight()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockWipe::setPixelRight(byte Column, byte Row)
{
    pDisplay->clearPixelFast(Column, Row);

    for(byte ColumnNext = Column + 1; ColumnNext < DISPLAY_NUMBER_OF_COLUMNS; ColumnNext++) {
        if(pDisplay->getPixelFast(ColumnNext, Row) == false) {
            pDisplay->setPixelFast(ColumnNext, Row);
            break;
        }
    }
} /* setPixelRight */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
