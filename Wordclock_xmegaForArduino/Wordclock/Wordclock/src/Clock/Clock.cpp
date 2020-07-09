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
    {                                                                    // ClockHoursTable[0][] = hh:00 CLOCK_HOUR_MODE_FULL_HOUR
        {DisplayWords::WORD_HOUR_ZWOELF, DisplayWords::WORD_UHR},        // 00:00
        {DisplayWords::WORD_HOUR_EIN,    DisplayWords::WORD_UHR},        // 01:00
        {DisplayWords::WORD_HOUR_ZWEI,   DisplayWords::WORD_UHR},        // 02:00
        {DisplayWords::WORD_HOUR_DREI,   DisplayWords::WORD_UHR},        // 03:00
        {DisplayWords::WORD_HOUR_VIER,   DisplayWords::WORD_UHR},        // 04:00
        {DisplayWords::WORD_HOUR_FUENF,  DisplayWords::WORD_UHR},        // 05:00
        {DisplayWords::WORD_HOUR_SECHS,  DisplayWords::WORD_UHR},        // 06:00
        {DisplayWords::WORD_HOUR_SIEBEN, DisplayWords::WORD_UHR},        // 07:00
        {DisplayWords::WORD_HOUR_ACHT,   DisplayWords::WORD_UHR},        // 08:00
        {DisplayWords::WORD_HOUR_NEUN,   DisplayWords::WORD_UHR},        // 09:00
        {DisplayWords::WORD_HOUR_ZEHN,   DisplayWords::WORD_UHR},        // 10:00
        {DisplayWords::WORD_HOUR_ELF,    DisplayWords::WORD_UHR}         // 11:00
    },
    {                                                                    // ClockHoursTable[1][] = hh:mm CLOCK_HOUR_MODE_NO_FULL_HOUR
        {DisplayWords::WORD_HOUR_ZWOELF, DisplayWords::WORD_NONE},       // 00:mm
        {DisplayWords::WORD_HOUR_EINS,   DisplayWords::WORD_NONE},       // 01:mm
        {DisplayWords::WORD_HOUR_ZWEI,   DisplayWords::WORD_NONE},       // 02:mm
        {DisplayWords::WORD_HOUR_DREI,   DisplayWords::WORD_NONE},       // 03:mm
        {DisplayWords::WORD_HOUR_VIER,   DisplayWords::WORD_NONE},       // 04:mm
        {DisplayWords::WORD_HOUR_FUENF,  DisplayWords::WORD_NONE},       // 05:mm
        {DisplayWords::WORD_HOUR_SECHS,  DisplayWords::WORD_NONE},       // 06:mm
        {DisplayWords::WORD_HOUR_SIEBEN, DisplayWords::WORD_NONE},       // 07:mm
        {DisplayWords::WORD_HOUR_ACHT,   DisplayWords::WORD_NONE},       // 08:mm
        {DisplayWords::WORD_HOUR_NEUN,   DisplayWords::WORD_NONE},       // 09:mm
        {DisplayWords::WORD_HOUR_ZEHN,   DisplayWords::WORD_NONE},       // 10:mm
        {DisplayWords::WORD_HOUR_ELF,    DisplayWords::WORD_NONE}        // 11:mm
    }
};


