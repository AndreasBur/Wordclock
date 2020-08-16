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
        MODE_SCHWABEN
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

    using HourType = ClockWords::HourWordsType;
    using ClockWordsListType = ClockWords::WordsListType;

    using HourTableElementType = HourType;
    using MinuteTableElementType = MinuteType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    ModeType Mode;

    static const HourType HoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinuteType MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    
    // functions
    constexpr Clock(ModeType sMode) : Mode(sMode) {}
    ~Clock() {}
    
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

    boolean calculateItIs(byte Minute) const {
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
        static Clock SingletonInstance(Clock::MODE_WESSI);
        return SingletonInstance;
    }
      
    // get methods
    ModeType getMode() const { return Mode; }
    StdReturnType getClockWords(byte, byte, ClockWords&) const;
    StdReturnType getClockWords(byte, byte, ClockWordsListType&) const;

    // set methods
    void setMode(ModeType sMode) { Mode = sMode; }

    // methods
    StdReturnType setClock(byte, byte);
    void setClockFast(byte, byte);
    void show() { Display::getInstance().show(); }
    void refresh(byte Hour, byte Minute) { setClock(Hour, Minute); show(); }
};


#endif
/******************************************************************************************************************************************************
 *  END OF FILE
******************************************************************************************************************************************************/
