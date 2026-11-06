// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The words that tell one time: optional "es ist", hour words and minute words.
 */

#ifndef _CLOCK_WORDS_H_
#define _CLOCK_WORDS_H_

#include "Arduino.h"
#include <array>
#include "DisplayWords.h"

#define CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS          2u /*!< The places for hour words. */
#define CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS        3u /*!< The places for minute words. */

#define CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS             2u /*!< The words of ES IST. */
/*! \brief The places for all the words of a time: ES IST, the minute words and the hour words. */
#define CLOCK_WORDS_MAX_NUMBER_OF_WORDS               (CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS + CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS + CLOCK_WORDS_IT_IS_NUMBER_OF_WORDS)

/*!
 * \brief The words that tell one time: whether ES IST stands, the hour words and the minute words.
 *
 * A place without a word holds `DisplayWords::WordIdType::kNone`. Clock::GetClockWords() fills in
 * the words of a time.
 */
class ClockWords
{
  public:
    /*! \brief All the words of one time in one list, as GetWordsList() makes it. */
    using WordsListType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_WORDS>;
    /*! \brief The hour words of one time, such as ZWEI UHR. */
    using HourWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS>;
    /*! \brief The minute words of one time, such as ZEHN VOR HALB. */
    using MinutesWordsType = std::array<DisplayWords::WordIdType, CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS>;
  
  private:
    bool show_it_is_;
    HourWordsType hour_words_;
    MinutesWordsType minute_words_;

  public:
    /*! \brief Makes the words of no time: without ES IST, and `kNone` in every place. */
    constexpr ClockWords() : show_it_is_(false), hour_words_{DisplayWords::WordIdType::kNone}, minute_words_{DisplayWords::WordIdType::kNone} { }

    /*!
     * \brief Makes the words of one time.
     *
     * \param[in] show_it_is True if ES IST stands.
     * \param[in] hour_words The hour words.
     * \param[in] minute_words The minute words.
     */
    constexpr ClockWords(bool show_it_is, HourWordsType hour_words, MinutesWordsType minute_words)
        : show_it_is_(show_it_is), hour_words_(hour_words), minute_words_(minute_words) { }

    /*! \brief Destroys the words. */
    ~ClockWords() = default;

    /*!
     * \brief Returns whether `clock_words` holds the same words.
     *
     * \param[in] clock_words The words to compare with.
     * \return True if ES IST, the hour words and the minute words are all the same.
     */
    bool operator==(const ClockWords& clock_words) const;

    /*!
     * \brief Returns whether `clock_words` holds other words.
     *
     * \param[in] clock_words The words to compare with.
     * \return True if ES IST, the hour words or the minute words differ.
     */
    bool operator!=(const ClockWords& clock_words) const;

    /*! \brief Returns whether ES IST stands. */
    bool GetShowItIs() const { return show_it_is_; }

    /*! \brief Returns the hour words. */
    HourWordsType GetHourWords() const { return hour_words_; }

    /*!
     * \brief Returns the hour word at `index`.
     *
     * \param[in] index The place, from 0 and below CLOCK_WORDS_MAX_NUMBER_OF_HOUR_WORDS; it is not
     *                  checked.
     * \return The word, or `kNone` if the place holds none.
     */
    DisplayWords::WordIdType GetHourWord(byte index) const { return hour_words_[index]; }

    /*! \brief Returns the minute words. */
    MinutesWordsType GetMinuteWords() const { return minute_words_; }

    /*!
     * \brief Returns the minute word at `index`.
     *
     * \param[in] index The place, from 0 and below CLOCK_WORDS_MAX_NUMBER_OF_MINUTE_WORDS; it is
     *                  not checked.
     * \return The word, or `kNone` if the place holds none.
     */
    DisplayWords::WordIdType GetMinuteWord(byte index) const { return minute_words_[index]; }

    /*!
     * \brief Returns the words as one list, in the order they are read.
     *
     * ES IST comes first where it stands, then the minute words, then the hour words, with no
     * `kNone` between them. The places left over hold `kNone`.
     *
     * \return The list of the words.
     */
    WordsListType GetWordsList() const;

    /*!
     * \brief Sets whether ES IST stands.
     *
     * \param[in] show_it_is True if ES IST stands.
     */
    void SetShowItIs(bool show_it_is) { show_it_is_ = show_it_is; }

    /*!
     * \brief Sets the minute words.
     *
     * \param[in] minut_words The minute words.
     */
    void SetMinuteWords(MinutesWordsType minut_words) { minute_words_ = minut_words; }

    /*!
     * \brief Sets the hour words.
     *
     * \param[in] hour_words The hour words.
     */
    void SetHourWords(HourWordsType hour_words) { hour_words_ = hour_words; }
};

#endif
