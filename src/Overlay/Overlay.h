// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The base of the overlays, with the idle and show states they share.
 */

#ifndef WORDCLOCK_SRC_OVERLAY_OVERLAY_H_
#define WORDCLOCK_SRC_OVERLAY_OVERLAY_H_

#include <type_traits>

#include "Arduino.h"
#include "ClockDateTime.h"
#include "RealTimeClock.h"
#include "Scheduler.h"
#include "StandardTypes.h"
#include "Text.h"

/*!
 * \brief The base of the date, temperature and text overlays: when an overlay starts, how long it
 *        shows, and the settings that decide both.
 *
 * An overlay scrolls a text over the clock through Text for a while. A new overlay is switched
 * off, StateType::kDisabled. Switched on, it waits in StateType::kIdle and starts by itself at
 * second 30 of each minute that its period divides, SetPeriodInMinutes(), if it has anything to
 * show, CanShow(), and its date allows it. It then shows, StateType::kShow, and its text starts
 * again each time it has scrolled through, until the endurance, SetEnduranceInSeconds(), has run
 * out. Then it goes back to StateType::kIdle.
 *
 * The month, the day and the valid-in-days limit the days on which it starts:
 * - With neither a month nor a day, it starts on every day.
 * - With a month, a day and a valid-in-days above 0, it starts on that date and on the
 *   valid-in-days days after it, across the turn of the year too.
 * - Otherwise the valid-in-days is not used: with a month and a day, it starts on that date each
 *   year; with a month only, on every day of that month; with a day only, on that day of every
 *   month.
 *
 * Overlays holds the overlays, calls Task() once a second, and keeps the one show timer they
 * share. It is Overlays that lets only one overlay show at a time.
 *
 * Each overlay derives from this class with itself as `Derived`, makes this class its friend, and
 * answers five private hooks, which are not virtual: `SetStateToShow(ClockDate, ClockTime)` starts
 * its text, `SetText()` starts it again, `SetStateToIdle(ClockDate, ClockTime)` stops it,
 * `IsReady()` says whether it has anything to show, and `ResetOwnToDefaults()` resets what it adds
 * to the settings of this class.
 *
 * \tparam Derived The overlay that derives from this class.
 */
template <typename Derived>
class Overlay {
 public:
  /*! \brief The state of an overlay. */
  enum class StateType {
    kDisabled, /*!< Switched off: Task() does nothing. */
    kIdle,     /*!< Switched on, and waiting for its period to start. */
    kShow      /*!< Showing: its text scrolls until the show timer has run out. */
  };

  /*! \brief A year in full, ClockDate::YearType. */
  using YearType = ClockDate::YearType;
  /*! \brief A month, 1 for January to 12 for December, ClockDateTime::MonthType. */
  using MonthType = ClockDateTime::MonthType;
  /*! \brief A day of the month or a number of days, ClockDateTime::DayType. */
  using DayType = ClockDateTime::DayType;
  /*! \brief An hour, ClockDateTime::HourType. */
  using HourType = ClockDateTime::HourType;
  /*! \brief A minute or a number of minutes, ClockDateTime::MinuteType. */
  using MinuteType = ClockDateTime::MinuteType;
  /*! \brief A second or a number of seconds, ClockDateTime::SecondType. */
  using SecondType = ClockDateTime::SecondType;
  /*! \brief A font of the text output, Text::FontType. */
  using FontType = Text::FontType;

 protected:
  /*! \brief The state the overlay is in; the derived overlay reads it in its `Task()`. */
  StateType state_{StateType::kDisabled};

  /*! \brief Makes an overlay with the settings of a new clock, switched off. */
  // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
  constexpr Overlay() {
    /*
     * Protected rather than private with the derived class as a friend, which is what
     * clang-tidy asks for: a friend would open this base's whole private side to it, where
     * protected opens only what is meant for it. What the private constructor would
     * prevent - somebody inheriting this template with another class's Derived - the
     * assertion catches instead, at the first point where Derived is a complete type.
     */
    static_assert(std::is_base_of_v<Overlay, Derived>,
                  "Overlay is a CRTP base: Derived has to be the class that inherits it");
  }

  /*! \brief Destroys the overlay. */
  ~Overlay() = default;

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

