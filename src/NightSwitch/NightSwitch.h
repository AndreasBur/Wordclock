// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Takes the display down for the night and gives it back in the morning.
 *
 * The clock lit the wall around the clock. This is the first setting that is a time of day rather
 * than a value, and the first that acts on its own rather than when something asks.
 *
 * What it does at night is one number: the night brightness, where zero means the display goes off
 * altogether and anything else means it is dimmed to that. One field rather than two settings,
 * because "off" and "very dim" are the same wish answered at different strengths.
 *
 * Dimming goes through the display's night level, which scales what the setting, the gamma
 * correction and the automatic arrived at. It does not write the brightness setting, which matters
 * twice: morning returns to the brightness its owner chose rather than to one this module
 * remembered, and Persistence keeps storing that same chosen value instead of whatever the clock
 * happened to be showing at the moment it wrote.
 *
 * A night brightness of zero also takes the strip's supply away where there is a switch to do it
 * with. That is the state the switch was built for: a display that is off still draws its quiescent
 * current, some 110 mA for this strip, and a night is the longest stretch in which nobody is
 * looking at what it buys. Power is asked rather than the port written, because the cut is a
 * sequence over several ticks and not a write.
 *
 * It acts on the crossing and not on the state. A clock switched on by hand at two in the morning
 * stays on until the window's next edge, rather than going dark again a second later - which looks
 * like a fault, and is the behaviour that makes people stop using a timer.
 *
 * The supply is the one exception, and it is one because a cut rail turns "on" into a switch that
 * does nothing: the display would be enabled, the data line gated and the wall still dark. So a
 * hand that switches the clock on inside the window gets the supply back, which is the crossing
 * rule kept rather than broken - the display stays where the hand put it.
 */

#ifndef WORDCLOCK_SRC_NIGHTSWITCH_NIGHTSWITCH_H_
#define WORDCLOCK_SRC_NIGHTSWITCH_NIGHTSWITCH_H_

#include "Arduino.h"
#include "ClockTime.h"

/*!
 * \brief Scheduler ticks between two calls of NightSwitch::Task(), so 100 * 10 ms = 1 s.
 *
 * A window's edge is a minute wide, so this is far finer than it needs to be - and still cheap,
 * because a tick that is not an edge costs one comparison.
 */
#define NIGHT_SWITCH_TASK_CYCLE 100u

/*!
 * \brief The night switch: dims the display or takes it down inside a window of the day, and gives
 *        it back at the window's end.
 *
 * The window runs from its start to its end, in hours and minutes of the real-time clock, and
 * may cross midnight. Task() acts on the window's edges only. The setters store what they are
 * given without a check.
 */
class NightSwitch {
 public:
  /*! \brief An hour of the day, as ClockTime counts it. */
  using HourType = ClockTime::HourType;

  /*! \brief A minute of the hour, as ClockTime counts it. */
  using MinuteType = ClockTime::MinuteType;

  /*!
   * \brief A time of day in minutes since midnight.
   *
   * What the window is compared in: two numbers rather than four, and the wrap around midnight
   * becomes one comparison instead of a case.
   */
  using TimeOfDayType = uint16_t;

 private:
  static constexpr byte kTaskCycle{NIGHT_SWITCH_TASK_CYCLE};
  static constexpr TimeOfDayType kMinutesPerHour{60u};
  static constexpr byte kNightBrightnessOff{0u};
  // What the display's night level is when nothing is dimming it.
  static constexpr byte kNightLevelFull{255u};

  static constexpr bool kIsActiveInitValue{false};
  static constexpr HourType kStartHourInitValue{23u};
  static constexpr MinuteType kStartMinuteInitValue{0u};
  static constexpr HourType kEndHourInitValue{6u};
  static constexpr MinuteType kEndMinuteInitValue{0u};
  static constexpr byte kNightBrightnessInitValue{kNightBrightnessOff};

  bool is_active_{kIsActiveInitValue};
  HourType start_hour_{kStartHourInitValue};
  MinuteType start_minute_{kStartMinuteInitValue};
  HourType end_hour_{kEndHourInitValue};
  MinuteType end_minute_{kEndMinuteInitValue};
  byte night_brightness_{kNightBrightnessInitValue};

  // What the last tick saw, so that only a change acts. Starts as day: a clock that comes
  // up inside the window gets its first tick as an edge and switches straight away.
  bool was_night_{false};

  /*
   * Whether it was the night that took the supply down. What it guards is the tick that
   * watches for a hand: without it a supply cut by hand at noon would be handed straight
   * back on the next tick, because the display is enabled the whole time. A window's edge
   * is another matter and gives it back either way - the same rule that already re-enables
   * a display somebody switched off.
   */
  bool supply_cut_for_night_{false};

  NightSwitch() {}
  ~NightSwitch() = default;

  static constexpr TimeOfDayType ToTimeOfDay(HourType hour, MinuteType minute) {
    return static_cast<TimeOfDayType>((hour * kMinutesPerHour) + minute);
  }

