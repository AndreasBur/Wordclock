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
/**     \file       AnimationClockCommon.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_COMMON_H_
#define _ANIMATION_CLOCK_COMMON_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClockCommon configuration parameter */


/* AnimationClockCommon parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N C L O C K C O M M O N
******************************************************************************************************************************************************/
class AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_NONE,
        STATE_UNINIT,
        STATE_IDLE,
        STATE_CLEAR_TIME,
        STATE_SET_TIME
    };
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:


/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  protected:
      StateType State;
      Clock* pClock;
      Display* pDisplay;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClockCommon();
    ~AnimationClockCommon();

	// get methods
    StateType getState() const { return State; }

	// set methods

	// methods
    void init(Display*, Clock*, StateType);
    void show() { pDisplay->show(); }
    boolean isPixelPartOfClockWords(const Clock::ClockWordsTableType, byte, byte) const;
    boolean isPixelPartOfClockWords(const Clock::ClockWordsTableType, byte) const;
    virtual void task() = 0;
    virtual stdReturnType setClock(byte, byte) = 0;
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
