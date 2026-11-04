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
 *  C L A S S   D I S P L A Y W O R D S
******************************************************************************************************************************************************/
class DisplayWords
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum class WordIdType {
        kNone,
        kEs,
        kIst,
        kFuenf,
        kZehn,
        kZwanzig,
        kDrei,
        kVier,
        kViertel,
        kDreiviertel,
        kNach,
        kVor,
        kHalb,
        kHourZwoelf,
        kHourZwei,
        kHourEin,
        kHourEins,
        kHourSieben,
        kHourDrei,
        kHourFuenf,
        kHourElf,
        kHourNeun,
        kHourVier,
        kHourAcht,
        kHourZehn,
        kHourSechs,
        kUhr,
        kNumberOfWords
    };
    static constexpr byte kNumberOfWords{static_cast<byte>(WordIdType::kNumberOfWords)};

    using DisplayWordsTableElementType = DisplayWord;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const DisplayWord kDisplayWordsTable[];

    // functions
    static DisplayWordsTableElementType GetDisplayWordsTableElement(byte word_id) {
        DisplayWordsTableElementType display_words_table_element;
        memcpy_P(&display_words_table_element, &kDisplayWordsTable[word_id], sizeof(DisplayWordsTableElementType));
        return display_words_table_element;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DisplayWords() { }
    ~DisplayWords() { }

    /*
     * Two forms of every reader, and the argument list says which: the one taking a
     * reference answers whether the word exists, the one taking none answers the word.
     * Both check, and the unchecked read is the private table element above - a word that
     * is not in the table has no place on the plate, which is what a length of zero says,
     * and a word of zero length lights nothing.
     */
    // get methods
    static StdReturnType GetDisplayWord(WordIdType, DisplayWord&);
    static StdReturnType GetDisplayWordLength(WordIdType, byte&);
    static StdReturnType GetDisplayWordColumn(WordIdType, byte&);
    static StdReturnType GetDisplayWordRow(WordIdType, byte&);

    static DisplayWord GetDisplayWord(WordIdType word_id) {
        if(!IsWordIdValid(word_id)) { return DisplayWord{}; }
        return GetDisplayWordsTableElement(ToUnderlying(word_id));
    }
    static byte GetDisplayWordRow(WordIdType word_id) { return GetDisplayWord(word_id).GetRow(); }
    static byte GetDisplayWordColumn(WordIdType word_id) { return GetDisplayWord(word_id).GetColumn(); }
    static byte GetDisplayWordLength(WordIdType word_id) { return GetDisplayWord(word_id).GetLength(); }

    // set methods

    // methods
    static bool IsWordIdValid(WordIdType word_id) { return word_id < WordIdType::kNumberOfWords; }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