  // An empty window - one whose ends are the same minute - is no window at all rather
  // than a whole day, because that is what somebody who has not configured it yet has.
  static constexpr bool IsWindowEmpty(TimeOfDayType start, TimeOfDayType end) {
    return start == end;
  }
  static constexpr bool IsWindowCrossingMidnight(TimeOfDayType start, TimeOfDayType end) {
    return start > end;
  }

  static constexpr bool IsWithinWindow(TimeOfDayType now, TimeOfDayType start, TimeOfDayType end) {
    if (IsWindowEmpty(start, end)) {
      return false;
    }
    // The ordinary window is a span; the one that crosses midnight is everything
    // outside the span between its ends, which is the same test read the other way.
    if (IsWindowCrossingMidnight(start, end)) {
      return (now >= start) || (now < end);
    }

    return (now >= start) && (now < end);
  }

  static bool IsDisplayOffAtNight(byte brightness) { return brightness == kNightBrightnessOff; }

  void CutSupplyForTheNight();
  void RestoreSupplyAfterTheNight();
  void ApplyNight();
  void ApplyDay();
  void FollowTheDisplayBackOn();

 public:
  /*! \brief Returns the one night switch. */
  static NightSwitch& GetInstance() {
    static NightSwitch singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns the scheduler ticks between two calls of Task(), NIGHT_SWITCH_TASK_CYCLE. */
  static constexpr byte GetTaskCycle() { return kTaskCycle; }

  /*! \brief Returns whether the night switch acts on its window. */
  bool GetIsActive() const { return is_active_; }

  /*! \brief Returns the hour the night starts at. */
  HourType GetStartHour() const { return start_hour_; }

  /*! \brief Returns the minute the night starts at. */
  MinuteType GetStartMinute() const { return start_minute_; }

  /*! \brief Returns the hour the night ends at. */
  HourType GetEndHour() const { return end_hour_; }

  /*! \brief Returns the minute the night ends at. */
  MinuteType GetEndMinute() const { return end_minute_; }

  /*! \brief Returns the night brightness: 0 for off, anything else the level to dim to. */
  byte GetNightBrightness() const { return night_brightness_; }

  /*!
   * \brief Returns whether the clock considers it night now.
   *
   * This is what the answer reports - a window that is configured but switched off still has
   * an answer to this. The start minute is inside the window and the end minute is not. A
   * window whose start and end are the same minute is no window at all, so it is never night
   * then.
   *
   * \return True if the time of the real-time clock lies inside the window.
   */
  bool IsNight() const;

  /*!
   * \brief Sets the hour the night starts at.
   *
   * \param[in] start_hour The hour.
   */
  void SetStartHour(HourType start_hour) { start_hour_ = start_hour; }

  /*!
   * \brief Sets the minute the night starts at.
   *
   * \param[in] start_minute The minute.
   */
  void SetStartMinute(MinuteType start_minute) { start_minute_ = start_minute; }

  /*!
   * \brief Sets the hour the night ends at.
   *
   * \param[in] end_hour The hour.
   */
  void SetEndHour(HourType end_hour) { end_hour_ = end_hour; }

  /*!
   * \brief Sets the minute the night ends at.
   *
   * \param[in] end_minute The minute.
   */
  void SetEndMinute(MinuteType end_minute) { end_minute_ = end_minute; }

  /*!
   * \brief Sets what the night does to the display.
   *
   * It takes effect at the next start of the night.
   *
   * \param[in] night_brightness 0 to switch the display off, anything else to dim it to that level.
   */
  void SetNightBrightness(byte night_brightness) { night_brightness_ = night_brightness; }

  /*!
   * \brief Switches the night switch on or off.
   *
   * Switching the whole thing off gives the display back rather than leaving it wherever the
   * last edge put it - a clock left dark by a timer that is no longer running is a clock nobody
   * can explain. Switching it on inside the window takes the next Task() for the start of the
   * night.
   *
   * \param[in] is_active True to switch it on, false to switch it off.
   */
  void SetIsActive(bool is_active);

  /*!
   * \brief Puts the night switch back to how a new clock has it: off, with a night from 23:00 to
   *        6:00 that switches the display off.
   *
   * Switching off gives the display back, as SetIsActive() does.
   */
  void ResetToDefaults();

  /*!
   * \brief Acts on the window's edges.
   *
   * Does nothing while the night switch is off. At the start of the night a night brightness
   * of 0 has Power cut the strip's supply, or disables the display where no switch is fitted;
   * any other dims the display's night level to it. At the end of the night the night level is
   * cleared, the supply given back if the night cut it, and the display enabled.
   *
   * Acts on the crossing, not on the state: between two edges the display is whatever anybody
   * last made it, so switching it on by hand at night keeps it on - which is what the tick
   * between two edges is for, since a hand that switched it on has a supply to get back.
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_NIGHTSWITCH_NIGHTSWITCH_H_
