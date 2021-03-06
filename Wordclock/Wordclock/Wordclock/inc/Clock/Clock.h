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
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


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

    class ClockWordsType {
      public:
        boolean ShowItIs;
        DisplayWords::WordIdType HourWords[CLOCK_MAX_NUMBER_OF_HOUR_WORDS];
        DisplayWords::WordIdType MinuteWords[CLOCK_MAX_NUMBER_OF_MINUTE_WORDS];

        bool operator==(const ClockWordsType& ClockWords)
        {
            if(ShowItIs       == ClockWords.ShowItIs        &&
               HourWords[0]   == ClockWords.HourWords[0]    &&
               HourWords[1]   == ClockWords.HourWords[1]    &&
               MinuteWords[0] == ClockWords.MinuteWords[0]  &&
               MinuteWords[1] == ClockWords.MinuteWords[1]  &&
               MinuteWords[2] == ClockWords.MinuteWords[2]  )
            {
                return true;
            } else {
                return false;
            }
        }
        bool operator!=(const ClockWordsType& ClockWords)
        {
            if(operator==(ClockWords)) return false;
            else return true;
        }
    };

    using ClockWordsTableType = DisplayWords::WordIdType[CLOCK_WORDS_TABLE_TYPE_SIZE];

    using HoursTableElementType = HoursType;
    using MinutesTableElementType = MinutesType;


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    Display* pDisplay;
    ModesType Mode;

    static const HoursType HoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinutesType MinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];
    
    // functions
    MinutesTableElementType getMinutesTableElement(ModesType Mode, byte Minute) {
        MinutesTableElementType MinutesTableElement;
        memcpy_P(&MinutesTableElement, &MinutesTable[Mode][Minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(MinutesType));
        return MinutesTableElement;
    }
    HoursTableElementType getHoursTableElement(HourModesType HourMode, byte Hour) {
        HoursTableElementType HoursTableElement;
        memcpy_P(&HoursTableElement, &HoursTable[HourMode][Hour], sizeof(HoursType));
        return HoursTableElement;
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
