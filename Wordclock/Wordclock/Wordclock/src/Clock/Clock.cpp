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
/**     \file       Template.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
 *****************************************************************************************************************************************************/
#define _TEMPLATE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
 *****************************************************************************************************************************************************/
#include "Clock.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
const DisplayWordsType Clock::ClockHoursTable[][CLOCK_NUMBER_OF_HOURS][CLOCK_MAX_NUMBER_OF_HOUR_WORDS] PROGMEM =
{
	{                                                                   // ClockHoursTable[0][] = hh:00
		{DISPLAY_WORD_HOUR_ZWOELF, DISPLAY_WORD_UHR },                  // 00:00
		{DISPLAY_WORD_HOUR_EIN,    DISPLAY_WORD_UHR },                  // 01:00
		{DISPLAY_WORD_HOUR_ZWEI,   DISPLAY_WORD_UHR },                  // 02:00
		{DISPLAY_WORD_HOUR_DREI,   DISPLAY_WORD_UHR },                  // 03:00
		{DISPLAY_WORD_HOUR_VIER,   DISPLAY_WORD_UHR },                  // 04:00
		{DISPLAY_WORD_HOUR_FUENF,  DISPLAY_WORD_UHR },                  // 05:00
		{DISPLAY_WORD_HOUR_SECHS,  DISPLAY_WORD_UHR },                  // 06:00
		{DISPLAY_WORD_HOUR_SIEBEN, DISPLAY_WORD_UHR },                  // 07:00
		{DISPLAY_WORD_HOUR_ACHT,   DISPLAY_WORD_UHR },                  // 08:00
		{DISPLAY_WORD_HOUR_NEUN,   DISPLAY_WORD_UHR },                  // 09:00
		{DISPLAY_WORD_HOUR_ZEHN,   DISPLAY_WORD_UHR },                  // 10:00
		{DISPLAY_WORD_HOUR_ELF,    DISPLAY_WORD_UHR }                   // 11:00
	},
	{                                                                   // ClockHoursTable[1][] = hh:mm
		{DISPLAY_WORD_HOUR_ZWOELF, DISPLAY_WORD_NONE },                 // 00:mm
		{DISPLAY_WORD_HOUR_EINS,   DISPLAY_WORD_NONE },                 // 01:mm
		{DISPLAY_WORD_HOUR_ZWEI,   DISPLAY_WORD_NONE },                 // 02:mm
		{DISPLAY_WORD_HOUR_DREI,   DISPLAY_WORD_NONE },                 // 03:mm
		{DISPLAY_WORD_HOUR_VIER,   DISPLAY_WORD_NONE },                 // 04:mm
		{DISPLAY_WORD_HOUR_FUENF,  DISPLAY_WORD_NONE },                 // 05:mm
		{DISPLAY_WORD_HOUR_SECHS,  DISPLAY_WORD_NONE },                 // 06:mm
		{DISPLAY_WORD_HOUR_SIEBEN, DISPLAY_WORD_NONE },                 // 07:mm
		{DISPLAY_WORD_HOUR_ACHT,   DISPLAY_WORD_NONE },                 // 08:mm
		{DISPLAY_WORD_HOUR_NEUN,   DISPLAY_WORD_NONE },                 // 09:mm
		{DISPLAY_WORD_HOUR_ZEHN,   DISPLAY_WORD_NONE },                 // 10:mm
		{DISPLAY_WORD_HOUR_ELF,    DISPLAY_WORD_NONE }                  // 11:mm
	}
};


const ClockMinutesType Clock::ClockMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM =
{
	{                                                                                         // ClockMinutesTable[0][] = WESSI
		{0},                                                                                  // 00
		{1, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 05
		{1, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 10
		{1, 0, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 15
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 20
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 25
		{1, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 30
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 35
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 40
		{1, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 45
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 50
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 55
	},

	{                                                                                         // ClockMinutesTable[1][] = OSSI
		{0},                                                                                  // 00
		{1, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 05
		{1, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 10
		{1, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 45
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 20
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 25
		{1, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 30
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 35
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 40
		{1, 1, {DISPLAY_WORD_DREIVIERTEL, DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 15
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 50
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 55
	},

	{                                                                                         // ClockMinutesTable[2][] = RHEIN-RUHR
		{0},                                                                                  // 00
		{1, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 05
		{1, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 10
		{1, 0, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 15
		{1, 0, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 20
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 25
		{1, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 30
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 35
		{1, 1, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 40
		{1, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 45
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 50
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 55
	},

	{                                                                                         // ClockMinutesTable[3][] = SCHWABEN
		{0},                                                                                  // 00
		{1, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 05
		{1, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 10
		{1, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 45
		{1, 0, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE   }},          // 20
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB   }},          // 25
		{1, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 30
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB   }},          // 35
		{1, 1, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 40
		{1, 1, {DISPLAY_WORD_DREIVIERTEL, DISPLAY_WORD_NONE, DISPLAY_WORD_NONE   }},          // 15
		{1, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 50
		{1, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE   }},          // 55
	}
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
//void init()
//{

//} /* init */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Clock::show(byte Hour, byte Minute)
{
	/* ----- Local Variables ---------------------------------------------- */
	//DisplayWordWordsType Words[WORD_NUMBER_OF_WORDS];
	//const ClockMinutesType* pMinutesTableEntry = (ClockMinutesType*) pgm_read_word(&ClockMinutesTable[ClockMode][Minute / 5]);
	ClockMinutesType MinutesTableEntry;
	DisplayWordsType HoursTableEntry[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];

	memcpy_P(&MinutesTableEntry, &ClockMinutesTable[ClockMode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(ClockMinutesType));
	memcpy_P(&HoursTableEntry, &ClockHoursTable[MinutesTableEntry.HourMode][Hour], CLOCK_MAX_NUMBER_OF_HOUR_WORDS);
	//const DisplayWordWordsType HoursTableEntry;


	/* ----- Implementation ----------------------------------------------- */
	if(Hour < 24 && Minute < 60) {
		/*  */
		if (CLOCK_SHOW_IT_IS_PERMANENTLY == STD_ON || Minute <= 4 || (Minute >= 30 && Minute <= 34)) {
			pDisplay->setWord(DISPLAY_WORD_ES);
			pDisplay->setWord(DISPLAY_WORD_IST);
		}

		for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_MINUTE_WORDS && MinutesTableEntry.Words[Index] != DISPLAY_WORD_NONE; Index++) {
			pDisplay->setWord(MinutesTableEntry.Words[Index]);
		}
		
		if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;
		Hour += MinutesTableEntry.HourOffset;                             // correct the hour offset from the minutes
		if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;

		for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_HOUR_WORDS && HoursTableEntry[Index] != DISPLAY_WORD_NONE; Index++) {
			pDisplay->setWord(HoursTableEntry[Index]);
		}

		return E_OK;
	} else {
		return E_NOT_OK;
	}
} /* show */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
 