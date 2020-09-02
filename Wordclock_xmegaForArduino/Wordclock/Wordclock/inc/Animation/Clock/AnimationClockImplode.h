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
/**     \file       AnimationClockImplode.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_IMPLODE_H_
#define _ANIMATION_CLOCK_IMPLODE_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "AnimationClockCommon.h"
#include "DisplayPixels.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationClockImplode configuration parameter */


/* AnimationClockImplode parameter */
#define ANIMATION_CLOCK_SHIFT_COUNTER_MAX_VALUE       5u


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N C L O C K T E L E T Y P E
******************************************************************************************************************************************************/
class AnimationClockImplode : public AnimationClockCommon
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    DisplayWords Words;
    //ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte ShiftCounter{0u};
    byte Hour{0u};
    byte Minute{0u};

    // functions
    void reset();
    void clearTimeTask();
    void shiftQuadrants();

    void shiftQuadrantUpperLeft();
    void shiftQuadrantUpperRight();
    void shiftQuadrantLowerLeft();
    void shiftQuadrantLowerRight();

    void shiftDownRight(byte, byte);
    void shiftDownLeft(byte, byte);
    void shiftUpRight(byte, byte);
    void shiftUpLeft(byte, byte);

    void clearOldAndSetNewPixel(byte, byte, byte, byte);

//    byte shiftLeft(byte Column) { return Column > (DISPLAY_NUMBER_OF_COLUMNS / 2u) ? --Column : Column; }
//    byte shiftRight(byte Column) { return Column < (DISPLAY_NUMBER_OF_COLUMNS / 2u) ? ++Column : Column; }
//    byte shiftDown(byte Row) { return Row < (DISPLAY_NUMBER_OF_ROWS / 2u) ? ++Row : Row; }
//    byte shiftUp(byte Row) { return Row > (DISPLAY_NUMBER_OF_ROWS / 2u) ?  --Row : Row; }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockImplode() : Words() { }
    ~AnimationClockImplode() { }

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
