// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The base of the overlays, with the idle and show states they share.
 */

#ifndef _OVERLAY_H_
#define _OVERLAY_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"
#include "Text.h"
#include "Scheduler.h"

#include <type_traits>

/* Overlay configuration parameter */

/* Overlay parameter */

template <typename Derived> class Overlay
{
  public:
    enum class StateType {
        kDisabled,
        kIdle,
        kShow
    };

    using YearType = ClockDate::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;
    using FontType = Text::FontType;

  protected:
    StateType state_{StateType::kDisabled};

    // functions
    /*
     * Protected rather than private with the derived class as a friend, which is what
     * clang-tidy asks for: a friend would open this base's whole private side to it, where
     * protected opens only what is meant for it. What the private constructor would
     * prevent - somebody inheriting this template with another class's Derived - the
     * assertion catches instead, at the first point where Derived is a complete type.
     */
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
    constexpr Overlay() {
        static_assert(std::is_base_of<Overlay, Derived>::value,
                      "Overlay is a CRTP base: Derived has to be the class that inherits it");
    }
    ~Overlay() { }

  private:
    static constexpr SecondType kSecondToStartShow{30u};

    /*
     * Named rather than written twice, so the initialisers below and ResetToDefaults()
     * cannot fall apart - a reset that put different values in than a clock starts with
     * would be a second set of defaults to keep in step.
     */
    static constexpr MinuteType kPeriodInMinutesInitValue{1u};
    static constexpr SecondType kEnduranceInSecondsInitValue{1u};
    static constexpr MonthType kMonthInitValue{0u};
    static constexpr DayType kDayInitValue{0u};
    static constexpr DayType kValidInDaysInitValue{0u};
    static constexpr byte kSpeedInitValue{1u};
    static constexpr FontType kFontInitValue{Text::FontType::kFont10x10};

    MinuteType period_in_minutes_{kPeriodInMinutesInitValue};
    SecondType endurance_in_seconds_{kEnduranceInSecondsInitValue};
    MonthType month_{kMonthInitValue};
    DayType day_{kDayInitValue};
    DayType valid_in_days_{kValidInDaysInitValue};
    byte speed_{kSpeedInitValue};
    FontType font_{kFontInitValue};

    // functions
    Derived& Underlying() { return static_cast<Derived&>(*this); }
    Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

    static bool IsPeriodValid(MinuteType period) { return period > 0u; }
    static bool IsEnduranceValid(SecondType endurance) { return endurance > 0u; }
    bool IsDateSet() const { return (month_ != 0u) || (day_ != 0u); }
    bool IsDayAndMonthSet() const { return IsDaySet() && IsMonthSet(); }
    bool IsMonthMatching(MonthType current_month) const { return current_month == month_; }
    bool IsDayMatching(DayType current_day) const { return current_day == day_; }
    bool IsDateMatchingDayAndMonthSet(MonthType current_month, DayType current_day) const { return IsMonthMatching(current_month) && IsDayMatching(current_day); }
    bool IsDateMatchingMonthSet(MonthType current_month) const { return current_month == month_; }
    bool IsDateMatchingDaySet(DayType current_day) const { return current_day == day_; }
    bool IsMinuteMatching(MinuteType current_minute) const { return !(current_minute % period_in_minutes_); }
    static bool IsSecondMatching(SecondType current_second) { return current_second == kSecondToStartShow; }
    bool IsDaySet() const { return day_ != 0u; }
    bool IsMonthSet() const { return month_ != 0u; }
    bool IsValidInDaysSet() const { return valid_in_days_ != 0; }
    bool IsNextYear(MonthType current_month) const { return current_month < month_; }

    bool IsTimeMatching(MinuteType current_minute, SecondType current_second) const { return IsMinuteMatching(current_minute) && IsSecondMatching(current_second); }
    bool IsDateMatching(MonthType current_month, DayType current_day) const {
        if(IsDayAndMonthSet()) { return IsDateMatchingDayAndMonthSet(current_month, current_day); }
        if(IsMonthSet()) { return IsDateMatchingMonthSet(current_month); }
        if(IsDaySet()) { return IsDateMatchingDaySet(current_day); }
        return false;
    }

    bool IsDateValid(ClockDate current_date) const {
        YearType year = current_date.GetYear();
        if(IsNextYear(current_date.GetMonth())) { year--;}
        return current_date.GetPassedDays(year, month_, day_) <= valid_in_days_;
    }

    SecondType CheckDateAndSetStateToShow(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(IsValidInDaysSet() && IsDayAndMonthSet()) {
            if(IsDateValid(current_date)) { return SetStateToShow(current_date, current_time); }
        } else if(IsDateMatching(current_date.GetMonth(), current_date.GetDay())) {
            return SetStateToShow(current_date, current_time);
        }
        return show_timer_in_seconds;
    }

    SecondType SetStateToShow(ClockDate current_date, ClockTime current_time) {
        state_ = StateType::kShow;
        Text::GetInstance().SetTaskCycle(Scheduler::ConvertSpeedToTaskCycle(speed_));
        Underlying().SetStateToShow(current_date, current_time);
        return endurance_in_seconds_;
    }

    void SetStateToIdle(ClockDate current_date, ClockTime current_time) {
        state_ = StateType::kIdle;
        Underlying().SetStateToIdle(current_date, current_time);
    }

    SecondType ShowTimerTask(SecondType show_timer_in_seconds) {
        return DecrementShowTimer(show_timer_in_seconds);
    }

    SecondType IdleTask(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(!CanShow()) { return show_timer_in_seconds; }

        if(IsTimeMatching(current_time.GetMinute(), current_time.GetSecond())) {
            if(IsDateSet()) { return CheckDateAndSetStateToShow(show_timer_in_seconds, current_date, current_time); }
            else { return SetStateToShow(current_date, current_time); }
        }
        return show_timer_in_seconds;
    }

    SecondType ShowTask(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(IsShowTimerExpired(show_timer_in_seconds)) { SetStateToIdle(current_date, current_time); }
        return ShowTimerTask(show_timer_in_seconds);
    }

    static bool IsShowTimerExpired(SecondType show_timer_in_seconds) { return show_timer_in_seconds == 0u; }
    static SecondType DecrementShowTimer(SecondType show_timer_in_seconds) {
        if(show_timer_in_seconds > 0u) { return show_timer_in_seconds - 1u; }
        else { return 0u; }
    }

  public:
    /*
     * The two transitions the period raster makes on its own, opened up for a caller that
     * wants the overlay now - the "show now" remote procedure calls. They are the same
     * transitions and not a second path into the show state, so an overlay started by
     * hand ends through the show timer like any other.
     *
     * Whether the overlay may start at all is not decided here: only Overlays knows
     * whether another one already holds the display, and it owns the timer this hands
     * back.
     */
    SecondType StartShow(ClockDate current_date, ClockTime current_time) {
        return SetStateToShow(current_date, current_time);
    }
    void StopShow(ClockDate current_date, ClockTime current_time) {
        SetStateToIdle(current_date, current_time);
    }

    /*
     * Whether the overlay has anything to show at all, which is not the same as being
     * switched on: the temperature has nothing until a chip has answered, and a clock
     * built without one never will. Asked before every start, by the period raster and by
     * the "show now" call alike, so that an overlay with nothing to say does not hold the
     * display for its endurance. The date and the text always have something.
     *
     * Named apart from the IsReady() each overlay answers, rather than being the same name
     * in both places like SetStateToShow(): a derived member of the same name would hide
     * this one, and a caller outside the overlays - Overlays itself - would then reach the
     * private hook instead of this.
     */
    bool CanShow() const { return Underlying().IsReady(); }

	// get methods
	StateType GetState() const { return state_; }
    MinuteType GetPeriodInMinutes() const { return period_in_minutes_; }
    SecondType GetEnduranceInSeconds() const { return endurance_in_seconds_; }
    MonthType GetMonth() const { return month_; }
    DayType GetDay() const { return day_; }
    DayType GetValidInDays() const { return valid_in_days_; }
    byte GetSpeed() const { return speed_; }
    FontType GetFont() const { return font_; }
    bool GetIsActive() const { return state_ != StateType::kDisabled; }

	// set methods
    StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) {
        if(IsPeriodValid(period_in_minutes)) {
            period_in_minutes_ = period_in_minutes;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) {
        if(IsEnduranceValid(endurance_in_seconds)) {
            endurance_in_seconds_ = endurance_in_seconds;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    StdReturnType SetFont(FontType font) {
        if(Text::IsFontValid(font)) {
            font_ = font;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
    void SetMonth(MonthType month) { month_ = month; }
    void SetDay(DayType day) { day_ = day; }
    void SetValidInDays(DayType valid_in_days) { valid_in_days_ = valid_in_days; }
    void SetSpeed(byte speed) { speed_ = speed; }
    void SetIsActive(bool is_active) { if(is_active) { EnableIsActive(); } else { DisableIsActive(); } }

	// methods
    /*
     * Back to what a clock that was never configured has, now that the overlays are
     * stored and a reset has something to undo.
     *
     * The derived hook is named apart from this for the reason CanShow() gives: a
     * ResetToDefaults() in an overlay would hide this one, and everything the base owns
     * would then quietly survive the reset.
     */
    void ResetToDefaults() {
        period_in_minutes_ = kPeriodInMinutesInitValue;
        endurance_in_seconds_ = kEnduranceInSecondsInitValue;
        month_ = kMonthInitValue;
        day_ = kDayInitValue;
        valid_in_days_ = kValidInDaysInitValue;
        speed_ = kSpeedInitValue;
        font_ = kFontInitValue;
        state_ = StateType::kDisabled;
        Underlying().ResetOwnToDefaults();
    }

    void EnableIsActive() { if(state_ == StateType::kDisabled) { state_ = StateType::kIdle; } }
    void DisableIsActive() { state_ = StateType::kDisabled; }

    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == StateType::kIdle) { return IdleTask(show_timer_in_seconds, current_date, current_time); }
        if(state_ == StateType::kShow) { return ShowTask(show_timer_in_seconds, current_date, current_time); }
        return 0u;
    }
};

#endif
