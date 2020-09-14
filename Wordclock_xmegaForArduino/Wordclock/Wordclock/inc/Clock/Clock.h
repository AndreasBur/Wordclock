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
#include "ClockWords.h"
#include <array>

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Clock configuration parameter */
#define CLOCK_SHOW_IT_IS_PERMANENTLY            STD_ON
#define CLOCK_INITIAL_MODE                      MODE_WESSI

/* Clock  parameter */
/* Hour */
#define CLOCK_NUMBER_OF_HOUR_MODES              2u
#define CLOCK_NUMBER_OF_HOURS                   12u

#define CLOCK_NUMBER_OF_HOURS_PER_DAY           24u
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR        60u

/* Minute */
#define CLOCK_MINUTE_STEP_IN_MINUTES            5u
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12u

/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   C L O C K
******************************************************************************************************************************************************/
class Clock
{
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
  public:
    /*  */
    enum ModeType {
        MODE_WESSI,
        MODE_OSSI,
        MODE_RHEIN_RUHR,
        MODE_SCHWABEN,
        MODE_NUMBER_OF_MODES
    };

    enum HourModeType {
        HOUR_MODE_FULL_HOUR,
        HOUR_MODE_NO_FULL_HOUR
    };

    struct MinuteType {
        HourModeType HourMode;
        byte HourOffset;
        ClockWords::MinutesWordsType Words;
    };

    struct TimeType {
        byte Hour;
        byte Minute;
    };

    using HourType = ClockWords::HourWordsType;
    using ClockWordsListType = ClockWords::WordsListType;

    using HourTableElementType = HourType;
    using MinuteTableElementType = MinuteType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    ModeType Mode{CLOCK_INITIAL_MODE};

    static const HourType HoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinuteType MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];

    // functions
    constexpr Clock(ModeType sMode) : Mode(sMode) { }
    ~Clock() { }

    MinuteTableElementType getMinutesTableElement(byte Minute) const {
        MinuteTableElementType minutesTableElement;
        memcpy_P(&minutesTableElement, &MinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(MinuteType));
        return minutesTableElement;
    }
    HourTableElementType getHoursTableElement(HourModeType HourMode, byte Hour) const {
        HourTableElementType hoursTableElement;
        memcpy_P(&hoursTableElement, &HoursTable[HourMode][Hour], sizeof(HourType));
        return hoursTableElement;
    }

    bool calculateItIs(byte Minute) const {
        byte minuteSteps = Minute / CLOCK_MINUTE_STEP_IN_MINUTES;
        // show "it is" only to full and half hour
        if(minuteSteps == 0u || minuteSteps == (CLOCK_NUMBER_OF_MINUTE_STEPS / 2u)) {
            return true;
        } else {
            return false;
        }
    }

    byte transform24hTo12hFormat(byte Hour) const {
        return Hour % CLOCK_NUMBER_OF_HOURS;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Clock& getInstance() {
        static Clock SingletonInstance(CLOCK_INITIAL_MODE);
        return SingletonInstance;
    }

    // get methods
    ModeType getMode() const { return Mode; }
    StdReturnType getClockWords(byte, byte, ClockWords&) const;
    StdReturnType getClockWords(byte, byte, ClockWordsListType&) const;

    // set methods
    void setModeFast(ModeType sMode) { Mode = sMode; }
    StdReturnType setMode(ModeType sMode)
    {
        if(isModeValid(sMode)) {
            Mode = sMode;
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }

    // methods
    static bool isModeValid(ModeType sMode) { return sMode < MODE_NUMBER_OF_MODES; }
    StdReturnType setTime(TimeType Time) const { return setTime(Time.Hour, Time.Minute); }
    StdReturnType setTime(byte, byte) const;
    void setTime(ClockWords::WordsListType ClockWordsTable) const {
        setTime(ClockWordsTable, 0u);
    }
    void setTime(ClockWords::WordsListType ClockWordsTable, byte MaxLength) const {
            for(byte index  = 0u; index < ClockWordsTable.size(); index++) {
            Display::getInstance().setWord(ClockWordsTable[index], MaxLength);
        }
    }

    void setTimeFast(TimeType Time) const { setTimeFast(Time.Hour, Time.Minute); }
    void setTimeFast(byte, byte) const;
    StdReturnType show() const { return Display::getInstance().show(); }
    StdReturnType refresh(byte Hour, byte Minute) const {
        StdReturnType returnValue{E_OK};
        if(setTime(Hour, Minute) == E_NOT_OK) { returnValue = E_NOT_OK; }
        if(show() == E_NOT_OK) { returnValue = E_NOT_OK; }
        return returnValue;
    }
};


#endif
/******************************************************************************************************************************************************
 *  END OF FILE
******************************************************************************************************************************************************/
