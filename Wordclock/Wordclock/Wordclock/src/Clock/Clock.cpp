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
const Clock::HoursType Clock::HoursTable[][CLOCK_NUMBER_OF_HOURS] PROGMEM
{
    {                                                                   // ClockHoursTable[0][] = hh:00 CLOCK_HOUR_MODE_FULL_HOUR
        {DisplayWords::WORD_HOUR_ZWOELF, DisplayWords::WORD_UHR },                  // 00:00
        {DisplayWords::WORD_HOUR_EIN,    DisplayWords::WORD_UHR },                  // 01:00
        {DisplayWords::WORD_HOUR_ZWEI,   DisplayWords::WORD_UHR },                  // 02:00
        {DisplayWords::WORD_HOUR_DREI,   DisplayWords::WORD_UHR },                  // 03:00
        {DisplayWords::WORD_HOUR_VIER,   DisplayWords::WORD_UHR },                  // 04:00
        {DisplayWords::WORD_HOUR_FUENF,  DisplayWords::WORD_UHR },                  // 05:00
        {DisplayWords::WORD_HOUR_SECHS,  DisplayWords::WORD_UHR },                  // 06:00
        {DisplayWords::WORD_HOUR_SIEBEN, DisplayWords::WORD_UHR },                  // 07:00
        {DisplayWords::WORD_HOUR_ACHT,   DisplayWords::WORD_UHR },                  // 08:00
        {DisplayWords::WORD_HOUR_NEUN,   DisplayWords::WORD_UHR },                  // 09:00
        {DisplayWords::WORD_HOUR_ZEHN,   DisplayWords::WORD_UHR },                  // 10:00
        {DisplayWords::WORD_HOUR_ELF,    DisplayWords::WORD_UHR }                   // 11:00
    },
    {                                                                   // ClockHoursTable[1][] = hh:mm CLOCK_HOUR_MODE_NO_FULL_HOUR
        {DisplayWords::WORD_HOUR_ZWOELF, DisplayWords::WORD_NONE },                 // 00:mm
        {DisplayWords::WORD_HOUR_EINS,   DisplayWords::WORD_NONE },                 // 01:mm
        {DisplayWords::WORD_HOUR_ZWEI,   DisplayWords::WORD_NONE },                 // 02:mm
        {DisplayWords::WORD_HOUR_DREI,   DisplayWords::WORD_NONE },                 // 03:mm
        {DisplayWords::WORD_HOUR_VIER,   DisplayWords::WORD_NONE },                 // 04:mm
        {DisplayWords::WORD_HOUR_FUENF,  DisplayWords::WORD_NONE },                 // 05:mm
        {DisplayWords::WORD_HOUR_SECHS,  DisplayWords::WORD_NONE },                 // 06:mm
        {DisplayWords::WORD_HOUR_SIEBEN, DisplayWords::WORD_NONE },                 // 07:mm
        {DisplayWords::WORD_HOUR_ACHT,   DisplayWords::WORD_NONE },                 // 08:mm
        {DisplayWords::WORD_HOUR_NEUN,   DisplayWords::WORD_NONE },                 // 09:mm
        {DisplayWords::WORD_HOUR_ZEHN,   DisplayWords::WORD_NONE },                 // 10:mm
        {DisplayWords::WORD_HOUR_ELF,    DisplayWords::WORD_NONE }                  // 11:mm
    }
};


