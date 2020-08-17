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
/**     \file       ClockWords.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _CLOCKWORDS_H_
#define _CLOCKWORDS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "array"
#include "DisplayWords.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ClockWords configuration parameter */


/* ClockWords parameter */
#define CLOCKWORDS_MAX_NUMBER_OF_HOUR_WORDS          2u
#define CLOCKWORDS_MAX_NUMBER_OF_MINUTE_WORDS        3u

#define CLOCKWORDS_IT_IS_NUMBER_OF_WORDS             2u
#define CLOCKWORDS_MAX_NUMBER_OF_WORDS               (CLOCKWORDS_MAX_NUMBER_OF_HOUR_WORDS + CLOCKWORDS_MAX_NUMBER_OF_MINUTE_WORDS + CLOCKWORDS_IT_IS_NUMBER_OF_WORDS)

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class ClockWords
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using WordsListType = std::array<DisplayWords::WordIdType, CLOCKWORDS_MAX_NUMBER_OF_WORDS>;
    using HourWordsType = std::array<DisplayWords::WordIdType, CLOCKWORDS_MAX_NUMBER_OF_HOUR_WORDS>;
    using MinutesWordsType = std::array<DisplayWords::WordIdType, CLOCKWORDS_MAX_NUMBER_OF_MINUTE_WORDS>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    bool ShowItIs;
    HourWordsType HourWords;
    MinutesWordsType MinuteWords;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr ClockWords() : ShowItIs(false), HourWords{DisplayWords::WORD_NONE}, MinuteWords{DisplayWords::WORD_NONE} { }
    constexpr ClockWords(bool sShowItIs, HourWordsType sHourWords, MinutesWordsType sMinuteWords) 
        : ShowItIs(sShowItIs), HourWords(sHourWords), MinuteWords(sMinuteWords) { }
    ~ClockWords() { }
    
    bool operator==(const ClockWords& sClockWords);
    bool operator!=(const ClockWords& sClockWords);

    // get methods
    bool getShowItIs() const { return ShowItIs; }
    HourWordsType getHourWords() const { return HourWords; }
    DisplayWords::WordIdType getHourWord(byte Index) const { return HourWords[Index]; }
    MinutesWordsType getMinuteWords() const { return MinuteWords; }
    DisplayWords::WordIdType getMinuteWord(byte Index) const { return MinuteWords[Index]; }
        
    WordsListType getWordsList() const;

    // set methods
    void setShowItIs(bool sShowItIs) {ShowItIs = sShowItIs;}
    void setMinuteWords(MinutesWordsType sMinutWords) { MinuteWords = sMinutWords; }
    void setHourWords(HourWordsType sHourWords) { HourWords = sHourWords; }

    // methods

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
