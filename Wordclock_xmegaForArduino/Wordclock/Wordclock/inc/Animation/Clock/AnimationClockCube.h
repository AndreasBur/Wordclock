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
/**     \file       AnimationClockCube.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_CUBE_H_
#define _ANIMATION_CLOCK_CUBE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "AnimationClockCommon.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationClockCube configuration parameter */


/* AnimationClockCube parameter */
#define ANIMATION_CLOCK_CUBE_COLUMN_START_MAX_VALUE         4
#define ANIMATION_CLOCK_CUBE_COLUMN_END_MIN_VALUE           6

#define ANIMATION_CLOCK_CUBE_ROW_START_MAX_VALUE            4
#define ANIMATION_CLOCK_CUBE_ROW_END_MIN_VALUE              5


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class AnimationClockCube : public AnimationClockCommon
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    struct BorderType {
        byte ColumnStart;
        byte ColumnEnd;
        byte RowStart;
        byte RowEnd;
    };

    enum BorderStateType {
        BORDER_STATE_MIN,
        BORDER_STATE_MAX,
        BORDER_STATE_BETWEEN
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    BorderType Border{ANIMATION_CLOCK_CUBE_COLUMN_START_MAX_VALUE, ANIMATION_CLOCK_CUBE_COLUMN_END_MIN_VALUE,
      ANIMATION_CLOCK_CUBE_ROW_START_MAX_VALUE, ANIMATION_CLOCK_CUBE_ROW_END_MIN_VALUE};
    BorderStateType BorderState{BORDER_STATE_MIN};

    // functions
    void reset();
    void setStateToSetTime();
    void setBorderPixels() { writeBorderPixels(true); }
    void clearBorderPixels() { writeBorderPixels(false); }
    void clearBorderPixelsWithoutClockPixels();
    void writeBorderPixels(bool);
    StdReturnType increaseBorder();
    StdReturnType decreaseBorder();
    void clearTimeTask();
    void setTimeTask();
    void setMinBorder();
    void setMaxBorder();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockCube() { }
    ~AnimationClockCube() { }

    // get methods


    // set methods

    // methods
    void init();
    StdReturnType setTime(byte, byte);
    void task();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
