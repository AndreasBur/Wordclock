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
/**     \file       DisplayWords.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _DISPLAY_WORDS_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "DisplayWords.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
const DisplayWord DisplayWords::DisplayWordsTable[] PROGMEM
{
    DisplayWord(0,0,0),                  //  0 = DISPLAY_WORD_NONE           = ""
    DisplayWord(0,0,2),                  //  1 = DisplayWords::WORD_ES       = "ES"
    DisplayWord(3,0,3),                  //  2 = DISPLAY_WORD_IST            = "IST"
    DisplayWord(7,0,4),                  //  3 = DISPLAY_WORD_FUENF          = "F�NF"
    DisplayWord(0,1,4),                  //  4 = DISPLAY_WORD_ZEHN           = "ZEHN"
    DisplayWord(4,1,7),                  //  5 = DISPLAY_WORD_ZWANZIG        = "ZWANZIG"
    DisplayWord(0,2,4),                  //  6 = DISPLAY_WORD_DREI           = "DREI"
    DisplayWord(4,2,4),                  //  7 = DISPLAY_WORD_VIER           = "VIER"
    DisplayWord(4,2,7),                  //  8 = DISPLAY_WORD_VIERTEL        = "VIERTEL"
    DisplayWord(0,2,11),                 //  9 = DISPLAY_WORD_DREIVIERTEL    = "DREIVIERTEL"
    DisplayWord(2,3,4),                  // 10 = DISPLAY_WORD_NACH           = "NACH"
    DisplayWord(6,3,3),                  // 11 = DISPLAY_WORD_VOR            = "VOR"
    DisplayWord(0,4,4),                  // 12 = DISPLAY_WORD_HALB           = "HALB"
    DisplayWord(5,4,5),                  // 13 = DISPLAY_WORD_HOUR_ZWOELF    = "ZW�LF"
    DisplayWord(0,5,4),                  // 14 = DISPLAY_WORD_HOUR_ZWEI      = "ZWEI"
    DisplayWord(2,5,3),                  // 15 = DISPLAY_WORD_HOUR_EIN       = "EIN"
    DisplayWord(2,5,4),                  // 16 = DISPLAY_WORD_HOUR_EINS      = "EINS"
    DisplayWord(5,5,6),                  // 17 = DISPLAY_WORD_HOUR_SIEBEN    = "SIEBEN"
    DisplayWord(1,6,4),                  // 18 = DISPLAY_WORD_HOUR_DREI      = "DREI"
    DisplayWord(7,6,4),                  // 19 = DISPLAY_WORD_HOUR_FUENF     = "F�NF"
    DisplayWord(0,7,3),                  // 20 = DISPLAY_WORD_HOUR_ELF       = "ELF"
    DisplayWord(3,7,4),                  // 21 = DISPLAY_WORD_HOUR_NEUN      = "NEUN"
    DisplayWord(7,7,4),                  // 22 = DISPLAY_WORD_HOUR_VIER      = "VIER"
    DisplayWord(1,8,4),                  // 23 = DISPLAY_WORD_HOUR_ACHT      = "ACHT"
    DisplayWord(5,8,4),                  // 24 = DISPLAY_WORD_HOUR_ZEHN      = "ZEHN"
    DisplayWord(1,9,5),                  // 25 = DISPLAY_WORD_HOUR_SECHS     = "SECHS"
    DisplayWord(8,9,3),                  // 26 = DISPLAY_WORD_UHR            = "UHR"
};

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getDisplayWord()
******************************************************************************************************************************************************/
StdReturnType DisplayWords::getDisplayWord(WordIdType WordId, DisplayWord& Word) const
{
    StdReturnType returnValue{E_NOT_OK};

    if(isWordIdValid(WordId)) {
        Word = getDisplayWordFast(WordId);
        returnValue = E_OK;
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* getDisplayWord */


/******************************************************************************************************************************************************
  getDisplayWordLength()
******************************************************************************************************************************************************/
StdReturnType DisplayWords::getDisplayWordLength(WordIdType WordId, byte& Length) const
{
    StdReturnType returnValue{E_NOT_OK};

    if(isWordIdValid(WordId)) {
        Length = getDisplayWordLengthFast(WordId);
        returnValue = E_OK;
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* getDisplayWordLength */


/******************************************************************************************************************************************************
  getDisplayWordColumn()
******************************************************************************************************************************************************/
StdReturnType DisplayWords::getDisplayWordColumn(WordIdType WordId, byte& Column) const
{
    StdReturnType returnValue{E_NOT_OK};

    if(isWordIdValid(WordId)) {
        Column = getDisplayWordColumnFast(WordId);
        returnValue = E_OK;
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* getDisplayWordColumn */


/******************************************************************************************************************************************************
  getDisplayWordRow()
******************************************************************************************************************************************************/
StdReturnType DisplayWords::getDisplayWordRow(WordIdType WordId, byte& Row) const
{
    StdReturnType returnValue{E_NOT_OK};

    if(isWordIdValid(WordId)) {
        Row = getDisplayWordRowFast(WordId);
        returnValue = E_OK;
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* getDisplayWordRow */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
