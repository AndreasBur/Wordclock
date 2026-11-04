// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The words that tell one time: optional "es ist", hour words and minute words.
 */

#ifndef _CLOCK_WORDS_H_
#define _CLOCK_WORDS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "array"
#include "DisplayWords.h"

/* ClockWords configuration parameter */

/* ClockWords parameter */
#define CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS          2u
#define CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS        3u

#define CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS             2u
#define CLOCK_WORDS_MAX_NUMBER_OF_WORDS               (CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS + CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS + CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS)

class ClockWords
{
  public:
    using WordsListType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_WORDS>;
    using HourWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS>;
    using MinutesWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS>;
  
  private:
    bool show_it_is_;
    HourWordsType hour_words_;
    MinutesWordsType minute_words_;

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
