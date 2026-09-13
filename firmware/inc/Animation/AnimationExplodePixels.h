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
/**     \file       AnimationExplodePixels.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_EXPLODE_PIXELS_H_
#define _ANIMATION_EXPLODE_PIXELS_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"
#include "DisplayPixels.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationExplodePixels configuration parameter */


/* AnimationExplodePixels parameter */

/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   E X P L O D E   P I X E L S
******************************************************************************************************************************************************/
class AnimationExplodePixels : public Animation
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
    static constexpr byte ShiftCounterMaxValue{5u};
    static constexpr byte ColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    static constexpr byte RowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    DisplayWords Words;
    byte ShiftCounter{0u};

    // functions
    void reset();
    static void clearTimeTask();
    void setTimeTask();
    void setStateToSetTime();

    static void shiftQuadrants(DisplayPixels&, byte);

    static void shiftQuadrantUpperLeft();
    static void shiftQuadrantUpperRight();
    static void shiftQuadrantLowerLeft();
    static void shiftQuadrantLowerRight();

    static void shiftDownRight(byte, byte);
    static void shiftDownLeft(byte, byte);
    static void shiftUpRight(byte, byte);
    static void shiftUpLeft(byte, byte);

    static void clearOldAndSetNewPixel(byte, byte, byte, byte);
    static void setNewPixel(byte, byte, byte, byte);

    static byte shiftLeft(byte Column, byte NumberOfShifts) {
         return (static_cast<int16_t>(Column) - NumberOfShifts > ColumnCenter) ? (Column - NumberOfShifts) : ColumnCenter;
    }
    static byte shiftRight(byte Column, byte NumberOfShifts) {
         return (static_cast<uint16_t>(Column) + NumberOfShifts < ColumnCenter) ? (Column + NumberOfShifts) : ColumnCenter;
    }
    static byte shiftUp(byte Row, byte NumberOfShifts) {
         return (static_cast<int16_t>(Row) - NumberOfShifts > RowCenter) ? (Row - NumberOfShifts) : RowCenter;
    }
    static byte shiftDown(byte Row, byte NumberOfShifts) {
         return (static_cast<uint16_t>(Row) + NumberOfShifts < RowCenter) ? (Row + NumberOfShifts) : RowCenter;
    }

    void setTime(DisplayPixels& Pixels) {
        for(byte Index  = 0u; Index < ClockWordsTable.size(); Index++) {
            Pixels.setWord(ClockWordsTable[Index]);
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationExplodePixels() : Words() { }
    ~AnimationExplodePixels() { }

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
