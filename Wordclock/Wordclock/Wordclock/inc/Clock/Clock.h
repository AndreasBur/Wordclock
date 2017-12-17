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


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Clock configuration parameter */
#define CLOCK_SHOW_IT_IS_PERMANENTLY            STD_ON

/* Clock  parameter */
/* Hour */
#define CLOCK_NUMBER_OF_HOUR_MODES              2
#define CLOCK_NUMBER_OF_HOURS                   12
#define CLOCK_MAX_NUMBER_OF_HOUR_WORDS          2

#define CLOCK_NUMBER_OF_HOURS_PER_DAY           24
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR        60

/* Minute */
#define CLOCK_MINUTE_STEP_IN_MINUTES            5
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12
#define CLOCK_MAX_NUMBER_OF_MINUTE_WORDS        3

#define CLOCK_IT_IS_NUMBER_OF_WORDS             2
#define CLOCK_WORDS_TABLE_TYPE_SIZE             (CLOCK_MAX_NUMBER_OF_HOUR_WORDS + CLOCK_MAX_NUMBER_OF_MINUTE_WORDS + CLOCK_IT_IS_NUMBER_OF_WORDS)


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  Clock
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
        DisplayWords::WordIdType Words[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
    };

    struct HoursType {
        DisplayWords::WordIdType Words[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
    };

    struct ClockWordsType {
        boolean ShowItIs;
        DisplayWords::WordIdType HourWords[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
        DisplayWords::WordIdType MinuteWords[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
    };

    using ClockWordsTableType = DisplayWords::WordIdType[CLOCK_WORDS_TABLE_TYPE_SIZE];

    using HoursTableEntryType = HoursType;
    using MinutesTableEntryType = MinutesType;


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    Display* pDisplay;
    ModesType Mode;

    static const HoursType HoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinutesType MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    
    // functions
    MinutesTableEntryType getMinutesTableEntry(ModesType Mode, byte Minute) {
        MinutesTableEntryType MinutesTableEntry;
        memcpy_P(&MinutesTableEntry, &MinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(MinutesType));
        return MinutesTableEntry;
    }
    HoursTableEntryType getHoursTableEntry(HourModesType HourMode, byte Hour) {
        HoursTableEntryType HoursTableEntry;
        memcpy_P(&HoursTableEntry, &HoursTable[HourMode][Hour], sizeof(HoursType));
        return HoursTableEntry;
    }


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Clock(Display*, ModesType);
    ~Clock();

    // get methods
    ModesType getMode() const { return Mode; }
    stdReturnType getClockWords(byte, byte, ClockWordsType*);
    stdReturnType getClockWords(byte, byte, ClockWordsTableType);
    boolean compareClockWords(ClockWordsType*, ClockWordsType*) const;

    // set methods
    void setMode(ModesType sMode) { Mode = sMode; }

    // methods
    stdReturnType setClock(byte, byte);
    void show() { pDisplay->show(); }
};


#endif
/******************************************************************************************************************************************************
 *  END OF FILE
******************************************************************************************************************************************************/
