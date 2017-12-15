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
/**     \file       AnimationDrop.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_DROP_H_
#define _ANIMATION_DROP_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationDrop configuration parameter */


/* AnimationDrop parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  AnimationDrop
******************************************************************************************************************************************************/
class AnimationDrop
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
    enum StateType {
        STATE_NONE,
        STATE_UNINIT,
        STATE_IDLE,
        //STATE_READY,
        STATE_CLEAR_TIME,
        STATE_SET_TIME
    };

  private:
    Clock* pClock;
    Display* pDisplay;
    StateType State;
    byte CurrentPixelIndex;
    Clock::ClockWordsTableType ClockWordsTable;
    Display::WordIlluminationType CurrenWordIllumination;
    byte CurrenWordIndex;
    byte CurrentCharIndex;

    // functions
    void reset();
    stdReturnType setNextActivePixelIndex();
    void clearTimeTask();
    void setTimeTask();

  public:
    AnimationDrop();
    ~AnimationDrop();

	// get methods


	// set methods

	// methods
    void init(Display*, Clock*);
    stdReturnType setClock(byte, byte);
    void task();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
