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
/**     \file       AnimationCollapse.cpp
 *      \brief      Letters slide together and the new ones slide apart again
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_COLLAPSE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationCollapse.h"


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
void AnimationCollapse::init()
{
    Animation::init(STATE_IDLE);
    /* all animations share one union, so nothing may rely on member initializers */
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationCollapse::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        Step = 0u;
        /* the previous time is on the display and is what collapses first. An empty
           display simply finishes that phase on its first task. */
        State = STATE_CLEAR_TIME;
        returnValue = E_OK;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationCollapse::task()
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
void AnimationCollapse::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    Step = 0u;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationCollapse::clearTimeTask()
{
    bool moved{false};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(collapseRow(row)) { moved = true; }
    }

    /* nothing moved any more, so every row is a solid block at the left edge */
    if(!moved) {
        Step = 0u;
        ExpandSteps = calcExpandSteps();
        State = STATE_SET_TIME;
    }
} /* clearTimeTask */


/******************************************************************************************************************************************************
  collapseRow()
******************************************************************************************************************************************************/
bool AnimationCollapse::collapseRow(byte Row)
{
    bool moved{false};

    /* From left to right on purpose: a pixel that just moved frees the place for its
       right neighbour, so a whole word slides by one column per frame instead of
       collapsing into a single pixel. */
    for(byte column = 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(Display::getInstance().getPixelFast(column, Row) &&
           !Display::getInstance().getPixelFast(column - 1u, Row))
        {
            Display::getInstance().clearPixelFast(column, Row);
            Display::getInstance().setPixelFast(column - 1u, Row);
            moved = true;
        }
    }

    return moved;
} /* collapseRow */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationCollapse::setTimeTask()
{
    Display::getInstance().clear();

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        byte blockColumn{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, row)) {
                Display::getInstance().setPixelFast(calcColumn(column, blockColumn), row);
                blockColumn++;
            }
        }
    }

    /* the last step draws the letters exactly in place, only then it is done */
    if(Step < ExpandSteps) { Step++; }
    else {
        State = STATE_IDLE;
        reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  calcColumn()
******************************************************************************************************************************************************/
byte AnimationCollapse::calcColumn(byte FinalColumn, byte BlockColumn) const
{
    /* Interpolates between the position in the block and the final one, which needs no
       memory of where a letter currently is: the block position is simply how many
       letters of that row come before it. */
    const byte remainingSteps = ExpandSteps - Step;

    return static_cast<byte>(FinalColumn - (((FinalColumn - BlockColumn) * remainingSteps) / ExpandSteps));
} /* calcColumn */


/******************************************************************************************************************************************************
  calcExpandSteps()
******************************************************************************************************************************************************/
byte AnimationCollapse::calcExpandSteps() const
{
    byte longestWay{0u};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        byte blockColumn{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, row)) {
                const byte way = column - blockColumn;
                if(way > longestWay) { longestWay = way; }
                blockColumn++;
            }
        }
    }

    /* never zero: calcColumn() divides by it, and a picture that needs no movement at
       all is then drawn once and done */
    if(longestWay == 0u) { return 1u; }

    return longestWay;
} /* calcExpandSteps */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
