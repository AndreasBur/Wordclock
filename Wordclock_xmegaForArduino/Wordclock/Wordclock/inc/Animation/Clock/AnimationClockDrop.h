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
 *  C L A S S   A N I M A T I O N   C L O C K   D R O P
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
    DisplayWords Words;
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte Row{0u};
    byte Column{0u};
    byte CurrentWordIndex{0u};

    // functions
    void reset();
    StdReturnType setNextActivePixelIndex();
    void clearTimeTask();
    void setTimeTask();
    StdReturnType setNextWordIndex();
    void setStateToSetTime();
    StdReturnType setNextRow(byte);
    StdReturnType setNextColumn(byte);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockDrop() : Words() { }
    ~AnimationClockDrop() { }

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
