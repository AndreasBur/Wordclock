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
/**     \file       AnimationClockDrop.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_DROP_H_
#define _ANIMATION_CLOCK_DROP_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "AnimationClockCommon.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationClockDrop configuration parameter */


/* AnimationClockDrop parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  AnimationClockDrop
******************************************************************************************************************************************************/
class AnimationClockDrop : public AnimationClockCommon
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    Clock* pClock;
    Display* pDisplay;
    DisplayWords Words;
    //byte CurrentPixelIndex;
    byte Row;
    byte Column;

    Clock::ClockWordsTableType ClockWordsTable;
    byte CurrenWordIndex;

    // functions
    void reset();
    stdReturnType setNextActivePixelIndex();
    void clearTimeTask();
    void setTimeTask();
    stdReturnType setNextWordIndex();
    void setStateToSetTime();
    stdReturnType setNextRow(byte);
    stdReturnType setNextColumn(byte);
    
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClockDrop();
    ~AnimationClockDrop();

	// get methods


	// set methods

	// methods
    void init(Display*, Clock*);
    stdReturnType setClock(byte, byte);
    void task();
    void show() { pDisplay->show(); }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
