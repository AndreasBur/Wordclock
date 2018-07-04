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
/**     \file       AnimationClockSnake.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_SNAKE_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationClockSnake.h"


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
  Constructor of AnimationClockSnake
******************************************************************************************************************************************************/
/*! \brief          AnimationClockSnake Constructor
 *  \details        Instantiation of the AnimationClockSnake library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockSnake::AnimationClockSnake()
{
    pDisplay = nullptr;
    pClock = nullptr;
    reset();
} /* AnimationClockSnake */


/******************************************************************************************************************************************************
  Destructor of AnimationClockSnake
******************************************************************************************************************************************************/
AnimationClockSnake::~AnimationClockSnake()
{

} /* ~AnimationClockSnake */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockSnake::init(Display* Display, Clock* Clock)
{
    pDisplay = Display;
    pClock = Clock;
    setState(AnimationClockCommon::STATE_IDLE);
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
stdReturnType AnimationClockSnake::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && getState() == AnimationClockCommon::STATE_IDLE) {
        ReturnValue = E_OK;
        SnakeBeginIndex = 0;
        SnakeEndIndex = 0;
        setState(AnimationClockCommon::STATE_SET_TIME);
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
void AnimationClockSnake::task()
{
    if(getState() == AnimationClockCommon::STATE_SET_TIME) {
        byte SnakeEndIndexTrans = transformToSerpentine(SnakeEndIndex);
        pDisplay->setPixelFast(transformToSerpentine(SnakeBeginIndex));

        if((SnakeBeginIndex - SnakeEndIndex) == ANIMATION_CLOCK_SNAKE_LENGTH ||
           (SnakeBeginIndex >= DISPLAY_NUMBER_OF_LEDS && SnakeEndIndex < DISPLAY_NUMBER_OF_LEDS))
        {
            //pDisplay->clearPixelFast(SnakeEndIndexTrans);
            if(isPixelPartOfClockWords(ClockWordsTable, SnakeEndIndexTrans) == false) pDisplay->clearPixelFast(SnakeEndIndexTrans);
            SnakeEndIndex++;
        }
        if(SnakeBeginIndex < DISPLAY_NUMBER_OF_LEDS) SnakeBeginIndex++;
        if(SnakeEndIndex >= DISPLAY_NUMBER_OF_LEDS) setState(AnimationClockCommon::STATE_IDLE);;
    }
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
void AnimationClockSnake::reset()
{

} /* reset */


/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
byte AnimationClockSnake::transformToSerpentine(byte Column, byte Row) const
{
    byte Index;

    if(IS_BIT_CLEARED(Row, 0)) Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    else Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - Column - 1);

    return Index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
byte AnimationClockSnake::transformToSerpentine(byte Index) const
{
    byte Column = pDisplay->indexToColumn(Index);
    byte Row = pDisplay->indexToRow(Index);

    return transformToSerpentine(Column, Row);
} /* transformToSerpentine */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/