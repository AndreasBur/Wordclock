// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The time the core reads, kept by the clock chip.
 *
 * Simpler than the ESP32's counterpart, because there is nothing to reconcile: that backend has a
 * network time and a chip and has to decide which of the two is right. Here the chip is the only
 * source there is, so this reads it and hands on what it said.
 *
 * A command that sets the time writes through to the chip and keeps the value at once, rather than
 * waiting for the next read to bring it back. Otherwise the answer to the command would still show
 * the old time.
 */

#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"
#include "DS3231.h"

/*!
 * \brief The calls of RealTimeClock::Task() that pass without reading the chip between two that
 *        read it.
 *
 * The application calls Task() once per tick, so the chip is read about once a second. A read is
 * two register reads on a bus the light sensor shares, and the display only ever shows minutes -
 * asking a hundred times a second would buy nothing.
 */
#define REAL_TIME_CLOCK_CHIP_READ_INTERVAL              100u

/*!
 * \brief The clock: the date and time the clock shows, kept up to date by Task().
 *
 * The core reads the time and the date wherever it shows or compares them, and sets them from the
 * time and the date commands. The application calls Task() on every tick, outside the scheduler.
 *
 * On the AVR the DS3231 is the time source. Task() reads it on its first call, and from then on
 * once in every REAL_TIME_CLOCK_CHIP_READ_INTERVAL + 1 calls; the setters write through to it.
 * Nothing counts the time here: between two reads the date and time stand still.
 *
 * \note Without a chip that answers, the clock does not run: the date and time stay at what they
 *       were made with, or at what a command last set them to.
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
    static constexpr byte kChipReadInterval{REAL_TIME_CLOCK_CHIP_READ_INTERVAL};

    ClockDateTime date_time_;
    /*
     * An instance of its own, next to the one the core's temperature side keeps. The chip
     * has no state that two readers could disturb, and both reach it through the same bus
     * driver.
     */
    DS3231 chip_;
    // Zero, so the first tick asks the chip straight away rather than showing the default
    // date for a second.
    byte chip_read_countdown_{0u};

    RealTimeClock() {}
    ~RealTimeClock() = default;

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
     * \brief Sets the date and time.
     *
     * It keeps them at once and writes them to the chip, ignoring whether the chip took them.
     * Task() then reads the chip again only after REAL_TIME_CLOCK_CHIP_READ_INTERVAL calls that do
     * not.
     *
     * \param[in] date_time The date and time.
     */
    void SetDateTime(ClockDateTime date_time);

    /*!
     * \brief Sets the time of day, and keeps the date.
     *
     * It keeps the time at once and writes the date and time to the chip, ignoring whether the
     * chip took them. Task() then reads the chip again only after
     * REAL_TIME_CLOCK_CHIP_READ_INTERVAL calls that do not.
     *
     * \param[in] time The time of day.
     */
    void SetTime(ClockTime time);

    /*!
     * \brief Sets the date, and keeps the time of day.
     *
     * It keeps the date at once and writes the date and time to the chip, ignoring whether the
     * chip took them. Task() then reads the chip again only after
     * REAL_TIME_CLOCK_CHIP_READ_INTERVAL calls that do not.
     *
     * \param[in] date The date.
     */
    void SetDate(ClockDate date);

    /*!
     * \brief Brings the date and time up to date.
     *
     * It reads the chip on its first call and then once in every
     * REAL_TIME_CLOCK_CHIP_READ_INTERVAL + 1 calls, and does nothing else. A read the chip refuses
     * leaves the date and time as they were.
     *
     * Called from the application's tick rather than through the scheduler, because the core
     * reads the time within its own tasks and would otherwise see it change part way through a
     * pass.
     */
    void Task();
};

#endif // _REAL_TIME_CLOCK_H_
