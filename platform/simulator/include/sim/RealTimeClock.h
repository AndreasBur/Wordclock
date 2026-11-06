// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator stub of the real-time clock driver.
 *
 * Mirrors the public API of the hardware RTC driver (date/time getters and setters, singleton
 * access) so the shared firmware core builds against the simulator. Time advances from a monotonic
 * host clock after it has been initialized, just like an RTC would.
 */

#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"
#include <chrono>
#include <wx/datetime.h>

/*!
 * \brief The clock: the date and time the clock shows, kept up to date by Task().
 *
 * The core reads the time and the date wherever it shows or compares them, and sets them from the
 * time and the date commands. The application calls Task() on every tick, outside the scheduler.
 *
 * The simulator has no clock chip and no network. The application sets the date and time from the
 * host's clock once, when it starts; from there Task() counts them forward off the host's
 * monotonic clock, so a time set over the serial console keeps running instead of being
 * overwritten.
 */
class RealTimeClock
{
  public:
    /*! \copydoc ClockDateTime::HourType */
    using HourType = ClockDateTime::HourType;
    /*! \copydoc ClockDateTime::MinuteType */
    using MinuteType = ClockDateTime::MinuteType;
    /*! \copydoc ClockDateTime::SecondType */
    using SecondType = ClockDateTime::SecondType;

    /*! \copydoc ClockDateTime::YearType */
    using YearType = ClockDateTime::YearType;
    /*! \copydoc ClockDateTime::MonthType */
    using MonthType = ClockDateTime::MonthType;
    /*! \copydoc ClockDateTime::DayType */
    using DayType = ClockDateTime::DayType;
    /*! \copydoc ClockDateTime::WeekdayType */
    using WeekdayType = ClockDateTime::WeekdayType;

  private:
    ClockDateTime date_time_;
    std::chrono::steady_clock::time_point last_update_{std::chrono::steady_clock::now()};

    RealTimeClock() {}
    ~RealTimeClock() = default;

    void UpdateDateTime(const wxDateTime& value) {
        date_time_.SetDateYear(value.GetYear());
        date_time_.SetDateMonth(value.GetMonth() + 1u);
        date_time_.SetDateDay(value.GetDay());
        date_time_.SetTimeHour(value.GetHour());
        date_time_.SetTimeMinute(value.GetMinute());
        date_time_.SetTimeSecond(value.GetSecond());
    }

  public:
    /*! \brief Returns the one clock. */
    static RealTimeClock& GetInstance() {
        static RealTimeClock singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the date and time. */
    ClockDateTime GetDateTime() const { return date_time_; }

    /*! \brief Returns the time of day. */
    ClockTime GetTime() const { return date_time_.GetTime(); }

    /*! \brief Returns the date. */
    ClockDate GetDate() const { return date_time_.GetDate(); }

    /*!
     * \brief Sets the date and time.
     *
     * In the simulator, Task() counts on from here: the seconds it adds are counted from this
     * call.
     *
     * \param[in] date_time The date and time.
     */
    void SetDateTime(ClockDateTime date_time) {
        date_time_ = date_time;
        last_update_ = std::chrono::steady_clock::now();
    }

    /*!
     * \brief Sets the time of day, and keeps the date.
     *
     * In the simulator, Task() counts on from here: the seconds it adds are counted from this
     * call.
     *
     * \param[in] time The time of day.
     */
    void SetTime(ClockTime time) {
        date_time_.SetTime(time);
        last_update_ = std::chrono::steady_clock::now();
    }

    /*!
     * \brief Sets the date, and keeps the time of day.
     *
     * In the simulator, Task() counts on from here: the seconds it adds are counted from this
     * call.
     *
     * \param[in] date The date.
     */
    void SetDate(ClockDate date) {
        date_time_.SetDate(date);
        last_update_ = std::chrono::steady_clock::now();
    }

    /*!
     * \brief Brings the date and time up to date.
     *
     * In the simulator, it adds the whole seconds the host's monotonic clock has counted since the
     * last time it added any, or since the date or time was last set, and carries the date over
     * with them. The part of a second left over counts towards the next call.
     */
    void Task() {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_);
        if(elapsed_seconds.count() == 0) { return; }

        wxDateTime current(date_time_.GetDateDay(),
                             static_cast<wxDateTime::Month>(date_time_.GetDateMonth() - 1u),
                             date_time_.GetDateYear(),
                             date_time_.GetTimeHour(),
                             date_time_.GetTimeMinute(),
                             date_time_.GetTimeSecond());
        current += wxTimeSpan::Seconds(elapsed_seconds.count());
        UpdateDateTime(current);
        last_update_ += elapsed_seconds;
    }
};

#endif
