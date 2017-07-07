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
/**     \file       Clock.h
 *      \brief      
 *
 *      \details    
 *                  
 *****************************************************************************************************************************************************/
#ifndef _CLOCK_H_
#define _CLOCK_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Clock configuration parameter */
#define CLOCK_SHOW_IT_IS_PERMANENTLY            STD_ON

#define CLOCK_NUMBER_OF_HOUR_MODES              2
#define CLOCK_NUMBER_OF_HOURS                   12
#define CLOCK_MAX_NUMBER_OF_HOUR_WORDS          2

#define CLOCK_MINUTE_STEP_IN_MINUTES            5
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12

/* evtl. �ndern */
#define CLOCK_MAX_NUMBER_OF_MINUTE_WORDS        3
 

/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/*  */
typedef enum {
    CLOCK_MODE_WESSI,
    CLOCK_MODE_OSSI,
    CLOCK_MODE_RHEIN_RUHR,
    CLOCK_MODE_SCHWABEN
} ClockModesType;


typedef struct {
    byte HourMode;
    byte HourOffset;
    DisplayWordsType Words[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
} ClockMinutesType;


/******************************************************************************************************************************************************
 *  CLASS  Clock
******************************************************************************************************************************************************/
class Clock
{
  private:
    Display* pDisplay;
    ClockModesType Mode;

    static const DisplayWordsType ClockHoursTable[][CLOCK_NUMBER_OF_HOURS][CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
    static const ClockMinutesType ClockMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    // functions


  public:
    Clock(Display*, ClockModesType);
    ~Clock();

    // get methods
	ClockModesType getMode() { return Mode; }

    // set methods
	void setMode(ClockModesType sMode) { Mode = sMode; }

    // methods
    stdReturnType show(byte, byte);
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
