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
/**     \file       DisplayWords.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_WORDS_H_
#define _DISPLAY_WORDS_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayWord.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* DisplayWords configuration parameter */


/* DisplayWords parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  DisplayWords
******************************************************************************************************************************************************/
class DisplayWords
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum WordIdType {
        WORD_NONE,
        WORD_ES,
        WORD_IST,
        WORD_FUENF,
        WORD_ZEHN,
        WORD_ZWANZIG,
        WORD_DREI,
        WORD_VIER,
        WORD_VIERTEL,
        WORD_DREIVIERTEL,
        WORD_NACH,
        WORD_VOR,
        WORD_HALB,
        WORD_HOUR_ZWOELF,
        WORD_HOUR_ZWEI,
        WORD_HOUR_EIN,
        WORD_HOUR_EINS,
        WORD_HOUR_SIEBEN,
        WORD_HOUR_DREI,
        WORD_HOUR_FUENF,
        WORD_HOUR_ELF,
        WORD_HOUR_NEUN,
        WORD_HOUR_VIER,
        WORD_HOUR_ACHT,
        WORD_HOUR_ZEHN,
        WORD_HOUR_SECHS,
        WORD_UHR,
        WORD_NUMBER_OF_WORDS
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    static const DisplayWord DisplayWordsTable[];

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    DisplayWords();
    ~DisplayWords();

	// get methods
    stdReturnType getDisplayWord(WordIdType, DisplayWord&) const;
    stdReturnType getDisplayWordLength(WordIdType, byte&) const;
    stdReturnType getDisplayWordColumn(WordIdType, byte&) const;
    stdReturnType getDisplayWordRow(WordIdType, byte&) const;

    // get methods fast
    DisplayWord getDisplayWordFast(WordIdType WordId) const { DisplayWord Word; memcpy_P(&Word, &DisplayWordsTable[WordId], sizeof(DisplayWord)); return Word; }
    byte getDisplayWordRowFast(WordIdType WordId) const { DisplayWord Word; memcpy_P(&Word, &DisplayWordsTable[WordId], sizeof(DisplayWord)); return Word.getRow(); }
    byte getDisplayWordColumnFast(WordIdType WordId) const { DisplayWord Word; memcpy_P(&Word, &DisplayWordsTable[WordId], sizeof(DisplayWord)); return Word.getColumn(); }
    byte getDisplayWordLengthFast(WordIdType WordId) const { DisplayWord Word; memcpy_P(&Word, &DisplayWordsTable[WordId], sizeof(DisplayWord)); return Word.getLength(); }

	// set methods

	// methods

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
