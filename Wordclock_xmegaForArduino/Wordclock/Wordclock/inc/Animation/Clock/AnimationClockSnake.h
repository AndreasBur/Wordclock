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
/**     \file       AnimationClockSnake.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_SNAKE_H_
#define _ANIMATION_CLOCK_SNAKE_H_

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
/* AnimationClockSnake configuration parameter */
#define ANIMATION_CLOCK_SNAKE_LENGTH              8

/* AnimationClockSnake parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class AnimationClockSnake : public AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum SnakeStateType {
        SNAKE_STATE_TO_RIGHT,
        SNAKE_STATE_TO_LEFT
    };
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    SnakeStateType SnakeState;
    Clock::ClockWordsTableType ClockWordsTable;
    byte SnakeBeginIndex;
    byte SnakeEndIndex;

    // functions
    void reset();
    byte transformToSerpentine(byte, byte) const;
    byte transformToSerpentine(byte) const;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClockSnake();
    ~AnimationClockSnake();

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
