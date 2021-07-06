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
/**     \file       AnimationClockExplode.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_CLOCK_EXPLODE_H_
#define _ANIMATION_CLOCK_EXPLODE_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "AnimationClockCommon.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationClockExplode configuration parameter */


/* AnimationClockExplode parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   C L O C K   E X P L O D E
******************************************************************************************************************************************************/
class AnimationClockExplode : public AnimationClockCommon
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
    static constexpr byte ColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    static constexpr byte RowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    DisplayWords Words;
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte ClockWordsTableIndex{0u};
    byte CurrentWordIndex{0u};
    byte CurrentWordLength{0u};
    byte CurrentColumn{0u};
    byte CurrentRow{0u};

    // functions
    void reset();
    void clearTimeTask();
    void setTimeTask();
    void setWordToSet(DisplayWords::WordIdType);
    StdReturnType setNextWordToClear();
    StdReturnType setNextWordToClearInColumn();
    StdReturnType setNextRowToClear();
    StdReturnType setNextWordToSet();
    void setWordIndex(DisplayWords::WordIdType);
    void setWordLength();
    StdReturnType shiftWord(byte, byte);
    void toggleWordOnDisplay();
    void setWordOnDisplay();

    //byte getFinalIndex() const { return Display::getInstance().columnAndRowToIndex(getFinalColumn(), getFinalRow()); }
    byte getClearFinalRow() const { return RowCenter; }
    byte getSetFinalRow() const { return Display::getInstance().indexToRow(CurrentWordIndex); }
    byte getClearFinalColumn() const { return ColumnCenter - (CurrentWordLength / 2u); }
    byte getSetFinalColumn() const { return Display::getInstance().indexToColumn(CurrentWordIndex); }

    byte getNextColumn(byte FinalColumn) const {
        byte nextColumn{CurrentColumn};
        if(CurrentColumn < FinalColumn) nextColumn++;
        if(CurrentColumn > FinalColumn) nextColumn--;
        return nextColumn;
    }

    byte getNextRow(byte FinalRow) const {
        byte nextRow{CurrentRow};
        if(CurrentRow < FinalRow) nextRow++;
        if(CurrentRow > FinalRow) nextRow--;
        return nextRow;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationClockExplode() : Words() { }
    ~AnimationClockExplode() { }

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
