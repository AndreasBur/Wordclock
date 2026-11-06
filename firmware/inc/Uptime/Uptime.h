// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How long the clock has been running, counted rather than measured.
 *
 * This used to be a platform's job: each backend divided its millisecond counter down and the
 * status command reported a uint16 of minutes. Both halves of that wrapped. The field ran out after
 * 45 days 12 hours, and underneath it the 32-bit millisecond counters on the ESP32 and the AVR come
 * round after 49 days 17 hours - so a clock that had stood for seven weeks reported nine minutes,
 * which is the sort of wrong answer a reader believes.
 *
 * Counting instead of asking is what fixes it. The source is the scheduler's tick, which does not
 * wrap, and what is kept is the answer itself rather than a number to divide: seconds into minutes
 * into hours into days. Days are a uint16, so the count reaches 179 years, and it saturates rather
 * than rolling over even there.
 *
 * In the core and not in a platform, because none of it is hardware - which also takes
 * getUptimeInMinutes() out of the platform contract that three backends each had to implement and
 * each got wrong the same way.
 */

#ifndef _UPTIME_H_
#define _UPTIME_H_

#include "Arduino.h"

/*!
 * \brief The cycle of the uptime's task, in scheduler ticks, so 100 * 10 ms = 1 s: Task() counts
 *        one second per call.
 *
 * The scheduler counts cycles in a byte, which caps this at 255 - a second is the coarsest unit
 * that still carries a minute exactly.
 */
#define UPTIME_TASK_CYCLE                               100u

/*!
 * \brief How long the clock has been running: days, hours and minutes, counted from the scheduler's
 *        tick.
 *
 * The days stop at their maximum rather than rolling over, so it never reports a small number for
 * a long time.
 */
class Uptime
{
  private:
    static constexpr byte kTaskCycle{UPTIME_TASK_CYCLE};

    static constexpr byte kSecondsPerMinute{60u};
    static constexpr byte kMinutesPerHour{60u};
    static constexpr byte kHoursPerDay{24u};

    /*
     * 179 years, which no clock reaches - but it holds there rather than rolling over,
     * because the whole point of this class is that it never reports a small number for a
     * long time.
     */
    static constexpr uint16_t kDaysMaximum{65535u};

    byte seconds_{0u};
    byte minutes_{0u};
    byte hours_{0u};
    uint16_t days_{0u};

    Uptime() { }
    ~Uptime() { }

    static bool Carry(byte& value, byte limit);

  public:
    /*! \brief Returns the one uptime of the firmware. */
    static Uptime& GetInstance() {
        static Uptime singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the scheduler ticks between two calls of Task(), UPTIME_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*! \brief Returns the whole days counted so far, up to 65535. */
    uint16_t GetDays() const { return days_; }

    /*! \brief Returns the whole hours counted so far beyond the days, 0 to 23. */
    byte GetHours() const { return hours_; }

    /*! \brief Returns the whole minutes counted so far beyond the hours, 0 to 59. */
    byte GetMinutes() const { return minutes_; }

    /*!
     * \brief Counts one second.
     *
     * The scheduler calls it every UPTIME_TASK_CYCLE ticks. Seconds carry into minutes, minutes
     * into hours and hours into days; the days stop at 65535 rather than rolling over.
     */
    void Task();
};

#endif // _UPTIME_H_
