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
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    Clock::ClockWordsTableType ClockWordsTable;
    BorderType Border;

    // functions
    void reset();
    void setBorderPixels(const BorderType& Border) { writeBorderPixels(true, Border); }
    void clearBorderPixels(const BorderType& Border) { writeBorderPixels(false, Border); }
    void writeBorderPixels(bool, const BorderType&);
    stdReturnType increaseBorder(BorderType&);
    stdReturnType decreaseBorder(BorderType&);
    void clearTimeTask();
    void setTimeTask();
    void setMinBorder(BorderType&);
    void setMaxBorder(BorderType&);
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    AnimationClockCube();
    ~AnimationClockCube();

	// get methods


	// set methods

	// methods
    void init(Display*, Clock*);
    stdReturnType setClock(byte, byte);
    void task();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
