// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief RP2350 real-time clock, backed by the system clock.
 *
 * Mirrors the public API of the simulator's real-time clock, so the shared firmware core reads the
 * time the same way on both.
 *
 * The RP2350's system clock *is* the clock, kept right by SNTP in the background and converted to
 * local time by the zone rule the application installs - which is also what makes daylight saving
 * somebody else's problem. Task() therefore reads rather than counts, and no seconds are counted
 * here.
 *
 * The system clock does not survive a power cut: it starts at the epoch. The DS3231 behind this
 * does, and fills in until the first SNTP answer arrives, if it holds a time. IsSystemTimeSet() is
 * what tells the two states of the system clock apart.
 */

#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"
#include "DS3231.h"

/*!
 * \brief The clock: the date and time the clock shows, kept up to date by Task().
 *
 * The core reads the time and the date wherever it shows or compares them, and sets them from the
 * time and the date commands. The application calls Task() on every tick, outside the scheduler.
 *
 * On the RP2350 the system clock is the source, which SNTP sets once the application has applied
 * the zone rule WORDCLOCK_TIMEZONE at start and started the time servers once the link is up; see
 * TimeSync.h. The date and time here are local time, by that rule. The DS3231 fills in while the
 * system clock holds no valid year, and is written from the system clock once an hour and after
 * every setting. The chip keeps UTC.
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
    // No hour, so the first pass with a system time writes the chip. A byte the hours
    // cannot reach, rather than a second flag beside it.
    static constexpr byte kNoHourWritten{0xFFu};

    /*
     * Ticks between two attempts to read the chip, so once a second. This runs from the
     * application's tick rather than through the scheduler, and a read is two transfers -
     * trying on every tick would put a hundred of them a second on a bus the light sensor
     * shares, for a value that only has to arrive before the display is worth looking at.
     */
    static constexpr byte kChipReadInterval{100u};

    ClockDateTime date_time_;

    /*
     * An instance of its own, next to the one Temperature keeps. The chip has no state
     * that two readers could disturb, both open the same bus - which Wire.begin() allows
     * twice - and sharing one would mean either a singleton the simulator's stand-in would
     * have to grow too, or the time-keeping side reaching through the core's Temperature,
     * which knows nothing about time.
     */
    DS3231 chip_;
    byte hour_written_to_chip_{kNoHourWritten};
    /* Zero, so the first tick without a system time asks the chip straight away. */
    byte chip_read_countdown_{0u};

    RealTimeClock() {}
    ~RealTimeClock() = default;

    static bool ReadSystemDateTime(ClockDateTime& value);
    static StdReturnType WriteSystemDateTime(const ClockDateTime& value);

    StdReturnType ReadChip();
    void WriteChip();

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
     * \brief Returns whether the system clock holds a date yet: one whose year ClockDate accepts.
     *
     * The system clock is set by SNTP, by a setter here, or by Task() from the chip. Until it is,
     * Task() leaves the date and time as they were rather than handing the display the epoch,
     * which ClockDate would reject anyway.
     *
     * Asked as "is the year one ClockDate accepts" rather than by querying SNTP: that answers the
     * question the firmware actually has, and it stays true for a time that arrived from a command
     * or from the chip instead of the net.
     *
     * \return True once the system clock holds such a date.
     */
    static bool IsSystemTimeSet();

    /*!
     * \brief Sets the date and time.
     *
     * On the RP2350 it keeps them at once and writes them to the system clock as local time,
     * through the zone rule, ignoring whether the system clock took them; the next Task() that
     * finds the system clock set writes the chip as well. Written through rather than just
     * stored: Task() reads the system clock back on the very next tick, so a value kept only here
     * would be overwritten within 10 ms and the time command would look like it did nothing.
     *
     * \param[in] date_time The date and time.
     */
    void SetDateTime(ClockDateTime date_time);

    /*!
     * \brief Sets the time of day, and keeps the date.
     *
     * On the RP2350 it writes the date it holds and the new time to the system clock, as
     * SetDateTime() does, and has the next Task() write the chip.
     *
     * \param[in] time The time of day.
     */
    void SetTime(ClockTime time);

    /*!
     * \brief Sets the date, and keeps the time of day.
     *
     * On the RP2350 it writes the new date and the time it holds to the system clock, as
     * SetDateTime() does, and has the next Task() write the chip.
     *
     * \param[in] date The date.
     */
    void SetDate(ClockDate date);

    /*!
     * \brief Brings the date and time up to date.
     *
     * On the RP2350 it takes them from the system clock, as local time, while the system clock
     * holds a valid year, and then writes the chip in UTC when the hour differs from the one it
     * last wrote, or after a setting. Nothing is counted forward here, unlike on the simulator:
     * SNTP keeps the system clock right on its own, so counting would only add a second source of
     * truth that drifts away from it.
     *
     * While the system clock holds no valid year it leaves the date and time as they were and
     * tries the chip instead, on its first such call and then about once a second. A chip that
     * answers sets the system clock, which the next call then reads. The clock chip is the source
     * of last resort rather than a second one: it is read only while the system clock holds
     * nothing. Which way round matters - the chip drifts a few seconds a month and SNTP does not,
     * so a chip that could overwrite a synchronised clock would make the clock worse.
     */
    void Task();
};

#endif // _REAL_TIME_CLOCK_H_
