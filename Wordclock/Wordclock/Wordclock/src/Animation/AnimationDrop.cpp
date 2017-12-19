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
/**     \file       AnimationDrop.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _ANIMATION_DROP_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "AnimationDrop.h"


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
  CONSTRUCTOR OF AnimationDrop
******************************************************************************************************************************************************/
/*! \brief          AnimationDrop Constructor
 *  \details        Instantiation of the AnimationDrop library
 *
 *  \return         -
******************************************************************************************************************************************************/
AnimationDrop::AnimationDrop()
{
    pDisplay = nullptr;
    pClock = nullptr;
    State = STATE_UNINIT;
    reset();
} /* AnimationDrop */


/******************************************************************************************************************************************************
  DESTRUCTOR OF AnimationDrop
******************************************************************************************************************************************************/
AnimationDrop::~AnimationDrop()
{

} /* ~AnimationDrop */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationDrop::init(Display* Display, Clock* Clock)
{
    pDisplay = Display;
    pClock = Clock;
    State = STATE_IDLE;
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
stdReturnType AnimationDrop::setClock(byte Hour, byte Minute)
{
    stdReturnType ReturnValue{E_NOT_OK};

    if(pClock->getClockWords(Hour, Minute, ClockWordsTable) == E_OK && State == STATE_IDLE) {
        setNextWordIndex();
        if(setNextActivePixelIndex() == E_NOT_OK) { setStateToSetTime(); }
        else { State = STATE_CLEAR_TIME; }
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
void AnimationDrop::task()
{
    if(State == STATE_CLEAR_TIME) { clearTimeTask(); }
    if(State == STATE_SET_TIME) { setTimeTask(); }
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
void AnimationDrop::reset()
{
    CurrentPixelIndex = 0;
    CurrenWordIndex = CLOCK_WORDS_TABLE_TYPE_SIZE - 1;
} /* reset */


/******************************************************************************************************************************************************
  clearTimeTask()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void AnimationDrop::clearTimeTask()
{
    byte Column, Row;
    pDisplay->indexToColumnAndRow(CurrentPixelIndex, Row, Column);
    // toggle current Pixel
    if(CurrentPixelIndex < DISPLAY_NUMBER_OF_PIXELS) { pDisplay->togglePixelFast(CurrentPixelIndex); }
    // increment row and check for out of bounds
    if(Row + 1 < DISPLAY_NUMBER_OF_ROWS) {
        // toggle Pixel in next row
        CurrentPixelIndex = pDisplay->columnAndRowToIndex(Column, Row + 1);
        pDisplay->togglePixelFast(CurrentPixelIndex);
    } else {
        // no more active pixels available
        if(setNextActivePixelIndex() == E_NOT_OK) { setStateToSetTime(); }
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
void AnimationDrop::setTimeTask()
{
    byte Column, Row;
    DisplayWords::Word CurrentWord = Words.getDisplayWordFast(ClockWordsTable[CurrenWordIndex]);

    pDisplay->indexToColumnAndRow(CurrentPixelIndex, Row, Column);

    // break condition
    if(Row >= CurrentWord.Row) {
        if(Column - Words.getDisplayWordColumnFast(ClockWordsTable[CurrenWordIndex]) >= CurrentWord.Length - 1) {
            if(setNextWordIndex() == E_NOT_OK) { 
                State = STATE_IDLE;
            } else { 
                CurrentPixelIndex = Words.getDisplayWordColumnFast(ClockWordsTable[CurrenWordIndex]);
                pDisplay->indexToColumnAndRow(CurrentPixelIndex, Row, Column);
            }
        } else {
            Column++;
            Row = 0;
        }
    } else {
        pDisplay->clearPixelFast(CurrentPixelIndex);
        Row++;
    }
    CurrentPixelIndex = pDisplay->columnAndRowToIndex(Column, Row);
    pDisplay->setPixelFast(CurrentPixelIndex);
} /* setTimeTask */


/******************************************************************************************************************************************************
  getNextActivePixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationDrop::setNextActivePixelIndex()
{
    for(int16_t Index = DISPLAY_NUMBER_OF_PIXELS - 1; Index >= 0; Index--) {
        if(pDisplay->getPixelFast(Index)) {
            CurrentPixelIndex = Index;
            return E_OK;
        }
    }
    return E_NOT_OK;
}


/******************************************************************************************************************************************************
  setNextWordIndex()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType AnimationDrop::setNextWordIndex()
{
    for(int8_t Index = CurrenWordIndex - 1; Index >= 0; Index--){
        if(ClockWordsTable[Index] != DisplayWords::WORD_NONE) {
            CurrenWordIndex = Index;
            return E_OK;
        }
    }
    return E_NOT_OK;
} /* setNextWordIndex */


/******************************************************************************************************************************************************
  setStateToSetTime()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void AnimationDrop::setStateToSetTime()
{
    CurrentPixelIndex = Words.getDisplayWordColumnFast(ClockWordsTable[CurrenWordIndex]);
    pDisplay->setPixelFast(CurrentPixelIndex);
    State = STATE_SET_TIME;
} /* setStateToSetTime */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 