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
        {DisplayWords::WordIdType::kHourZwoelf, DisplayWords::WordIdType::kUhr},        // 00:00
        {DisplayWords::WordIdType::kHourEin,    DisplayWords::WordIdType::kUhr},        // 01:00
        {DisplayWords::WordIdType::kHourZwei,   DisplayWords::WordIdType::kUhr},        // 02:00
        {DisplayWords::WordIdType::kHourDrei,   DisplayWords::WordIdType::kUhr},        // 03:00
        {DisplayWords::WordIdType::kHourVier,   DisplayWords::WordIdType::kUhr},        // 04:00
        {DisplayWords::WordIdType::kHourFuenf,  DisplayWords::WordIdType::kUhr},        // 05:00
        {DisplayWords::WordIdType::kHourSechs,  DisplayWords::WordIdType::kUhr},        // 06:00
        {DisplayWords::WordIdType::kHourSieben, DisplayWords::WordIdType::kUhr},        // 07:00
        {DisplayWords::WordIdType::kHourAcht,   DisplayWords::WordIdType::kUhr},        // 08:00
        {DisplayWords::WordIdType::kHourNeun,   DisplayWords::WordIdType::kUhr},        // 09:00
        {DisplayWords::WordIdType::kHourZehn,   DisplayWords::WordIdType::kUhr},        // 10:00
        {DisplayWords::WordIdType::kHourElf,    DisplayWords::WordIdType::kUhr}         // 11:00
    },
    {                                                                    // ClockHoursTable[1][] = hh:mm CLOCK_HOUR_MODE_NO_FULL_HOUR
        {DisplayWords::WordIdType::kHourZwoelf, DisplayWords::WordIdType::kNone},       // 00:mm
        {DisplayWords::WordIdType::kHourEins,   DisplayWords::WordIdType::kNone},       // 01:mm
        {DisplayWords::WordIdType::kHourZwei,   DisplayWords::WordIdType::kNone},       // 02:mm
        {DisplayWords::WordIdType::kHourDrei,   DisplayWords::WordIdType::kNone},       // 03:mm
        {DisplayWords::WordIdType::kHourVier,   DisplayWords::WordIdType::kNone},       // 04:mm
        {DisplayWords::WordIdType::kHourFuenf,  DisplayWords::WordIdType::kNone},       // 05:mm
        {DisplayWords::WordIdType::kHourSechs,  DisplayWords::WordIdType::kNone},       // 06:mm
        {DisplayWords::WordIdType::kHourSieben, DisplayWords::WordIdType::kNone},       // 07:mm
        {DisplayWords::WordIdType::kHourAcht,   DisplayWords::WordIdType::kNone},       // 08:mm
        {DisplayWords::WordIdType::kHourNeun,   DisplayWords::WordIdType::kNone},       // 09:mm
        {DisplayWords::WordIdType::kHourZehn,   DisplayWords::WordIdType::kNone},       // 10:mm
        {DisplayWords::WordIdType::kHourElf,    DisplayWords::WordIdType::kNone}        // 11:mm
    }
};


const Clock::MinuteType Clock::kMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM
{
    {                                                                                                                              // ClockMinutesTable[0][] = WESSI
        {HourModeType::kFullHour,    0u, {DisplayWords::WordIdType::kNone,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 00
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 05
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 10
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 15
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 20
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 25
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kHalb,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 30
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 35
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 40
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 45
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 50
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[1][] = OSSI
        {HourModeType::kFullHour,    0u, {DisplayWords::WordIdType::kNone,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 00
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 05
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 10
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 15
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 20
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 25
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kHalb,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 30
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 35
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 40
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kDreiviertel, DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 45
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 50
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[2][] = RHEIN-RUHR
        {HourModeType::kFullHour,    0u, {DisplayWords::WordIdType::kNone,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 00
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 05
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 10
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 15
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZwanzig,     DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 20
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 25
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kHalb,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 30
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 35
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZwanzig,     DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 40
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 45
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 50
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 55
    },
    {                                                                                                                              // ClockMinutesTable[3][] = SCHWABEN
        {HourModeType::kFullHour,    0u, {DisplayWords::WordIdType::kNone,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 00
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 05
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 10
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kViertel,     DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 15
        {HourModeType::kNoFullHour, 0u, {DisplayWords::WordIdType::kZwanzig,     DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kNone}},          // 20
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kHalb}},          // 25
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kHalb,        DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 30
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kNach, DisplayWords::WordIdType::kHalb}},          // 35
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZwanzig,     DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 40
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kDreiviertel, DisplayWords::WordIdType::kNone, DisplayWords::WordIdType::kNone}},          // 45
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kZehn,        DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 50
        {HourModeType::kNoFullHour, 1u, {DisplayWords::WordIdType::kFuenf,       DisplayWords::WordIdType::kVor,  DisplayWords::WordIdType::kNone}},          // 55
    }
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  GetClockWords()
******************************************************************************************************************************************************/
StdReturnType Clock::GetClockWords(byte hour, byte minute, ClockWords& clock_words) const
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType return_value{kENotOk};

    /* ----- Implementation ----------------------------------------------- */
    if(hour < CLOCK_NUMBER_OF_HOURS_PER_DAY && minute < CLOCK_NUMBER_OF_MINUTES_PER_HOUR) {
        return_value = kEOk;
        /*
         * Show "it is" permanently, or only at the full and the half hour. The switch this
         * was is now the default of the setting; what a running clock does is asked here
         * rather than compiled in.
         */
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
} /* GetClockWords */


/******************************************************************************************************************************************************
  GetClockWords()
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
} /* GetClockWords */


/******************************************************************************************************************************************************
  SetTime()
******************************************************************************************************************************************************/
StdReturnType Clock::SetTime(byte hour, byte minute) const
{
    /* ----- Local Variables ---------------------------------------------- */
    StdReturnType return_value{kEOk};
    ClockWords clock_words;

    /* ----- Implementation ----------------------------------------------- */
    if(GetClockWords(hour, minute, clock_words) == kENotOk) return_value = kENotOk;

    if(clock_words.GetShowItIs()) {
        if(Display::GetInstance().SetWord(DisplayWords::WordIdType::kEs) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().SetWord(DisplayWords::WordIdType::kIst) == kENotOk) return_value = kENotOk;
    }
    for(byte index = 0u; index < clock_words.GetHourWords().size() && clock_words.GetHourWord(index) != DisplayWords::WordIdType::kNone; index++) {
        if(Display::GetInstance().SetWord(clock_words.GetHourWord(index)) == kENotOk) return_value = kENotOk;
    }
    for(byte index = 0u; index < clock_words.GetMinuteWords().size() && clock_words.GetMinuteWord(index) != DisplayWords::WordIdType::kNone; index++) {
        if(Display::GetInstance().SetWord(clock_words.GetMinuteWord(index)) == kENotOk) return_value = kENotOk;
    }
    return return_value;
} /* SetTime */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