const Clock::MinutesType Clock::MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM
{
    {                                                                                                                  // ClockMinutesTable[0][] = WESSI
        {HOUR_MODE_FULL_HOUR,    0, {DisplayWords::WORD_NONE,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 00
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 05
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 10
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 15
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 20
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 25
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_HALB,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 30
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 35
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 40
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 45
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 50
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 55
    },

    {                                                                                                                  // ClockMinutesTable[1][] = OSSI
        {HOUR_MODE_FULL_HOUR,    0, {DisplayWords::WORD_NONE,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 00
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 05
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 10
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 45
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 20
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 25
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_HALB,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 30
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 35
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 40
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_DREIVIERTEL, DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 15
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 50
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 55
    },

    {                                                                                                                  // ClockMinutesTable[2][] = RHEIN-RUHR
        {HOUR_MODE_FULL_HOUR,    0, {DisplayWords::WORD_NONE,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 00
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 05
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 10
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 15
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZWANZIG,     DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 20
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 25
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_HALB,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 30
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 35
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZWANZIG,     DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 40
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 45
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 50
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 55
    },

    {                                                                                                                  // ClockMinutesTable[3][] = SCHWABEN
        {HOUR_MODE_FULL_HOUR,    0, {DisplayWords::WORD_NONE,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 00
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 05
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 10
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_VIERTEL,     DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 45
        {HOUR_MODE_NO_FULL_HOUR, 0, {DisplayWords::WORD_ZWANZIG,     DisplayWords::WORD_NACH, DisplayWords::WORD_NONE}},          // 20
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_HALB}},          // 25
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_HALB,        DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 30
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_NACH, DisplayWords::WORD_HALB}},          // 35
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZWANZIG,     DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 40
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_DREIVIERTEL, DisplayWords::WORD_NONE, DisplayWords::WORD_NONE}},          // 15
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_ZEHN,        DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 50
        {HOUR_MODE_NO_FULL_HOUR, 1, {DisplayWords::WORD_FUENF,       DisplayWords::WORD_VOR,  DisplayWords::WORD_NONE}},          // 55
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
Clock::Clock(Display* Display, ModesType sMode)
{
    pDisplay = Display;
    Mode = sMode;
} /* Clock */


/******************************************************************************************************************************************************
  DESTRUCTOR OF Clock
******************************************************************************************************************************************************/
Clock::~Clock()
{

} /* ~Clock */


/******************************************************************************************************************************************************
  getClockWords()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Clock::getClockWords(byte Hour, byte Minute, ClockWordsType* ClockWords)
{
    /* ----- Local Variables ---------------------------------------------- */
    stdReturnType ReturnValue{E_NOT_OK};

    /* ----- Implementation ----------------------------------------------- */
    if(Hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && Minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        ReturnValue = E_OK;
        /* show IT IS permanently or only to full and half hour */
#if (CLOCK_SHOW_IT_IS_PERMANENTLY == STD_ON)
        ClockWords->ShowItIs = true;
#else
        if (Minute < CLOCK_MINUTE_STEP_IN_MINUTES || (Minute >= (CLOCK_NUMBER_OF_MINUTES_PER_HOUR/2) && 
            Minute < (CLOCK_NUMBER_OF_MINUTES_PER_HOUR/2) + CLOCK_MINUTE_STEP_IN_MINUTES)) 
        {
            ClockWords->ShowItIs = true;
        } else {
            ClockWords->ShowItIs = false;
        }
#endif
        MinutesTableEntryType MinutesTableEntry = getMinutesTableEntry(Mode, Minute);

        for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_MINUTE_WORDS; Index++) {
            ClockWords->MinuteWords[Index] = MinutesTableEntry.Words[Index];
        }

        if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;
        Hour += MinutesTableEntry.HourOffset;                             // correct the hour offset from the minutes
        if(Hour >= CLOCK_NUMBER_OF_HOURS) Hour -= CLOCK_NUMBER_OF_HOURS;

        HoursTableEntryType HoursTableEntry = getHoursTableEntry(MinutesTableEntry.HourMode, Hour);

        for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_HOUR_WORDS; Index++) {
            ClockWords->HourWords[Index] = HoursTableEntry.Words[Index];
        }
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getClockWords */


/******************************************************************************************************************************************************
  getClockWords()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Clock::getClockWords(byte Hour, byte Minute, ClockWordsTableType ClockWordsTable)
{
    ClockWordsType ClockWords;
    stdReturnType ReturnValue{E_OK};

    if(getClockWords(Hour, Minute, &ClockWords) == E_NOT_OK) ReturnValue = E_NOT_OK;

    if(ClockWords.ShowItIs) {
        ClockWordsTable[0] = DisplayWords::WORD_ES;
        ClockWordsTable[1] = DisplayWords::WORD_IST;
    } else {
        ClockWordsTable[0] = DisplayWords::WORD_NONE;
        ClockWordsTable[1] = DisplayWords::WORD_NONE;
    }

    ClockWordsTable[2] = ClockWords.MinuteWords[0];
    ClockWordsTable[3] = ClockWords.MinuteWords[1];
    ClockWordsTable[4] = ClockWords.MinuteWords[2];
    ClockWordsTable[5] = ClockWords.HourWords[0];
    ClockWordsTable[6] = ClockWords.HourWords[1];

    return ReturnValue;

} /* getClockWords */


/******************************************************************************************************************************************************
  compareClockWords()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean Clock::compareClockWords(ClockWordsType* ClockWords1, ClockWordsType* ClockWords2) const
{
    if( ClockWords1->HourWords[0] == ClockWords2->HourWords[0]      &&
        ClockWords1->HourWords[1] == ClockWords2->HourWords[1]      &&
        ClockWords1->MinuteWords[0] == ClockWords2->MinuteWords[0]  &&
        ClockWords1->MinuteWords[1] == ClockWords2->MinuteWords[1]  &&
        ClockWords1->MinuteWords[2] == ClockWords2->MinuteWords[2]  )
    {
        return true;
    } else {
        return false;
    }
} /* compareClockWords */

/******************************************************************************************************************************************************
  setClock()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Clock::setClock(byte Hour, byte Minute)
{
    /* ----- Local Variables ---------------------------------------------- */
    stdReturnType ReturnValue{E_OK};
    ClockWordsType ClockWords;

    /* ----- Implementation ----------------------------------------------- */
    if(getClockWords(Hour, Minute, &ClockWords) == E_NOT_OK) ReturnValue = E_NOT_OK;

    if(ClockWords.ShowItIs) {
        if(pDisplay->setWord(DisplayWords::WORD_ES) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(pDisplay->setWord(DisplayWords::WORD_IST) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }

    for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_HOUR_WORDS && ClockWords.HourWords[Index] != DisplayWords::WORD_NONE; Index++) {
        if(pDisplay->setWord(ClockWords.HourWords[Index]) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }

    for(byte Index = 0; Index < CLOCK_MAX_NUMBER_OF_MINUTE_WORDS && ClockWords.MinuteWords[Index] != DisplayWords::WORD_NONE; Index++) {
        if(pDisplay->setWord(ClockWords.MinuteWords[Index]) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* show */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
