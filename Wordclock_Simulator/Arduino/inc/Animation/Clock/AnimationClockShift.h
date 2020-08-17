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
/**     \file       AnimationClockShift.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_SHIFT_H_
#define _ANIMATION_CLOCK_SHIFT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Transformation.h"
#include "AnimationClockCommon.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClockShift configuration parameter */
#define ANIMATION_CLOCK_SHIFT_HORIZONTAL              STD_ON
#define ANIMATION_CLOCK_SHIFT_VERTICAL                STD_OFF


#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_OFF && ANIMATION_CLOCK_SHIFT_VERTICAL == STD_OFF)
# error "AnimationClockShift: Exactly one switch must be set to STD_ON"
#endif

#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_ON && ANIMATION_CLOCK_SHIFT_VERTICAL == STD_ON)
# error "AnimationClockShift: ANIMATION_CLOCK_SHIFT_HORIZONTAL or ANIMATION_CLOCK_SHIFT_VERTICAL has to be STD_ON, but not both"
#endif


/* AnimationClockShift parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N C L O C K S H I F T
******************************************************************************************************************************************************/
class AnimationClockShift : public AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    Transformation wcTransformation;
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};

#if (ANIMATION_CLOCK_SHIFT_HORIZONTAL == STD_ON)
    byte CurrentColumn{0u};
#endif

#if (ANIMATION_CLOCK_SHIFT_VERTICAL == STD_ON)
    byte CurrentRow{0u};
#endif

    // functions
    void reset();
    void clearTimeTask();
    void setTimeTask();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockShift() : wcTransformation() { }
    ~AnimationClockShift() { }

    // get methods


    // set methods

    // methods
    void init();
    StdReturnType setClock(byte, byte);
    void task();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
