// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The clock: turns a time into the words to show, in the selectable dialects.
 */

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"

/*!
 * \brief Whether a new clock shows ES IST at every time, STD_ON, or only at the full and the half
 *        hour, STD_OFF.
 *
 * It is the default of Clock::SetShowItIsPermanently(), which changes it while the clock runs.
 */
#define CLOCK_SHOW_IT_IS_PERMANENTLY            STD_ON
#define CLOCK_INITIAL_MODE                      ModeType::kWessi /*!< The mode of a new clock. */

#define CLOCK_NUMBER_OF_HOUR_MODES              2u /*!< The number of hour modes, HourModeType. */
#define CLOCK_NUMBER_OF_HOURS                   12u /*!< The hours of the clock face. */

#define CLOCK_NUMBER_OF_HOURS_PER_DAY           24u /*!< The hours of a day. */
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR        60u /*!< The minutes of an hour. */

#define CLOCK_MINUTE_STEP_IN_MINUTES            5u /*!< The minutes of one step of the words. */
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12u /*!< The steps of an hour. */

/*!
 * \brief The clock: turns a time into the words to light, in one of four regional modes.
 *
 * A time is said in steps of five minutes, rounded down, on a face of twelve hours, in the mode
 * set with SetMode(). ES IST stands at every time, or only at the full and the half hour.
 *
 * SetTime() lights the words in the display's pixels and leaves lit what was lit before; Show()
 * asks the display to show them.
 */
class Clock
{
  public:
    /*!
     * \brief The regional modes: the ways of saying the time in German.
     *
     * They differ in how the quarters are said and in whether 20 and 40 minutes past are counted
     * from the hour or from the half hour; each value says 3:15 and 3:20 as an example.
     */
    enum class ModeType {
        kWessi, /*!< Wessi, the default: VIERTEL NACH DREI at 3:15, ZEHN VOR HALB VIER at 3:20. */
        kOssi, /*!< Ossi: VIERTEL VIER at 3:15, ZEHN VOR HALB VIER at 3:20. */
        kRheinRuhr, /*!< Rhein-Ruhr: VIERTEL NACH DREI at 3:15, ZWANZIG NACH DREI at 3:20. */
        kSchwaben, /*!< Schwaben: VIERTEL VIER at 3:15, ZWANZIG NACH DREI at 3:20. */
        kNumberOfModes /*!< The number of modes; not a mode. */
    };

    /*! \brief The number of modes, four. */
    static constexpr byte kNumberOfModes{static_cast<byte>(ModeType::kNumberOfModes)};

    /*! \brief How the hour is said: with UHR at the full hour, without it at any other time. */
    enum class HourModeType {
        kFullHour, /*!< The full hour, with UHR, as in EIN UHR. */
        kNoFullHour /*!< Any other time, without UHR, as in ZEHN NACH EINS. */
    };

    /*! \brief One step of five minutes in a mode: how the hour is said, and the minute words. */
    struct MinuteType {
        HourModeType hour_mode; /*!< Whether the hour is said as a full hour, with UHR. */
        /*!
         * \brief What is added to the hour: 1 where the words count towards the next hour, as ZEHN
         *        VOR HALB VIER at 3:20 does, and 0 otherwise.
         */
        byte hour_offset;
        ClockWords::MinutesWordsType words; /*!< The minute words, `kNone` in each unused place. */
    };

    /*! \brief A time of day in hours and minutes, as SetTime(TimeType) takes it. */
    struct TimeType {
        byte hour; /*!< The hour, 0 to 23. */
        byte minute; /*!< The minute, 0 to 59. */
    };

  private:
    using HourType = ClockWords::HourWordsType;
    using ClockWordsListType = ClockWords::WordsListType;

    using HourTableElementType = HourType;
    using MinuteTableElementType = MinuteType;

  private:
    static constexpr bool kShowItIsPermanentlyInitValue{CLOCK_SHOW_IT_IS_PERMANENTLY == STD_ON};

    ModeType mode_{CLOCK_INITIAL_MODE};
    /*
     * Whether "es ist" stands in every time or only in the two it is said in. A setting
     * rather than the compile-time switch it was: both are ways German tells the time, the
     * clock can say either, and which one somebody wants is not a property of the build.
     */
    bool show_it_is_permanently_{kShowItIsPermanentlyInitValue};

    static const HourType kHoursTable[][CLOCK_NUMBER_OF_HOURS];
    static const MinuteType kMinutesTable[][CLOCK_NUMBER_OF_MINUTE_STEPS];

    constexpr Clock(ModeType mode) : mode_(mode) { }
    ~Clock() = default;

    MinuteTableElementType GetMinutesTableElement(byte minute) const {
        MinuteTableElementType minutes_table_element;
        memcpy_P(&minutes_table_element, &kMinutesTable[ToUnderlying(mode_)][minute / CLOCK_MINUTE_STEP_IN_MINUTES], sizeof(MinuteTableElementType));
        return minutes_table_element;
    }

    static HourTableElementType GetHoursTableElement(HourModeType hour_mode, byte hour) {
        HourTableElementType hours_table_element;
        memcpy_P(&hours_table_element, &kHoursTable[ToUnderlying(hour_mode)][hour], sizeof(HourTableElementType));
        return hours_table_element;
    }

    static bool CalculateItIs(byte minute);

    static byte Transform24hTo12hFormat(byte hour) {
        return hour % CLOCK_NUMBER_OF_HOURS;
    }

