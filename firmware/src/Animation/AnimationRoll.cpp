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
/**     \file       AnimationRoll.cpp
 *      \brief      Rolls the previous time out while the new one rolls in
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_ROLL_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationRoll.h"


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
void AnimationRoll::init()
{
    Animation::init(STATE_IDLE);
    /* all animations share one union, so nothing may rely on member initializers */
    reset();
} /* init */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationRoll::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        Direction = calcDirection(Hour, Minute);
        Step = 0u;
        /* no clear phase: the previous time is still on the display and rolls out
           while the new one rolls in behind it */
        State = STATE_SET_TIME;
        returnValue = E_OK;
    }
    return returnValue;
} /* setTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationRoll::task()
{
    if(State == STATE_SET_TIME) { setTimeTask(); }
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationRoll::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    Step = 0u;
    Direction = DIRECTION_RIGHT;
} /* reset */


/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationRoll::setTimeTask()
{
    if(Step < numberOfSteps()) {
        if(Direction == DIRECTION_RIGHT || Direction == DIRECTION_LEFT) { rollHorizontal(); }
        else { rollVertical(); }
        Step++;
    } else {
        /* the new time has reached its place, nothing left to shift */
        State = STATE_IDLE;
        reset();
    }
} /* setTimeTask */


/******************************************************************************************************************************************************
  rollHorizontal()
******************************************************************************************************************************************************/
void AnimationRoll::rollHorizontal()
{
    /* Shift everything by one and feed the column of the new time that belongs in at
       the freed edge. The remaining shifts carry it to its final column, so the
       column entering in step n is the one that ends up n columns away from the edge
       it entered at. */
    if(Direction == DIRECTION_RIGHT) {
        wcTransformation.shiftRightFast();
        for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
            if(isPixelPartOfClockWords(ClockWordsTable, DISPLAY_NUMBER_OF_COLUMNS - Step - 1u, row)) {
                Display::getInstance().setPixelFast(0u, row);
            }
        }
    } else {
        wcTransformation.shiftLeftFast();
        for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
            if(isPixelPartOfClockWords(ClockWordsTable, Step, row)) {
                Display::getInstance().setPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1u, row);
            }
        }
    }
} /* rollHorizontal */


/******************************************************************************************************************************************************
  rollVertical()
******************************************************************************************************************************************************/
void AnimationRoll::rollVertical()
{
    /* same as rollHorizontal(), with rows instead of columns */
    if(Direction == DIRECTION_DOWN) {
        wcTransformation.shiftDownFast();
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, DISPLAY_NUMBER_OF_ROWS - Step - 1u)) {
                Display::getInstance().setPixelFast(column, 0u);
            }
        }
    } else {
        wcTransformation.shiftUpFast();
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            if(isPixelPartOfClockWords(ClockWordsTable, column, Step)) {
                Display::getInstance().setPixelFast(column, DISPLAY_NUMBER_OF_ROWS - 1u);
            }
        }
    }
} /* rollVertical */


/******************************************************************************************************************************************************
  numberOfSteps()
******************************************************************************************************************************************************/
byte AnimationRoll::numberOfSteps() const
{
    if(Direction == DIRECTION_RIGHT || Direction == DIRECTION_LEFT) { return DISPLAY_NUMBER_OF_COLUMNS; }

    return DISPLAY_NUMBER_OF_ROWS;
} /* numberOfSteps */


/******************************************************************************************************************************************************
  calcDirection()
******************************************************************************************************************************************************/
AnimationRoll::DirectionType AnimationRoll::calcDirection(byte Hour, byte Minute) const
{
    /* Same hash as AnimationMatrix::startFrame(), for the same reason: taking the
       minute modulo the number of directions would walk through them in a fixed order
       instead of drawing one. */
    byte hash = static_cast<byte>(((Hour * 60u) + Minute) * DirectionHashFactor);
    hash ^= static_cast<byte>(hash >> 4u);
    hash = static_cast<byte>(hash * DirectionHashFactor);

    /* scaled by multiplication instead of a modulo, which would favour the first
       directions because 256 is not a multiple of the number of directions */
    return static_cast<DirectionType>((static_cast<uint16_t>(hash) * DIRECTION_NUMBER_OF_DIRECTIONS) >> 8u);
} /* calcDirection */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
