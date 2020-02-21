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
#define CLOCK_NUMBER_OF_HOUR_MODES              2
#define CLOCK_NUMBER_OF_HOURS                   12

#define CLOCK_NUMBER_OF_HOURS_PER_DAY           24
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR        60

/* Minute */
#define CLOCK_MINUTE_STEP_IN_MINUTES            5
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12

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
    enum ModesType {
        MODE_WESSI,
        MODE_OSSI,
        MODE_RHEIN_RUHR,
        MODE_SCHWABEN
    };

    enum HourModesType {
        HOUR_MODE_FULL_HOUR,
        HOUR_MODE_NO_FULL_HOUR
    };

    struct MinutesType {
        HourModesType HourMode;
        byte HourOffset;
        ClockWords::MinutesWordsType Words;
    };

    using HoursType = ClockWords::HourWordsType;
    using ClockWordsListType = ClockWords::WordsListType;

    using HoursTableElementType = HoursType;
    using MinutesTableElementType = MinutesType;


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    ModesType Mode;

    static const HoursType HoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinutesType MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    
    // functions
	Clock(ModesType);
    ~Clock();
	
	MinutesTableElementType getMinutesTableElement(byte Minute) const {
        MinutesTableElementType MinutesTableElement;
        memcpy_P(&MinutesTableElement, &MinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(MinutesType));
        return MinutesTableElement;
    }
    HoursTableElementType getHoursTableElement(HourModesType HourMode, byte Hour) const {
        HoursTableElementType HoursTableElement;
        memcpy_P(&HoursTableElement, &HoursTable[HourMode][Hour], sizeof(HoursType));
        return HoursTableElement;
    }

    boolean calculateItIs(byte Minute) const {
        byte MinuteSteps = Minute / CLOCK_MINUTE_STEP_IN_MINUTES;
        // show "it is" only to full and half hour
        if(MinuteSteps == 0 || MinuteSteps == (CLOCK_NUMBER_OF_MINUTE_STEPS / 2)) {
            return true;
        } else {
            return false;
        }
    }

    byte transformFrom24hTo12hFormat(byte Hour) const {
        return Hour % CLOCK_NUMBER_OF_HOURS;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
  	static Clock& getInstance();
	  
    // get methods
    ModesType getMode() const { return Mode; }
    stdReturnType getClockWords(byte, byte, ClockWords&) const;
    stdReturnType getClockWords(byte, byte, ClockWordsListType&) const;

    // set methods
    void setMode(ModesType sMode) { Mode = sMode; }

    // methods
    stdReturnType setClock(byte, byte);
    void setClockFast(byte, byte);
    void show() { Display::getInstance().show(); }
};


#endif
/******************************************************************************************************************************************************
 *  END OF FILE
******************************************************************************************************************************************************/
