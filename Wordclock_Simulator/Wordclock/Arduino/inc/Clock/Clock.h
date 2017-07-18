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

#define CLOCK_NUMBER_OF_HOURS_PER_DAY			24
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR		60

/* Minute */
#define CLOCK_MINUTE_STEP_IN_MINUTES            5
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12
#define CLOCK_MAX_NUMBER_OF_MINUTE_WORDS        3



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/*  */
typedef enum {
    CLOCK_MODE_WESSI,
    CLOCK_MODE_OSSI,
    CLOCK_MODE_RHEIN_RUHR,
    CLOCK_MODE_SCHWABEN
} ClockModesType;


typedef enum {
	CLOCK_HOUR_MODE_FULL_HOUR,
	CLOCK_HOUR_MODE_NO_FULL_HOUR
} ClockHourModesType;


typedef struct {
    ClockHourModesType HourMode;
    byte HourOffset;
    DisplayWordType Words[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
} ClockMinutesType;

typedef struct {
    DisplayWordType Words[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
} ClockHoursType;


typedef struct {
	boolean ShowItIs;
	DisplayWordType HourWords[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
	DisplayWordType MinuteWords[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];
} ClockWordsType;

typedef ClockHoursType HoursTableEntryType;
typedef ClockMinutesType MinutesTableEntryType;


/******************************************************************************************************************************************************
 *  CLASS  Clock
******************************************************************************************************************************************************/
class Clock
{
  private:
    Display* pDisplay;
    ClockModesType Mode;

    static const ClockHoursType ClockHoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const ClockMinutesType ClockMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    // functions
    inline MinutesTableEntryType getMinutesTableEntry(ClockModesType Mode, byte Minute) {
        MinutesTableEntryType MinutesTableEntry;
        memcpy_P(&MinutesTableEntry, &ClockMinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(ClockMinutesType));
        return MinutesTableEntry;
    }
    inline HoursTableEntryType getHoursTableEntry(ClockModesType Mode, ClockHourModesType HourMode, byte Hour) {
        HoursTableEntryType HoursTableEntry;
        memcpy_P(&HoursTableEntry, &ClockHoursTable[HourMode][Hour], sizeof(ClockHoursType));
        return HoursTableEntry;
    }

  public:
    Clock(Display*, ClockModesType);
    ~Clock();

    // get methods
	ClockModesType getMode() { return Mode; }
	stdReturnType getClockWords(byte, byte, ClockWordsType*);


    // set methods
	void setMode(ClockModesType sMode) { Mode = sMode; }

    // methods
    stdReturnType show(byte, byte);
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
