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
/**     \file       AnimationClockShift.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_SHIFT_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "AnimationClockShift.h"


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
  Constructor of AnimationClockShift
******************************************************************************************************************************************************/
/*! \brief          AnimationClockShift Constructor
 *  \details        Instantiation of the AnimationClockShift library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationClockShift::AnimationClockShift()
{
    reset();
} /* AnimationClockShift */


/******************************************************************************************************************************************************
  Destructor of AnimationClockShift
******************************************************************************************************************************************************/
AnimationClockShift::~AnimationClockShift()
{

} /* ~AnimationClockShift */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockShift::init()
{
    AnimationClockCommon::init(STATE_IDLE);
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
StdReturnType AnimationClockShift::setClock(byte Hour, byte Minute)
{
    StdReturnType ReturnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        State = STATE_CLEAR_TIME;
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
void AnimationClockShift::task()
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
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockShift::reset()
{
#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_ON)
    CurrentColumn = 0;
#endif

#if (ANIMATION_CLOCK_SHIFT_VERTICAL == STD_ON)
    CurrentRow = 0;
#endif
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockShift::clearTimeTask()
{
#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_ON)
    if(CurrentColumn < DISPLAY_NUMBER_OF_COLUMNS) {
        wcTransformation.shiftRightFast();
        CurrentColumn++;
    } else {
        State = STATE_SET_TIME;
        reset();
    }
#endif

#if (ANIMATION_CLOCK_SHIFT_VERTICAL == STD_ON)
    if(CurrentRow < DISPLAY_NUMBER_OF_ROWS) {
        wcTransformation.shiftDownFast();
        CurrentRow++;
    } else {
        State = STATE_SET_TIME;
        reset();
    }
#endif

} /* clearTimeTask */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationClockShift::setTimeTask()
{
#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_ON)
    if(CurrentColumn < DISPLAY_NUMBER_OF_COLUMNS) {
        wcTransformation.shiftRightFast();
        for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
            if(isPixelPartOfClockWords(ClockWordsTable, DISPLAY_NUMBER_OF_COLUMNS - CurrentColumn - 1, Row)) {
                Display::getInstance().setPixelFast(0, Row);
            }
        }
        CurrentColumn++;
    } else {
        State = STATE_IDLE;
    }
#endif

#if (ANIMATION_CLOCK_SHIFT_VERTICAL == STD_ON)
    if(CurrentRow < DISPLAY_NUMBER_OF_ROWS) {
        wcTransformation.shiftDownFast();
        for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, Column, DISPLAY_NUMBER_OF_ROWS - CurrentRow - 1)) {
                Display::getInstance().setPixelFast(Column, 0);
            }
        }
        CurrentRow++;
    } else {
        setState(STATE_IDLE);
    }
#endif
} /* setTimeTask */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
