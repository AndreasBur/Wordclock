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
    static constexpr byte kShiftCounterMaxValue{5u};
    static constexpr byte kColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    static constexpr byte kRowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::kWordNone}};
    DisplayWords words_;
    byte shift_counter_{0u};

    // functions
    void Reset();
    static void ClearTimeTask();
    void SetTimeTask();
    void SetStateToSetTime();

    static void ShiftQuadrants(DisplayPixels&, byte);

    static void ShiftQuadrantUpperLeft();
    static void ShiftQuadrantUpperRight();
    static void ShiftQuadrantLowerLeft();
    static void ShiftQuadrantLowerRight();

    static void ShiftDownRight(byte, byte);
    static void ShiftDownLeft(byte, byte);
    static void ShiftUpRight(byte, byte);
    static void ShiftUpLeft(byte, byte);

    static void ClearOldAndSetNewPixel(byte, byte, byte, byte);
    static void SetNewPixel(byte, byte, byte, byte);

    static byte ShiftLeft(byte column, byte number_of_shifts) {
         return (static_cast<int16_t>(column) - number_of_shifts > kColumnCenter) ? (column - number_of_shifts) : kColumnCenter;
    }
    static byte ShiftRight(byte column, byte number_of_shifts) {
         return (static_cast<uint16_t>(column) + number_of_shifts < kColumnCenter) ? (column + number_of_shifts) : kColumnCenter;
    }
    static byte ShiftUp(byte row, byte number_of_shifts) {
         return (static_cast<int16_t>(row) - number_of_shifts > kRowCenter) ? (row - number_of_shifts) : kRowCenter;
    }
    static byte ShiftDown(byte row, byte number_of_shifts) {
         return (static_cast<uint16_t>(row) + number_of_shifts < kRowCenter) ? (row + number_of_shifts) : kRowCenter;
    }

    void SetTime(DisplayPixels& pixels) {
        for(byte index  = 0u; index < clock_words_table_.size(); index++) {
            pixels.SetWord(clock_words_table_[index]);
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationExplodePixels() : words_() { }
    ~AnimationExplodePixels() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
