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
        Direction = static_cast<DirectionType>(selectByTime(Hour, Minute, DIRECTION_NUMBER_OF_DIRECTIONS));
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
    ExpandSteps = 1u;
    Direction = DIRECTION_TO_LEFT;
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

    /* Scanned towards the target edge on purpose: a pixel that just moved frees the
       place for the next one behind it, so a whole word slides by one column per frame
       instead of collapsing into a single pixel. */
    if(Direction == DIRECTION_TO_LEFT) {
        for(byte column = 1u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(Display::getInstance().getPixelFast(column, Row) &&
               !Display::getInstance().getPixelFast(column - 1u, Row))
            {
                Display::getInstance().clearPixelFast(column, Row);
                Display::getInstance().setPixelFast(column - 1u, Row);
                moved = true;
            }
        }
    } else {
        for(int8_t column = DISPLAY_NUMBER_OF_COLUMNS - 2; column >= 0; column--) {
            if(Display::getInstance().getPixelFast(static_cast<byte>(column), Row) &&
               !Display::getInstance().getPixelFast(static_cast<byte>(column + 1), Row))
            {
                Display::getInstance().clearPixelFast(static_cast<byte>(column), Row);
                Display::getInstance().setPixelFast(static_cast<byte>(column + 1), Row);
                moved = true;
            }
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
        const byte lettersOfRow = countLettersOfRow(row);
        byte letterIndex{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, row)) {
                Display::getInstance().setPixelFast(calcColumn(column, calcBlockColumn(letterIndex, lettersOfRow)), row);
                letterIndex++;
            }
        }
    }

    /* the last step draws the letters exactly in place, only then it is done */
    if(Step < ExpandSteps) { Step++; }
    else {
        finishWithClockWords(ClockWordsTable);
        reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  calcColumn()
******************************************************************************************************************************************************/
byte AnimationCollapse::calcColumn(byte FinalColumn, byte BlockColumn) const
{
    /* Interpolates between the position in the block and the final one, which needs no
       memory of where a letter currently is. Split by direction to stay unsigned: the
       block is left of the final position in one and right of it in the other. */
    const byte remainingSteps = ExpandSteps - Step;

    if(BlockColumn > FinalColumn) {
        return static_cast<byte>(FinalColumn + (((BlockColumn - FinalColumn) * remainingSteps) / ExpandSteps));
    }

    return static_cast<byte>(FinalColumn - (((FinalColumn - BlockColumn) * remainingSteps) / ExpandSteps));
} /* calcColumn */


/******************************************************************************************************************************************************
  calcBlockColumn()
******************************************************************************************************************************************************/
byte AnimationCollapse::calcBlockColumn(byte LetterIndex, byte LettersOfRow) const
{
    /* Where a letter sits while the row is one solid block. Counting the letters of the
       row up front keeps this a plain calculation, so both directions come out of one
       ascending loop instead of two mirrored ones. */
    if(Direction == DIRECTION_TO_LEFT) { return LetterIndex; }

    return static_cast<byte>(DISPLAY_NUMBER_OF_COLUMNS - LettersOfRow + LetterIndex);
} /* calcBlockColumn */


/******************************************************************************************************************************************************
  countLettersOfRow()
******************************************************************************************************************************************************/
byte AnimationCollapse::countLettersOfRow(byte Row) const
{
    byte letters{0u};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(isPixelPartOfClockWords(ClockWordsTable, column, Row)) { letters++; }
    }

    return letters;
} /* countLettersOfRow */


/******************************************************************************************************************************************************
  calcExpandSteps()
******************************************************************************************************************************************************/
byte AnimationCollapse::calcExpandSteps() const
{
    byte longestWay{0u};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        const byte lettersOfRow = countLettersOfRow(row);
        byte letterIndex{0u};

        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, row)) {
                const byte blockColumn = calcBlockColumn(letterIndex, lettersOfRow);
                const byte way = (blockColumn > column) ? (blockColumn - column) : (column - blockColumn);
                if(way > longestWay) { longestWay = way; }
                letterIndex++;
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
