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
//#define ANIMATION_WIPE_NUMBER_OF_SHIFT_CYCLES       (DISPLAY_CHARACTERS_NUMBER_OF_ROWS + DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS)

/* AnimationClockWipe parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   C L O C K   W I P E
******************************************************************************************************************************************************/
class AnimationClockWipe : public AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum SetPixelStateType {
        SET_PIXEL_STATE_RIGHT,
        SET_PIXEL_STATE_DOWN
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte Index{0u};
    SetPixelStateType SetPixelState{SET_PIXEL_STATE_DOWN};

    // functions
    void reset();
    void clearTimeTask();
    void setTimeTask();
    bool setNextIndex();
    void setPixelDown(byte, byte);
    void setPixelRight(byte, byte);


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockWipe() { }
    ~AnimationClockWipe() { }

    // get methods


    // set methods


    // methods
    void init();
    StdReturnType setTime(byte, byte);
    void task();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