    static constexpr ModeType ToNextMode(ModeType mode) {
        return (ToUnderlying(mode) + 1u >= kNumberOfModes) ? ModeType::kWessi : static_cast<ModeType>(ToUnderlying(mode) + 1u);
    }

  public:
    /*! \brief Returns the one clock. */
    static Clock& GetInstance() {
        static Clock singleton_instance(CLOCK_INITIAL_MODE);
        return singleton_instance;
    }

    /*! \brief Returns the mode. */
    ModeType GetMode() const { return mode_; }

    /*!
     * \brief Returns whether ES IST stands at every time, rather than only at the full and the half
     *        hour.
     */
    bool GetShowItIsPermanently() const { return show_it_is_permanently_; }

    /*!
     * \brief Returns in `clock_words` the words that say `hour`:`minute` in the current mode.
     *
     * The minute is rounded down to its step of five. The hour is taken modulo 12, and where the
     * words count towards the next hour, as ZEHN VOR HALB VIER at 3:20 does, they name that hour.
     * ES IST stands as GetShowItIsPermanently() says. Nothing is lit.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \param[out] clock_words The words of the time.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59; `clock_words` is then
     *         left as it was.
     */
    StdReturnType GetClockWords(byte hour, byte minute, ClockWords& clock_words) const;

    /*!
     * \brief Returns in `clock_words_list` the words that say `hour`:`minute` in the current mode,
     *        as one flat list.
     *
     * The list is what ClockWords::GetWordsList() makes of the words the other overload returns.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \param[out] clock_words_list The words of the time.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59; `clock_words_list` is
     *         then left as it was.
     */
    StdReturnType GetClockWords(byte hour, byte minute, ClockWordsListType& clock_words_list) const;

    /*!
     * \brief Sets whether ES IST stands at every time, or only at the full and the half hour.
     *
     * \param[in] show_it_is_permanently True for every time, false for the full and the half hour
     *                                   only.
     */
    void SetShowItIsPermanently(bool show_it_is_permanently) { show_it_is_permanently_ = show_it_is_permanently; }

    /*!
     * \brief Sets the mode.
     *
     * \param[in] mode The mode.
     * \return kEOk, or kENotOk if `mode` is not a mode, such as `kNumberOfModes`; the mode is then
     *         left as it was.
     */
    StdReturnType SetMode(ModeType mode);

    /*!
     * \brief Switches to the next mode, from the last one back to the first.
     *
     * One wording further, wrapping at the last, for a control that cannot name a mode. It draws
     * nothing: the words of the new wording differ from the ones DisplayManager has latched, so its
     * next task finds the change and puts them up the way it does for every other one - with the
     * animation the mode provides, as after `8 -M<id>`. Where two wordings say the same thing at
     * the current time there is nothing to redraw, and nothing happens, which is the right answer
     * rather than a missed one.
     */
    void NextMode() { SetMode(ToNextMode(mode_)); }

    /*!
     * \brief Puts the clock back to how a new clock has it: the mode CLOCK_INITIAL_MODE, and ES IST
     *        as CLOCK_SHOW_IT_IS_PERMANENTLY says.
     */
    void ResetToDefaults() {
        SetMode(CLOCK_INITIAL_MODE);
        show_it_is_permanently_ = kShowItIsPermanentlyInitValue;
    }

    /*!
     * \brief Returns whether `mode` is one of the modes.
     *
     * \param[in] mode The mode to check.
     * \return True if `mode` lies below `kNumberOfModes`.
     */
    static bool IsModeValid(ModeType mode) { return mode < ModeType::kNumberOfModes; }

    /*!
     * \brief Lights the words that say `time` in the current mode.
     *
     * \param[in] time The time, its hour 0 to 23 and its minute 0 to 59.
     * \return kEOk, or kENotOk if the hour is above 23 or the minute above 59, and then nothing is
     *         lit, or if the display did not take a word; the other words are lit all the same.
     */
    StdReturnType SetTime(TimeType time) const { return SetTime(time.hour, time.minute); }

    /*!
     * \brief Lights the words that say `hour`:`minute` in the current mode.
     *
     * ES IST where it stands, the hour words and the minute words go into the display's pixels
     * through Display::SetWord(), each word whole. What was lit before stays lit.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59, and then nothing is lit,
     *         or if the display did not take a word; the other words are lit all the same.
     */
    StdReturnType SetTime(byte hour, byte minute) const;

    /*!
     * \brief Lights the words of `clock_words_table`, each word whole.
     *
     * \param[in] clock_words_table The words, as GetClockWords() returns them; a place that holds
     *                              `kNone` lights nothing.
     */
    static void SetTime(ClockWords::WordsListType clock_words_table) {
        SetTime(clock_words_table, 0u);
    }

    /*!
     * \brief Lights the first `max_length` letters of each word of `clock_words_table`.
     *
     * What was lit before stays lit.
     *
     * \param[in] clock_words_table The words, as GetClockWords() returns them; a place that holds
     *                              `kNone` lights nothing.
     * \param[in] max_length The letters to light of each word; 0, or a length the word does not
     *                       exceed, lights it whole.
     */
    static void SetTime(ClockWords::WordsListType clock_words_table, byte max_length);

    /*!
     * \brief Asks the display to show what is lit, through Display::Show().
     *
     * \return What Display::Show() returns, kEOk on every backend.
     */
    static StdReturnType Show();

    /*!
     * \brief Lights the words that say `hour`:`minute` in the current mode, and has the display
     *        show them.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if SetTime(byte, byte) or Show() returns it; Show() is called either
     *         way.
     */
    StdReturnType Refresh(byte hour, byte minute) const;
};

#endif
