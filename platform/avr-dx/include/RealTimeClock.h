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

/* RealTimeClock configuration parameter */
/*
 * Application ticks between two reads of the chip, so once a second. A read is two
 * transfers on a bus the light sensor shares, and the display only ever shows minutes -
 * asking a hundred times a second would buy nothing.
 */
#define REAL_TIME_CLOCK_CHIP_READ_INTERVAL              100u

class RealTimeClock
{
  public:
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

    using YearType = ClockDateTime::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
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

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    StdReturnType ReadChip();
    void WriteChip();

  public:
    static RealTimeClock& GetInstance() {
        static RealTimeClock singleton_instance;
        return singleton_instance;
    }

    // get methods
    ClockDateTime GetDateTime() const { return date_time_; }
    ClockTime GetTime() const { return date_time_.GetTime(); }
    ClockDate GetDate() const { return date_time_.GetDate(); }

    // set methods
    void SetDateTime(ClockDateTime date_time);
    void SetTime(ClockTime time);
    void SetDate(ClockDate date);

    // methods
    void Task();
};

#endif // _REAL_TIME_CLOCK_H_
