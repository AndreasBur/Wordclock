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
#ifndef _CLOCK_WORDS_H_
#define _CLOCK_WORDS_H_

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
#define CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS          2u
#define CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS        3u

#define CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS             2u
#define CLOCK_WORDS_MAX_NUMBER_OF_WORDS               (CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS + CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS + CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS)

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   C L O C K   W O R D S
******************************************************************************************************************************************************/
class ClockWords
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using WordsListType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_WORDS>;
    using HourWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS>;
    using MinutesWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    bool show_it_is_;
    HourWordsType hour_words_;
    MinutesWordsType minute_words_;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr ClockWords() : show_it_is_(false), hour_words_{DisplayWords::WordIdType::kNone}, minute_words_{DisplayWords::WordIdType::kNone} { }
    constexpr ClockWords(bool show_it_is, HourWordsType hour_words, MinutesWordsType minute_words)
        : show_it_is_(show_it_is), hour_words_(hour_words), minute_words_(minute_words) { }
    ~ClockWords() { }

    bool operator==(const ClockWords& clock_words) const;
    bool operator!=(const ClockWords& clock_words) const;

    // get methods
    bool GetShowItIs() const { return show_it_is_; }
    HourWordsType GetHourWords() const { return hour_words_; }
    DisplayWords::WordIdType GetHourWord(byte index) const { return hour_words_[index]; }
    MinutesWordsType GetMinuteWords() const { return minute_words_; }
    DisplayWords::WordIdType GetMinuteWord(byte index) const { return minute_words_[index]; }

    WordsListType GetWordsList() const;

    // set methods
    void SetShowItIs(bool show_it_is) { show_it_is_ = show_it_is; }
    void SetMinuteWords(MinutesWordsType minut_words) { minute_words_ = minut_words; }
    void SetHourWords(HourWordsType hour_words) { hour_words_ = hour_words; }

    // methods

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
