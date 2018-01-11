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
const DisplayWords::Word DisplayWords::DisplayWordsTable[] PROGMEM
{
    {0,0,0},                                //  0 = DISPLAY_WORD_NONE           = ""
    {0,0,2},                                //  1 = DisplayWords::WORD_ES       = "ES"
    {0,3,3},                                //  2 = DISPLAY_WORD_IST            = "IST"
    {0,7,4},                                //  3 = DISPLAY_WORD_FUENF          = "F�NF"
    {1,0,4},                                //  4 = DISPLAY_WORD_ZEHN           = "ZEHN"
    {1,4,7},                                //  5 = DISPLAY_WORD_ZWANZIG        = "ZWANZIG"
    {2,0,4},                                //  6 = DISPLAY_WORD_DREI           = "DREI"
    {2,4,4},                                //  7 = DISPLAY_WORD_VIER           = "VIER"
    {2,4,7},                                //  8 = DISPLAY_WORD_VIERTEL        = "VIERTEL"
    {2,0,11},                               //  9 = DISPLAY_WORD_DREIVIERTEL    = "DREIVIERTEL"
    {3,2,4},                                // 10 = DISPLAY_WORD_NACH           = "NACH"
    {3,6,3},                                // 11 = DISPLAY_WORD_VOR            = "VOR"
    {4,0,4},                                // 12 = DISPLAY_WORD_HALB           = "HALB"
    {4,5,5},                                // 13 = DISPLAY_WORD_HOUR_ZWOELF    = "ZW�LF"
    {5,0,4},                                // 14 = DISPLAY_WORD_HOUR_ZWEI      = "ZWEI"
    {5,2,3},                                // 15 = DISPLAY_WORD_HOUR_EIN       = "EIN"
    {5,2,4},                                // 16 = DISPLAY_WORD_HOUR_EINS      = "EINS"
    {5,5,6},                                // 17 = DISPLAY_WORD_HOUR_SIEBEN    = "SIEBEN"
    {6,1,4},                                // 18 = DISPLAY_WORD_HOUR_DREI      = "DREI"
    {6,7,4},                                // 19 = DISPLAY_WORD_HOUR_FUENF     = "F�NF"
    {7,0,3},                                // 20 = DISPLAY_WORD_HOUR_ELF       = "ELF"
    {7,3,4},                                // 21 = DISPLAY_WORD_HOUR_NEUN      = "NEUN"
    {7,7,4},                                // 22 = DISPLAY_WORD_HOUR_VIER      = "VIER"
    {8,1,4},                                // 23 = DISPLAY_WORD_HOUR_ACHT      = "ACHT"
    {8,5,4},                                // 24 = DISPLAY_WORD_HOUR_ZEHN      = "ZEHN"
    {9,1,5},                                // 25 = DISPLAY_WORD_HOUR_SECHS     = "SECHS"
    {9,8,3},                                // 26 = DISPLAY_WORD_UHR            = "UHR"
};

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of DisplayWords
******************************************************************************************************************************************************/
/*! \brief          DisplayWords Constructor
 *  \details        Instantiation of the DisplayWords library
 *
 *  \return         -
******************************************************************************************************************************************************/
DisplayWords::DisplayWords()
{

} /* DisplayWords */


/******************************************************************************************************************************************************
  Destructor of DisplayWords
******************************************************************************************************************************************************/
DisplayWords::~DisplayWords()
{

} /* ~DisplayWords */


/******************************************************************************************************************************************************
  getDisplayWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType DisplayWords::getDisplayWord(WordIdType WordId, Word& Word) const
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        Word = getDisplayWordFast(WordId);
        ReturnValue = E_OK;
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getDisplayWord */


/******************************************************************************************************************************************************
  getDisplayWordLength()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType DisplayWords::getDisplayWordLength(WordIdType WordId, byte& Length) const
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        Length = getDisplayWordLengthFast(WordId);
        ReturnValue = E_OK;
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getDisplayWordLength */


/******************************************************************************************************************************************************
  getDisplayWordColumn()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType DisplayWords::getDisplayWordColumn(WordIdType WordId, byte& Column) const
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        Column = getDisplayWordColumnFast(WordId);
        ReturnValue = E_OK;
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getDisplayWordColumn */


/******************************************************************************************************************************************************
  getDisplayWordRow()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType DisplayWords::getDisplayWordRow(WordIdType WordId, byte& Row) const
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        Row = getDisplayWordRowFast(WordId);
        ReturnValue = E_OK;
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getDisplayWordRow */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 