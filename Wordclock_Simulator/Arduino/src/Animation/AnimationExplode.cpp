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
/**     \file       AnimationExplode.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _ANIMATION_CLOCK_EXPLODE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationExplode.h"


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
void AnimationExplode::init()
{
    Animation::init(STATE_IDLE);
    reset();
} /* init */

/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::setTime(byte Hour, byte Minute)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Clock::getInstance().getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        returnValue = E_OK;
        if(setNextWordToClear() == E_OK) { State = STATE_CLEAR_TIME; }
        else { State = STATE_SET_TIME; }
    }
    return returnValue;
} /* setTime */

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void AnimationExplode::task()
{
    if(State == STATE_CLEAR_TIME) clearTimeTask();
    else if(State == STATE_SET_TIME) setTimeTask();
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
void AnimationExplode::reset()
{
    ClockWordsTable.fill(DisplayWords::WORD_NONE);
    ClockWordsTableIndex = 0u;
    CurrentWordIndex = Display::getInstance().columnAndRowToIndex(0u, RowCenter);
    CurrentWordLength = 0u;
    CurrentColumn = 0u;
    CurrentRow = 0u;
} /* reset */

/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
void AnimationExplode::clearTimeTask()
{
    if(shiftWord(getClearFinalColumn(), getClearFinalRow()) == E_NOT_OK) {
        toggleWordOnDisplay();
        if(setNextWordToClear() == E_NOT_OK) {
            CurrentWordIndex = 0u;
            setWordToSet(ClockWordsTable[CurrentWordIndex]);
            setWordOnDisplay();
            State = STATE_SET_TIME;
        }
    }
} /* clearTimeTask */

/******************************************************************************************************************************************************
  setTimeTask()
******************************************************************************************************************************************************/
void AnimationExplode::setTimeTask()
{
    if(shiftWord(getSetFinalColumn(), getSetFinalRow()) == E_NOT_OK) {
        if(setNextWordToSet() == E_NOT_OK) { State = STATE_IDLE; }
        else { toggleWordOnDisplay(); }
    }
} /* setTimeTask */

/******************************************************************************************************************************************************
  shiftWord()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::shiftWord(byte FinalColum, byte FinalRow)
{
    if(CurrentColumn != getNextColumn(FinalColum) || CurrentRow != getNextRow(FinalRow)) {
        toggleWordOnDisplay();
        CurrentColumn = getNextColumn(FinalColum);
        CurrentRow = getNextRow(FinalRow);
        toggleWordOnDisplay();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* shiftWord */

/******************************************************************************************************************************************************
  toggleWordOnDisplay()
******************************************************************************************************************************************************/
void AnimationExplode::toggleWordOnDisplay()
{
    for(byte columnIndex = CurrentColumn; columnIndex < CurrentColumn + CurrentWordLength; columnIndex++) {
        Display::getInstance().togglePixelFast(columnIndex, CurrentRow);
    }
} /* toggleWordOnDisplay */

/******************************************************************************************************************************************************
  setWordOnDisplay()
******************************************************************************************************************************************************/
void AnimationExplode::setWordOnDisplay()
{
    for(byte columnIndex = CurrentColumn; columnIndex < CurrentColumn + CurrentWordLength; columnIndex++) {
        Display::getInstance().setPixelFast(columnIndex, CurrentRow);
    }
} /* setWordOnDisplay */

/******************************************************************************************************************************************************
  setNextWordToClear()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::setNextWordToClear()
{
    CurrentColumn = Display::getInstance().indexToColumn(CurrentWordIndex) + CurrentWordLength;
    CurrentRow = Display::getInstance().indexToRow(CurrentWordIndex);

    do {
        if(setNextWordToClearInColumn() == E_OK) { return E_OK; }
        CurrentColumn = 0;
    } while(setNextRowToClear() == E_OK);

    return E_NOT_OK;
} /* setNextWordToClear */

/******************************************************************************************************************************************************
  setNextWordToClearInColumn()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::setNextWordToClearInColumn()
{
    for(; CurrentColumn < DISPLAY_NUMBER_OF_COLUMNS; CurrentColumn++) {
        if(Display::getInstance().getPixelFast(CurrentColumn, CurrentRow)) {
            CurrentWordIndex = Display::getInstance().columnAndRowToIndex(CurrentColumn, CurrentRow);
            setWordLength();
            return E_OK;
        }
    }
    return E_NOT_OK;
} /* setNextWordToClearInColumn */

/******************************************************************************************************************************************************
  setRowNextToClear()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::setNextRowToClear()
{
    if(CurrentRow > 0 && CurrentRow < DISPLAY_NUMBER_OF_ROWS) {
        if(CurrentRow >= RowCenter) { CurrentRow = DISPLAY_NUMBER_OF_ROWS - CurrentRow - 1u; }
        else { CurrentRow = DISPLAY_NUMBER_OF_ROWS - CurrentRow; }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setRowNextToClear */

/******************************************************************************************************************************************************
  setNextWordToSet()
******************************************************************************************************************************************************/
StdReturnType AnimationExplode::setNextWordToSet()
{
    for(byte index = ClockWordsTableIndex + 1u; index < ClockWordsTable.size(); index++) {
        if(ClockWordsTable[index] != DisplayWords::WORD_NONE) {
            ClockWordsTableIndex = index;
            setWordToSet(ClockWordsTable[ClockWordsTableIndex]);
            return E_OK;
        }
    }
    return E_NOT_OK;
} /* setNextWordToSet */

/******************************************************************************************************************************************************
  setWordToSet()
******************************************************************************************************************************************************/
void AnimationExplode::setWordToSet(DisplayWords::WordIdType WordId)
{
    setWordIndex(WordId);
    CurrentWordLength = Words.getDisplayWordLengthFast(WordId);
    CurrentColumn = getClearFinalColumn();
    CurrentRow = getClearFinalRow();
} /* setWordToSet */

/******************************************************************************************************************************************************
  setWordIndex()
******************************************************************************************************************************************************/
void AnimationExplode::setWordIndex(DisplayWords::WordIdType WordId)
{
    byte column = Words.getDisplayWordColumnFast(WordId);
    byte row = Words.getDisplayWordRowFast(WordId);
    CurrentWordIndex = Display::getInstance().columnAndRowToIndex(column, row);
} /* setWordIndex */

/******************************************************************************************************************************************************
  setWordLength()
******************************************************************************************************************************************************/
void AnimationExplode::setWordLength()
{
    CurrentWordLength = 1u;

    for(byte index = CurrentWordIndex + 1u; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
        if(Display::getInstance().getPixelFast(index)) {
            CurrentWordLength++;
        } else {
            break;
        }
    }
} /* setWordLength */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
