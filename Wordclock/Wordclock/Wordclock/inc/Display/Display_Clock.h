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
/**     \file       Display_Clock.h
 *      \brief      
 *
 *      \details    
 *                  
 *****************************************************************************************************************************************************/
#ifndef _DISPLAY_CLOCK_H_
#define _DISPLAY_CLOCK_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display_Word.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* DisplayClock configuration parameter */
#define DISPLAY_CLOCK_SHOW_IT_IS_PERMANENTLY			STD_ON

#define DISPLAY_CLOCK_NUMBER_OF_HOUR_MODES				2
#define DISPLAY_CLOCK_NUMBER_OF_HOURS					12
#define DISPLAY_CLOCK_MAX_NUMBER_OF_HOUR_WORDS			2

#define DISPLAY_CLOCK_MINUTE_STEP_IN_MINUTES			5
#define DISPLAY_CLOCK_NUMBER_OF_MINUTE_STEPS			12

/* evtl. ändern */
#define DISPLAY_CLOCK_MAX_NUMBER_OF_MINUTE_WORDS		3
 
/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/*  */
typedef enum {
    DISPLAY_CLOCK_MODE_WESSI,
    DISPLAY_CLOCK_MODE_OSSI,
    DISPLAY_CLOCK_MODE_RHEIN_RUHR,
    DISPLAY_CLOCK_MODE_SCHWABEN
} DisplayClockModesType;


typedef struct {
	byte HourMode;
	byte HourOffset;
	DisplayWordWordsType Words[DISPLAY_CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
} DisplayClockMinutesType;


/******************************************************************************************************************************************************
 *  CLASS  DisplayClock
******************************************************************************************************************************************************/
class DisplayClock
{
  private:
    DisplayClockModesType ClockMode;
	DisplayWord WcDisplayWord;
	static const DisplayWordWordsType ClockHoursTable[][DISPLAY_CLOCK_NUMBER_OF_HOURS][DISPLAY_CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
	static const DisplayClockMinutesType ClockMinutesTable[][DISPLAY_CLOCK_NUMBER_OF_MINUTE_STEPS];
	// functions


  public:
	DisplayClock() {};
	~DisplayClock() {};

	// get methods


	// set methods

	// methods
	stdReturnType showClock(byte, byte);
	//show();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
