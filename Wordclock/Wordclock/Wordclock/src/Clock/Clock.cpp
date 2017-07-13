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
/**     \file       Clock.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
 *****************************************************************************************************************************************************/
#define _CLOCK_SOURCE_

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
    {                                                                   // ClockHoursTable[0][] = hh:00 CLOCK_HOUR_MODE_FULL_HOUR
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
    {                                                                   // ClockHoursTable[1][] = hh:mm CLOCK_HOUR_MODE_NO_FULL_HOUR
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
    {																												   // ClockMinutesTable[0][] = WESSI
        {CLOCK_HOUR_MODE_FULL_HOUR},																				   // 00
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 05
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 10
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 15
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 20
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 25
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 30
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 35
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 40
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 45
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 50
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 55
    },

    {																												   // ClockMinutesTable[1][] = OSSI
        {CLOCK_HOUR_MODE_FULL_HOUR},																				   // 00
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 05
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 10
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 45
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 20
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 25
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 30
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 35
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 40
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_DREIVIERTEL, DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 15
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 50
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 55
    },

    {																												   // ClockMinutesTable[2][] = RHEIN-RUHR
        {CLOCK_HOUR_MODE_FULL_HOUR},																				   // 00
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 05
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 10
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 15
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 20
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 25
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 30
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 35
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 40
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 45
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 50
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 55
    },

    {																												   // ClockMinutesTable[3][] = SCHWABEN
        {CLOCK_HOUR_MODE_FULL_HOUR},																				   // 00
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 05
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 10
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_VIERTEL,     DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 45
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 0, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_NACH, DISPLAY_WORD_NONE }},          // 20
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},          // 25
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_HALB,        DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 30
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},          // 35
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZWANZIG,     DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 40
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_DREIVIERTEL, DISPLAY_WORD_NONE, DISPLAY_WORD_NONE }},          // 15
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_ZEHN,        DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 50
        {CLOCK_HOUR_MODE_NO_FULL_HOUR, 1, {DISPLAY_WORD_FUENF,       DISPLAY_WORD_VOR,  DISPLAY_WORD_NONE }},          // 55
    }
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  CONSTRUCTOR OF Clock
******************************************************************************************************************************************************/
/*! \brief          Template Constructor
 *  \details        Instantiation of the Template library
 *
 *  \return         -
******************************************************************************************************************************************************/
Clock::Clock(Display* Display, ClockModesType sMode)
{
	if(Display != NULL) {
		pDisplay = Display;
	}
	Mode = sMode;
} /* Clock */


/******************************************************************************************************************************************************
  DESTRUCTOR OF Clock
******************************************************************************************************************************************************/
Clock::~Clock()
{

} /* ~Clock */


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
	stdReturnType ReturnValue = E_NOT_OK;
    ClockMinutesType MinutesTableEntry;
    DisplayWordsType HoursTableEntry[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];

    /* ----- Implementation ----------------------------------------------- */
    if(Hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && Minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
		ReturnValue = E_OK;
        /* show IT IS permanently or only to full and half hour */
        if (CLOCK_SHOW_IT_IS_PERMANENTLY == STD_ON	||
			Minute < CLOCK_MINUTE_STEP_IN_MINUTES	||
			(Minute >= (CLOCK_NUMBER_OF_MINUTES_PER_HOUR/2) && Minute < (CLOCK_NUMBER_OF_MINUTES_PER_HOUR/2) + CLOCK_MINUTE_STEP_IN_MINUTES)) 
		{
            if(pDisplay->setWord(DISPLAY_WORD_ES) == E_NOT_OK) ReturnValue = E_NOT_OK;
            if(pDisplay->setWord(DISPLAY_WORD_IST) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }

		memcpy_P(&MinutesTableEntry, &ClockMinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(ClockMinutesType));

        for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_MINUTE_WORDS && MinutesTableEntry.Words[Index] != DISPLAY_WORD_NONE; Index++) {
            if(pDisplay->setWord(MinutesTableEntry.Words[Index]) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
        
        if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;
        Hour += MinutesTableEntry.HourOffset;                             // correct the hour offset from the minutes
        if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;

		memcpy_P(&HoursTableEntry, &ClockHoursTable[MinutesTableEntry.HourMode][Hour], sizeof(DisplayWordsType) * CLOCK_MAX_NUMBER_OF_HOUR_WORDS);

        for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_HOUR_WORDS && HoursTableEntry[Index] != DISPLAY_WORD_NONE; Index++) {
            if(pDisplay->setWord(HoursTableEntry[Index]) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }        
    } else {
        ReturnValue = E_NOT_OK;
    }
	return ReturnValue;
} /* show */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
 