  Derived& Underlying() { return static_cast<Derived&>(*this); }
  Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

  static bool IsPeriodValid(MinuteType period) { return period > 0u; }
  static bool IsEnduranceValid(SecondType endurance) { return endurance > 0u; }
  bool IsDateSet() const { return (month_ != 0u) || (day_ != 0u); }
  bool IsDayAndMonthSet() const { return IsDaySet() && IsMonthSet(); }
  bool IsMonthMatching(MonthType current_month) const { return current_month == month_; }
  bool IsDayMatching(DayType current_day) const { return current_day == day_; }
  bool IsDateMatchingDayAndMonthSet(MonthType current_month, DayType current_day) const {
    return IsMonthMatching(current_month) && IsDayMatching(current_day);
  }
  bool IsDateMatchingMonthSet(MonthType current_month) const { return current_month == month_; }
  bool IsDateMatchingDaySet(DayType current_day) const { return current_day == day_; }
  bool IsMinuteMatching(MinuteType current_minute) const {
    return (current_minute % period_in_minutes_) == 0;
  }
  static bool IsSecondMatching(SecondType current_second) {
    return current_second == kSecondToStartShow;
  }
  bool IsDaySet() const { return day_ != 0u; }
  bool IsMonthSet() const { return month_ != 0u; }
  bool IsValidInDaysSet() const { return valid_in_days_ != 0; }
  bool IsBeforeInYear(MonthType current_month, DayType current_day) const {
    return (current_month < month_) || ((current_month == month_) && (current_day < day_));
  }

  bool IsTimeMatching(MinuteType current_minute, SecondType current_second) const {
    return IsMinuteMatching(current_minute) && IsSecondMatching(current_second);
  }
  bool IsDateMatching(MonthType current_month, DayType current_day) const {
    if (IsDayAndMonthSet()) {
      return IsDateMatchingDayAndMonthSet(current_month, current_day);
    }
    if (IsMonthSet()) {
      return IsDateMatchingMonthSet(current_month);
    }
    if (IsDaySet()) {
      return IsDateMatchingDaySet(current_day);
    }
    return false;
  }

  /*
   * From the date on: the days since its last time on or before today, this year's or, before
   * it in the year, last year's. GetPassedDays() counts in either direction, and against this
   * year's date alone the window also took in the days before the date within its month.
   */
  bool IsDateValid(ClockDate current_date) const {
    YearType year = current_date.GetYear();
    if (IsBeforeInYear(current_date.GetMonth(), current_date.GetDay())) {
      year--;
    }
    return current_date.GetPassedDays(year, month_, day_) <= valid_in_days_;
  }