const Clock::MinutesType Clock::MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM
{
    {                                                                                                                             // ClockMinutesTable[0][] = WESSI
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
    {                                                                                                                             // ClockMinutesTable[1][] = OSSI
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
    {                                                                                                                             // ClockMinutesTable[2][] = RHEIN-RUHR
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
    {                                                                                                                             // ClockMinutesTable[3][] = SCHWABEN
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
  Constructor of Clock
******************************************************************************************************************************************************/
/*! \brief          Template Constructor
 *  \details        Instantiation of the Template library
 *
 *  \return         -
******************************************************************************************************************************************************/
Clock::Clock(ModesType sMode)
{
    Mode = sMode;
} /* Clock */


/******************************************************************************************************************************************************
  Destructor of Clock
******************************************************************************************************************************************************/
Clock::~Clock()
{

} /* ~Clock */

/******************************************************************************************************************************************************
  Singleton Instance of Clock
******************************************************************************************************************************************************/
Clock& Clock::getInstance()
{
    static Clock SingletonInstance(Clock::MODE_WESSI);
    return SingletonInstance;
}


/******************************************************************************************************************************************************
  getClockWords()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType Clock::getClockWords(byte Hour, byte Minute, ClockWords& ClockWords) const
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType ReturnValue{E_NOT_OK};

    /* ----- Implementation ----------------------------------------------- */
    if(Hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && Minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        ReturnValue = E_OK;
        /* show IT IS permanently or only to full and half hour */
#if (CLOCK_SHOW_IT_IS_PERMANENTLY == STD_ON)
        ClockWords.setShowItIs(true);
#else
        ClockWords.setShowItIs(calculateItIs(Minute));
#endif
        MinutesTableElementType MinutesTableElement = getMinutesTableElement(Minute);
        Hour = transform24hTo12hFormat(Hour);
        // correct the hour offset from the minutes and take care of overflow
        Hour = (Hour + MinutesTableElement.HourOffset) % CLOCK_NUMBER_OF_HOURS;

        ClockWords.setHourWords(getHoursTableElement(MinutesTableElement.HourMode, Hour));
        ClockWords.setMinuteWords(MinutesTableElement.Words);
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
StdReturnType Clock::getClockWords(byte Hour, byte Minute, ClockWordsListType& ClockWordsList) const
{
    /* ----- Local Variables ---------------------------------------------- */
    ClockWords ClockWords;

    /* ----- Implementation ----------------------------------------------- */
    if(getClockWords(Hour, Minute, ClockWords) == E_OK) {
        ClockWordsList = ClockWords.getWordsList();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getClockWords */


/******************************************************************************************************************************************************
  setClock()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType Clock::setClock(byte Hour, byte Minute)
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType ReturnValue{E_OK};
    ClockWords ClockWords;

    /* ----- Implementation ----------------------------------------------- */
    if(getClockWords(Hour, Minute, ClockWords) == E_NOT_OK) ReturnValue = E_NOT_OK;

    if(ClockWords.getShowItIs()) {
        if(Display::getInstance().setWord(DisplayWords::WORD_ES) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(Display::getInstance().setWord(DisplayWords::WORD_IST) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    for(byte Index = 0; Index < ClockWords.getHourWords().size() && ClockWords.getHourWord(Index) != DisplayWords::WORD_NONE; Index++) {
        if(Display::getInstance().setWord(ClockWords.getHourWord(Index)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    for(byte Index = 0; Index < ClockWords.getMinuteWord(Index) && ClockWords.getMinuteWord(Index) != DisplayWords::WORD_NONE; Index++) {
        if(Display::getInstance().setWord(ClockWords.getMinuteWord(Index)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setClock */


/******************************************************************************************************************************************************
  setClockFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Clock::setClockFast(byte Hour, byte Minute)
{
    ClockWords ClockWords;

    if(getClockWords(Hour, Minute, ClockWords) == E_OK) {
        if(ClockWords.getShowItIs()) {
            Display::getInstance().setWordFast(DisplayWords::WORD_ES);
            Display::getInstance().setWordFast(DisplayWords::WORD_IST);
        }
        for(byte Index = 0; Index < ClockWords.getHourWords().size() && ClockWords.getHourWord(Index) != DisplayWords::WORD_NONE; Index++) {
            Display::getInstance().setWordFast(ClockWords.getHourWord(Index));
        }
        for(byte Index = 0; Index < ClockWords.getMinuteWord(Index) && ClockWords.getMinuteWord(Index) != DisplayWords::WORD_NONE; Index++) {
            Display::getInstance().setWordFast(ClockWords.getMinuteWord(Index));
        }
    }
} /* setClockFast */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
