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
/**     \file       AnimationClockWipe.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_WIPE_H_
#define _ANIMATION_CLOCK_WIPE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "AnimationClockCommon.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClockWipe configuration parameter */
#define ANIMATION_WIRE_NUMBER_OF_SHIFT_CYCLES       (DISPLAY_CHARACTERS_NUMBER_OF_ROWS + DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS)

/* AnimationClockWipe parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class AnimationClockWipe : public AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_NONE,
        STATE_UNINIT,
        STATE_IDLE,
        //STATE_READY,
        STATE_CLEAR_TIME,
        STATE_SET_TIME
    };

    enum SetPixelStateType {
        STATE_SET_PIXEL_RIGHT,
        STATE_SET_PIXEL_DOWN
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    Clock* pClock;
    Display* pDisplay;
    StateType State;
    Clock::ClockWordsTableType ClockWordsTable;
    byte Index;
    SetPixelStateType SetPixelState;

    // functions
    void reset();
    void clearTimeTask();
    void setTimeTask();
    boolean setNextIndex();
    void setPixelDown(byte, byte);
    void setPixelRight(byte, byte);


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClockWipe();
    ~AnimationClockWipe();

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
