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
const Clock::HourType Clock::kHoursTable[][CLOCK_NUMBER_OF_HOURS] PROGMEM
{
    {                                                                    // ClockHoursTable[0][] = hh:00 CLOCK_HOUR_MODE_FULL_HOUR
        {DisplayWords::kWordHourZwoelf, DisplayWords::kWordUhr},        // 00:00
        {DisplayWords::kWordHourEin,    DisplayWords::kWordUhr},        // 01:00
        {DisplayWords::kWordHourZwei,   DisplayWords::kWordUhr},        // 02:00
        {DisplayWords::kWordHourDrei,   DisplayWords::kWordUhr},        // 03:00
        {DisplayWords::kWordHourVier,   DisplayWords::kWordUhr},        // 04:00
        {DisplayWords::kWordHourFuenf,  DisplayWords::kWordUhr},        // 05:00
        {DisplayWords::kWordHourSechs,  DisplayWords::kWordUhr},        // 06:00
        {DisplayWords::kWordHourSieben, DisplayWords::kWordUhr},        // 07:00
        {DisplayWords::kWordHourAcht,   DisplayWords::kWordUhr},        // 08:00
        {DisplayWords::kWordHourNeun,   DisplayWords::kWordUhr},        // 09:00
        {DisplayWords::kWordHourZehn,   DisplayWords::kWordUhr},        // 10:00
        {DisplayWords::kWordHourElf,    DisplayWords::kWordUhr}         // 11:00
    },
    {                                                                    // ClockHoursTable[1][] = hh:mm CLOCK_HOUR_MODE_NO_FULL_HOUR
        {DisplayWords::kWordHourZwoelf, DisplayWords::kWordNone},       // 00:mm
        {DisplayWords::kWordHourEins,   DisplayWords::kWordNone},       // 01:mm
        {DisplayWords::kWordHourZwei,   DisplayWords::kWordNone},       // 02:mm
        {DisplayWords::kWordHourDrei,   DisplayWords::kWordNone},       // 03:mm
        {DisplayWords::kWordHourVier,   DisplayWords::kWordNone},       // 04:mm
        {DisplayWords::kWordHourFuenf,  DisplayWords::kWordNone},       // 05:mm
        {DisplayWords::kWordHourSechs,  DisplayWords::kWordNone},       // 06:mm
        {DisplayWords::kWordHourSieben, DisplayWords::kWordNone},       // 07:mm
        {DisplayWords::kWordHourAcht,   DisplayWords::kWordNone},       // 08:mm
        {DisplayWords::kWordHourNeun,   DisplayWords::kWordNone},       // 09:mm
        {DisplayWords::kWordHourZehn,   DisplayWords::kWordNone},       // 10:mm
        {DisplayWords::kWordHourElf,    DisplayWords::kWordNone}        // 11:mm
    }
};


const Clock::MinuteType Clock::kMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM
{
    {                                                                                                                              // ClockMinutesTable[0][] = WESSI
        {kHourModeFullHour,    0u, {DisplayWords::kWordNone,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 00
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 05
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 10
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordViertel,     DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 15
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 20
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 25
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordHalb,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 30
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 35
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 40
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordViertel,     DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 45
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 50
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[1][] = OSSI
        {kHourModeFullHour,    0u, {DisplayWords::kWordNone,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 00
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 05
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 10
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordViertel,     DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 15
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 20
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 25
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordHalb,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 30
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 35
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 40
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordDreiviertel, DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 45
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 50
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[2][] = RHEIN-RUHR
        {kHourModeFullHour,    0u, {DisplayWords::kWordNone,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 00
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 05
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 10
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordViertel,     DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 15
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZwanzig,     DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 20
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 25
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordHalb,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 30
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 35
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZwanzig,     DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 40
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordViertel,     DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 45
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 50
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[3][] = SCHWABEN
        {kHourModeFullHour,    0u, {DisplayWords::kWordNone,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 00
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 05
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZehn,        DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 10
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordViertel,     DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 15
        {kHourModeNoFullHour, 0u, {DisplayWords::kWordZwanzig,     DisplayWords::kWordNach, DisplayWords::kWordNone}},          // 20
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordHalb}},          // 25
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordHalb,        DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 30
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordNach, DisplayWords::kWordHalb}},          // 35
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZwanzig,     DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 40
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordDreiviertel, DisplayWords::kWordNone, DisplayWords::kWordNone}},          // 45
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordZehn,        DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 50
        {kHourModeNoFullHour, 1u, {DisplayWords::kWordFuenf,       DisplayWords::kWordVor,  DisplayWords::kWordNone}},          // 55
    }
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getClockWords()
******************************************************************************************************************************************************/
StdReturnType Clock::GetClockWords(byte hour, byte minute, ClockWords& clock_words) const
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType return_value{kENotOk};

    /* ----- Implementation ----------------------------------------------- */
    if(hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        return_value = kEOk;
        /* Show "it is" permanently, or only at the full and the half hour. The switch this
           was is now the default of the setting; what a running clock does is asked here
           rather than compiled in. */
        clock_words.SetShowItIs(show_it_is_permanently_ || CalculateItIs(minute));
        MinuteTableElementType minutes_table_element = GetMinutesTableElement(minute);
        hour = Transform24hTo12hFormat(hour);
        // correct the hour offset from the minutes and take care of overflow
        hour = (hour + minutes_table_element.hour_offset) % CLOCK_NUMBER_OF_HOURS;

        clock_words.SetHourWords(GetHoursTableElement(minutes_table_element.hour_mode, hour));
        clock_words.SetMinuteWords(minutes_table_element.words);
    } else {
        return_value = kENotOk;
    }
    return return_value;
} /* getClockWords */


/******************************************************************************************************************************************************
  getClockWords()
******************************************************************************************************************************************************/
StdReturnType Clock::GetClockWords(byte hour, byte minute, ClockWordsListType& clock_words_list) const
{
    /* ----- Local Variables ---------------------------------------------- */
    ClockWords clock_words;

    /* ----- Implementation ----------------------------------------------- */
    if(GetClockWords(hour, minute, clock_words) == kEOk) {
        clock_words_list = clock_words.GetWordsList();
        return kEOk;
    } else {
        return kENotOk;
    }
} /* getClockWords */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
StdReturnType Clock::SetTime(byte hour, byte minute) const
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType return_value{kEOk};
    ClockWords clock_words;

    /* ----- Implementation ----------------------------------------------- */
    if(GetClockWords(hour, minute, clock_words) == kENotOk) return_value = kENotOk;

    if(clock_words.GetShowItIs()) {
        if(Display::GetInstance().SetWord(DisplayWords::kWordEs) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().SetWord(DisplayWords::kWordIst) == kENotOk) return_value = kENotOk;
    }
    for(byte index = 0u; index < clock_words.GetHourWords().size() && clock_words.GetHourWord(index) != DisplayWords::kWordNone; index++) {
        if(Display::GetInstance().SetWord(clock_words.GetHourWord(index)) == kENotOk) return_value = kENotOk;
    }
    for(byte index = 0u; index < clock_words.GetMinuteWords().size() && clock_words.GetMinuteWord(index) != DisplayWords::kWordNone; index++) {
        if(Display::GetInstance().SetWord(clock_words.GetMinuteWord(index)) == kENotOk) return_value = kENotOk;
    }
    return return_value;
} /* setTime */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
