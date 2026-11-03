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
/**     \file       AnimationExplodeWords.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_EXPLODE_WORDS_H_
#define _ANIMATION_EXPLODE_WORDS_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationExplodeWords configuration parameter */


/* AnimationExplodeWords parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   E X P L O D E   W O R D S
******************************************************************************************************************************************************/
class AnimationExplodeWords : public Animation
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
    static constexpr byte kColumnCenter{DISPLAY_NUMBER_OF_COLUMNS / 2u};
    static constexpr byte kRowCenter{DISPLAY_NUMBER_OF_ROWS / 2u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte clock_words_table_index_{0u};
    byte current_word_index_{0u};
    byte current_word_length_{0u};
    byte current_column_{0u};
    byte current_row_{0u};

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();
    void SetWordToSet(DisplayWords::WordIdType);
    StdReturnType SetNextWordToClear();
    StdReturnType SetNextWordToClearInColumn();
    StdReturnType SetNextRowToClear();
    StdReturnType SetNextWordToSet();
    void SetWordIndex(DisplayWords::WordIdType);
    void SetWordLength();
    StdReturnType ShiftWord(byte, byte);
    void ToggleWordOnDisplay() const;
    void SetWordOnDisplay() const;

    //byte getFinalIndex() const { return Display::GetInstance().ColumnAndRowToIndex(getFinalColumn(), getFinalRow()); }
    static byte GetClearFinalRow() { return kRowCenter; }
    byte GetSetFinalRow() const { return Display::GetInstance().IndexToRow(current_word_index_); }
    byte GetClearFinalColumn() const { return kColumnCenter - (current_word_length_ / 2u); }
    byte GetSetFinalColumn() const { return Display::GetInstance().IndexToColumn(current_word_index_); }

    byte GetNextColumn(byte final_column) const {
        byte next_column = current_column_;
        if(current_column_ < final_column) { next_column++; }
        if(current_column_ > final_column) { next_column--; }
        return next_column;
    }

    byte GetNextRow(byte final_row) const {
        byte next_row = current_row_;
        if(current_row_ < final_row) { next_row++; }
        if(current_row_ > final_row) { next_row--; }
        return next_row;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationExplodeWords() : words_() { }
    ~AnimationExplodeWords() { }

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
