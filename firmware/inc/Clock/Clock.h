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
#include "Display.h"
#include "ClockWords.h"
#include <array>

/* Clock configuration parameter */
#define CLOCK_SHOW_IT_IS_PERMANENTLY            STD_ON
#define CLOCK_INITIAL_MODE                      ModeType::kWessi

/* Clock  parameter */
/* Hour */
#define CLOCK_NUMBER_OF_HOUR_MODES              2u
#define CLOCK_NUMBER_OF_HOURS                   12u

#define CLOCK_NUMBER_OF_HOURS_PER_DAY           24u
#define CLOCK_NUMBER_OF_MINUTES_PER_HOUR        60u

/* Minute */
#define CLOCK_MINUTE_STEP_IN_MINUTES            5u
#define CLOCK_NUMBER_OF_MINUTE_STEPS            12u

class Clock
{
  public:
    enum class ModeType {
        kWessi,
        kOssi,
        kRheinRuhr,
        kSchwaben,
        kNumberOfModes
    };
    static constexpr byte kNumberOfModes{static_cast<byte>(ModeType::kNumberOfModes)};

    enum class HourModeType {
        kFullHour,
        kNoFullHour
    };

    struct MinuteType {
        HourModeType hour_mode;
        byte hour_offset;
        ClockWords::MinutesWordsType words;
    };

    struct TimeType {
        byte hour;
        byte minute;
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

    // functions
    constexpr Clock(ModeType mode) : mode_(mode) { }
    ~Clock() { }

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

    static bool CalculateItIs(byte minute) {
        byte minute_steps = minute / CLOCK_MINUTE_STEP_IN_MINUTES;
        // show "it is" only to full and half hour
        if(minute_steps == 0u || minute_steps == (CLOCK_NUMBER_OF_MINUTE_STEPS / 2u)) {
            return true;
        } else {
            return false;
        }
    }

    static byte Transform24hTo12hFormat(byte hour) {
        return hour % CLOCK_NUMBER_OF_HOURS;
    }

    static constexpr ModeType ToNextMode(ModeType mode) {
        return (ToUnderlying(mode) + 1u >= kNumberOfModes) ? ModeType::kWessi : static_cast<ModeType>(ToUnderlying(mode) + 1u);
    }

  public:
    static Clock& GetInstance() {
        static Clock singleton_instance(CLOCK_INITIAL_MODE);
        return singleton_instance;
    }

    // get methods
    ModeType GetMode() const { return mode_; }
    bool GetShowItIsPermanently() const { return show_it_is_permanently_; }
    StdReturnType GetClockWords(byte, byte, ClockWords&) const;
    StdReturnType GetClockWords(byte, byte, ClockWordsListType&) const;

    // set methods
    void SetShowItIsPermanently(bool show_it_is_permanently) { show_it_is_permanently_ = show_it_is_permanently; }
    StdReturnType SetMode(ModeType mode)
    {
        if(IsModeValid(mode)) {
            mode_ = mode;
            return kEOk;
        } else {
            return kENotOk;
        }
    }

    // methods
    /*
     * One wording further, wrapping at the last, for a control that cannot name a mode.
     * It draws nothing: the words of the new wording differ from the ones DisplayManager
     * has latched, so its next task finds the change and puts them up the way it does
     * for every other one - with the selected animation, which "8 -M<id>" does not do.
     * Where two wordings say the same thing at the current time there is nothing to
     * redraw, and nothing happens, which is the right answer rather than a missed one.
     */
    void NextMode() { SetMode(ToNextMode(mode_)); }

    void ResetToDefaults() {
        SetMode(CLOCK_INITIAL_MODE);
        show_it_is_permanently_ = kShowItIsPermanentlyInitValue;
    }

    static bool IsModeValid(ModeType mode) { return mode < ModeType::kNumberOfModes; }
    StdReturnType SetTime(TimeType time) const { return SetTime(time.hour, time.minute); }
    StdReturnType SetTime(byte, byte) const;
    static void SetTime(ClockWords::WordsListType clock_words_table) {
        SetTime(clock_words_table, 0u);
    }
    static void SetTime(ClockWords::WordsListType clock_words_table, byte max_length) {
        for(byte index = 0u; index < clock_words_table.size(); index++) {
            Display::GetInstance().SetWord(clock_words_table[index], max_length);
        }
    }

    static StdReturnType Show() { return Display::GetInstance().Show(); }
    StdReturnType Refresh(byte hour, byte minute) const {
        StdReturnType return_value{kEOk};
        if(SetTime(hour, minute) == kENotOk) { return_value = kENotOk; }
        if(Show() == kENotOk) { return_value = kENotOk; }
        return return_value;
    }
};

#endif