  SecondType CheckDateAndSetStateToShow(SecondType show_timer_in_seconds, ClockDate current_date,
                                        ClockTime current_time) {
    if (IsValidInDaysSet() && IsDayAndMonthSet()) {
      if (IsDateValid(current_date)) {
        return SetStateToShow(current_date, current_time);
      }
    } else if (IsDateMatching(current_date.GetMonth(), current_date.GetDay())) {
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

  /*
   * Here rather than in each overlay's own Task(), which hid this one: a call through the base
   * would have skipped the restart, and the three copies were the same.
   */
  void RestartTextOnceScrolledThrough() {
    if (Text::GetInstance().GetState() == Text::StateType::kIdle) {
      Underlying().SetText();
    }
  }

  SecondType ShowTimerTask(SecondType show_timer_in_seconds) {
    return DecrementShowTimer(show_timer_in_seconds);
  }

  SecondType IdleTask(SecondType show_timer_in_seconds, ClockDate current_date,
                      ClockTime current_time) {
    if (!CanShow()) {
      return show_timer_in_seconds;
    }

    if (IsTimeMatching(current_time.GetMinute(), current_time.GetSecond())) {
      if (IsDateSet()) {
        return CheckDateAndSetStateToShow(show_timer_in_seconds, current_date, current_time);
      } else {
        return SetStateToShow(current_date, current_time);
      }
    }
    return show_timer_in_seconds;
  }

  // Counted down first, so that an endurance of n seconds ends the show n seconds after its
  // start; asked first, it ran a second longer.
  SecondType ShowTask(SecondType show_timer_in_seconds, ClockDate current_date,
                      ClockTime current_time) {
    SecondType const remaining_seconds = ShowTimerTask(show_timer_in_seconds);
    if (IsShowTimerExpired(remaining_seconds)) {
      SetStateToIdle(current_date, current_time);
    }
    return remaining_seconds;
  }

  /*
   * Ends a show the way StopShow() does, before the overlay leaves it another way. Switched off
   * or reset while it showed, the overlay left Text scrolling its text over the clock.
   */
  void EndShowIfShowing() {
    if (state_ == StateType::kShow) {
      SetStateToIdle(RealTimeClock::GetInstance().GetDate(),
                     RealTimeClock::GetInstance().GetTime());
    }
  }

  static bool IsShowTimerExpired(SecondType show_timer_in_seconds) {
    return show_timer_in_seconds == 0u;
  }
  static SecondType DecrementShowTimer(SecondType show_timer_in_seconds) {
    if (show_timer_in_seconds > 0u) {
      return static_cast<SecondType>(show_timer_in_seconds - 1u);
    } else {
      return 0u;
    }
  }

 public:
  /*!
   * \brief Starts the overlay now rather than at its next period, for the "show now" procedures.
   *
   * It is the transition the period raster makes on its own, opened up for a caller that wants
   * the overlay now - the "show now" remote procedure calls. It is the same transition and not
   * a second path into the show state, so an overlay started by hand ends through the show timer
   * like any other: the state goes to StateType::kShow, the text output takes the task cycle of
   * the speed, SetSpeed(), and the derived overlay starts its text.
   *
   * Whether the overlay may start at all is not decided here: only Overlays knows whether
   * another one already holds the display, and it owns the timer this hands back. It starts from
   * any state, StateType::kDisabled included, starts the text of an overlay that shows already
   * anew, and does not ask CanShow(); Overlays asks all of that first.
   *
   * \param[in] current_date The date now, which the date overlay shows.
   * \param[in] current_time The time now; none of the three overlays uses it.
   * \return The endurance in seconds, the show timer to count down from.
   */
  SecondType StartShow(ClockDate current_date, ClockTime current_time) {
    return SetStateToShow(current_date, current_time);
  }

  /*!
   * \brief Ends the overlay now rather than when its show timer runs out.
   *
   * It is the transition the run-out show timer makes: the state goes to StateType::kIdle and
   * the derived overlay stops its text, where it stands. The display is left as it is;
   * DisplayManager draws the clock again once no overlay shows. The show timer is not touched
   * here, as Overlays owns it.
   *
   * \param[in] current_date The date now; none of the three overlays uses it.
   * \param[in] current_time The time now; none of the three overlays uses it.
   * \note It goes to StateType::kIdle from any state, so it switches on an overlay that is switched
   *       off. Overlays::Abort() calls it on the overlay that shows only.
   */
  void StopShow(ClockDate current_date, ClockTime current_time) {
    SetStateToIdle(current_date, current_time);
  }

  /*!
   * \brief Returns whether the overlay has anything to show, as the derived overlay's `IsReady()`
   *        answers.
   *
   * Having something to show is not the same as being switched on: the temperature has nothing
   * until a chip has answered, and a clock built without one never will. It is asked before
   * every start, by the period raster in Task() and by the "show now" call in Overlays alike, so
   * that an overlay with nothing to say does not hold the display for its endurance. The date
   * and the text always have something.
   *
   * \return True if the overlay has something to show.
   * \note It is named apart from the `IsReady()` each overlay answers, rather than being the same
   *       name in both places like `SetStateToShow()`: a derived member of the same name would hide
   *       this one, and a caller outside the overlays - Overlays itself - would then reach the
   *       private hook instead of this.
   */
  bool CanShow() const { return Underlying().IsReady(); }

  /*! \brief Returns the state the overlay is in. */
  StateType GetState() const { return state_; }

  /*! \brief Returns the period, in minutes; see SetPeriodInMinutes(). */
  MinuteType GetPeriodInMinutes() const { return period_in_minutes_; }

  /*! \brief Returns the endurance, in seconds; see SetEnduranceInSeconds(). */
  SecondType GetEnduranceInSeconds() const { return endurance_in_seconds_; }

  /*! \brief Returns the month the overlay is limited to, 0 for none. */
  MonthType GetMonth() const { return month_; }

  /*! \brief Returns the day of the month the overlay is limited to, 0 for none. */
  DayType GetDay() const { return day_; }

  /*! \brief Returns the days after its date on which the overlay starts; see SetValidInDays(). */
  DayType GetValidInDays() const { return valid_in_days_; }

  /*! \brief Returns the speed the text scrolls at; see SetSpeed(). */
  byte GetSpeed() const { return speed_; }

  /*! \brief Returns the font the overlay's text is drawn in. */
  FontType GetFont() const { return font_; }

  /*! \brief Returns whether the overlay is switched on: in any state but StateType::kDisabled. */
  bool GetIsActive() const { return state_ != StateType::kDisabled; }

  /*!
   * \brief Sets the period: the overlay starts in each minute of the hour that `period_in_minutes`
   *        divides.
   *
   * It starts at second 30 of such a minute, the minutes counted from the full hour: at a period
   * of 15, in the minutes 0, 15, 30 and 45. A period that does not divide 60 leaves a shorter
   * gap before the full hour, and a period of 60 or more starts the overlay once an hour, in
   * minute 0. A new overlay has a period of 1, every minute.
   *
   * \param[in] period_in_minutes The period, in minutes.
   * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
   */
  StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) {
    if (IsPeriodValid(period_in_minutes)) {
      period_in_minutes_ = period_in_minutes;
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*!
   * \brief Sets the endurance: how long the overlay shows once it has started, in seconds.
   *
   * The overlay takes it at its next start; one that shows already keeps the show timer it
   * started with. A new overlay has an endurance of 1.
   *
   * \param[in] endurance_in_seconds The endurance, in seconds.
   * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it was.
   */
  StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) {
    if (IsEnduranceValid(endurance_in_seconds)) {
      endurance_in_seconds_ = endurance_in_seconds;
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*!
   * \brief Sets the font the overlay's text is drawn in.
   *
   * The text takes it each time it starts, also each time it starts again while the overlay
   * shows. A new overlay has Text::FontType::kFont10x10.
   *
   * \param[in] font The font; whether it is built in is not checked, see Text.
   * \return kEOk, or kENotOk if Text::IsFontValid() refuses `font`; the font is then left as it
   *         was.
   */
  StdReturnType SetFont(FontType font) {
    if (Text::IsFontValid(font)) {
      font_ = font;
      return kEOk;
    } else {
      return kENotOk;
    }
  }

  /*!
   * \brief Limits the overlay to `month`, or lifts the limit with 0.
   *
   * How the month, the day and the valid-in-days work together is described at Overlay.
   *
   * \param[in] month The month, 1 to 12, or 0 for none; it is not checked.
   */
  void SetMonth(MonthType month) { month_ = month; }

  /*!
   * \brief Limits the overlay to `day` of the month, or lifts the limit with 0.
   *
   * How the month, the day and the valid-in-days work together is described at Overlay.
   *
   * \param[in] day The day of the month, 1 to 31, or 0 for none; it is not checked.
   */
  void SetDay(DayType day) { day_ = day; }

  /*!
   * \brief Sets on how many days after its date the overlay goes on starting.
   *
   * It counts only while both a month and a day are set; otherwise the overlay keeps to them as
   * if it were 0. At 0 the overlay starts on the date only. The days count from the date on,
   * across the end of the year as well: with 24 December and 3, the overlay starts from 24 to
   * 27 December, and with 30 December and 5 up to 4 January.
   *
   * \param[in] valid_in_days The days after the date, 0 for the date only.
   */
  void SetValidInDays(DayType valid_in_days) { valid_in_days_ = valid_in_days; }

  /*!
   * \brief Sets the speed the text scrolls at.
   *
   * The overlay hands it to the text output at each start, as the task cycle
   * Scheduler::ConvertSpeedToTaskCycle() makes of it: at 1 the text moves one column every 255
   * scheduler ticks, at 255 every tick. At 0 the scheduler does not call Text::Task(), and the
   * text does not move. A new overlay has the speed 1.
   *
   * \param[in] speed The speed, 0 to 255.
   */
  void SetSpeed(byte speed) { speed_ = speed; }

  /*!
   * \brief Switches the overlay on through EnableIsActive() or off through DisableIsActive().
   *
   * \param[in] is_active True to switch the overlay on, false to switch it off.
   */
  void SetIsActive(bool is_active) {
    if (is_active) {
      EnableIsActive();
    } else {
      DisableIsActive();
    }
  }

  /*!
   * \brief Puts the overlay back to what a clock that was never configured has.
   *
   * The period is then 1 minute and the endurance 1 second, there is no month and no day, the
   * valid-in-days is 0, the speed 1, the font Text::FontType::kFont10x10, and the overlay is
   * switched off. Then the derived overlay resets its own settings: the text overlay empties its
   * text. The reset exists now that the overlays are stored and a reset has something to undo.
   *
   * \note The derived hook, `ResetOwnToDefaults()`, is named apart from this for the reason
   *       CanShow() gives: a `ResetToDefaults()` in an overlay would hide this one, and everything
   *       the base owns would then quietly survive the reset. An overlay that shows ends its show
   *       first, as DisableIsActive() ends it.
   */
  void ResetToDefaults() {
    period_in_minutes_ = kPeriodInMinutesInitValue;
    endurance_in_seconds_ = kEnduranceInSecondsInitValue;
    month_ = kMonthInitValue;
    day_ = kDayInitValue;
    valid_in_days_ = kValidInDaysInitValue;
    speed_ = kSpeedInitValue;
    font_ = kFontInitValue;
    EndShowIfShowing();
    state_ = StateType::kDisabled;
    Underlying().ResetOwnToDefaults();
  }

  /*!
   * \brief Switches the overlay on: one that is switched off goes to StateType::kIdle, one that is
   *        on already stays as it is.
   */
  void EnableIsActive() {
    if (state_ == StateType::kDisabled) {
      state_ = StateType::kIdle;
    }
  }

  /*!
   * \brief Switches the overlay off, StateType::kDisabled, from any state.
   *
   * An overlay that shows ends its show first, as StopShow() ends it: its text stops.
   */
  void DisableIsActive() {
    EndShowIfShowing();
    state_ = StateType::kDisabled;
  }

  /*!
   * \brief Takes the overlay one second further: starts it when its time has come, and ends it when
   *        its show timer has run out.
   *
   * Overlays calls it once a second, with the show timer it keeps for all overlays, and keeps
   * what comes back as the timer's new value.
   *
   * Switched off, the overlay does nothing. Idle, it starts if CanShow() is true, the time is
   * second 30 of a minute that the period divides, and the date passes the month, the day and
   * the valid-in-days: as StartShow() does, it goes to StateType::kShow and the derived overlay
   * starts its text. Showing, it first starts the text again if the text output is idle, which it
   * is once the previous run has scrolled through, and then ends once the timer is 0: as
   * StopShow() does, it goes back to StateType::kIdle and the derived overlay stops its text.
   *
   * It draws nothing itself and does not show the display; Text::Task() does that, called by the
   * scheduler.
   *
   * \param[in] show_timer_in_seconds The show timer, in seconds.
   * \param[in] current_date The date now, which the month, the day and the valid-in-days are
   *                         checked against.
   * \param[in] current_time The time now, whose minute and second decide a start.
   * \return The new show timer, in seconds: the endurance at a start; while the overlay shows,
   *         `show_timer_in_seconds` one lower, but not below 0; while it is idle and does not
   *         start, `show_timer_in_seconds` unchanged; and 0 while it is switched off. The overlay
   *         ends on the call that counts the timer down to 0, so that an overlay called once a
   *         second shows for exactly its endurance.
   */
  SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date,
                  ClockTime current_time) {
    if (state_ == StateType::kIdle) {
      return IdleTask(show_timer_in_seconds, current_date, current_time);
    }
    if (state_ == StateType::kShow) {
      RestartTextOnceScrolledThrough();
      return ShowTask(show_timer_in_seconds, current_date, current_time);
    }
    return 0u;
  }
};

#endif  // WORDCLOCK_SRC_OVERLAY_OVERLAY_H_
