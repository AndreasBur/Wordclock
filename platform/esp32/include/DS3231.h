// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Temperature of the DS3231 real time clock chip.
 *
 * Two things the chip keeps for the clock: the time it counts on its battery, and the die
 * temperature it measures to compensate its crystal. The first fills the gap between power-on and
 * the first SNTP answer, the second is what the temperature overlay shows.
 *
 * What is stored in the time registers is UTC, not the local time the display shows. A clock that
 * stored local time would have nothing to say about the hour that occurs twice when summer time
 * ends, and would be an hour wrong from a switch until the next SNTP answer. RealTimeClock does the
 * conversion in both directions.
 *
 * A die temperature is not a room temperature. The chip sits in the case next to whatever else is
 * warm in there, so what it reads is a degree or two above the room and needs the offset the case
 * turns out to have.
 */

#ifndef _DS3231_H_
#define _DS3231_H_

#include "StandardTypes.h"
#include "Bit.h"
#include "Arduino.h"
#include "ClockDateTime.h"

/*!
 * \brief The bus address of the chip.
 *
 * The only address the chip has; A0 to A2 do not exist on it.
 */
#define DS3231_I2C_ADDR                                 0x68u

// The same pins and speed as the light sensor, which is the point of putting both on one
// bus. Wire.begin() is idempotent, so whichever driver runs first opens it.
/*! \brief The pin of the bus's data line, the board's `SDA`. */
#define DS3231_I2C_PIN_SDA                              SDA
/*! \brief The pin of the bus's clock line, the board's `SCL`. */
#define DS3231_I2C_PIN_SCL                              SCL
/*! \brief The clock of the bus, in Hz. */
#define DS3231_I2C_FREQUENCY_HZ                         100000u

/*!
 * \brief The scheduler ticks between two calls of DS3231::Task().
 *
 * In scheduler ticks, so 250 * 10 ms = 2.5 s between readings. The chip converts on its own every
 * 64 s, so this does not set how fresh the value is - it sets how long after a start it takes until
 * there is one at all, and how quickly a reappearing chip is picked up again. The scheduler counts
 * cycles in a byte, so this cannot be raised past 255.
 */
#define DS3231_TASK_CYCLE                               250u

// Temperature register pair: the whole degrees as a signed byte, and the quarters in the
// top two bits of the byte behind it.
/*! \brief The register of the whole degrees, the first of the temperature pair. */
#define DS3231_REG_TEMPERATURE_MSB                      0x11u
/*! \brief The number of registers of the temperature pair. */
#define DS3231_TEMPERATURE_NUMBER_OF_BYTES              2u
/*! \brief The mask of the quarters in the second register of the pair, written down unshifted. */
#define DS3231_TEMPERATURE_FRACTION_GM                  0b11u
/*! \brief The position of the quarters in the second register of the pair. */
#define DS3231_TEMPERATURE_FRACTION_GP                  6u

/* Time and date, seven registers in one block, every field in BCD. */
/*! \brief The register of the seconds, the first of the date and time block. */
#define DS3231_REG_SECONDS                              0x00u
/*! \brief The number of registers of the date and time block. */
#define DS3231_DATE_TIME_NUMBER_OF_BYTES                7u

/*
 * The bits beside the value in the registers that carry one: the hours register selects
 * 12-hour mode in bit 6 and holds the AM/PM or the tens bit in bit 5, and the month
 * register carries the century in bit 7. Both are masked off on the way in and left at
 * zero on the way out - the clock runs in 24-hour mode, and ClockDate covers 2000 to 2099,
 * which is exactly the century the chip counts in with that bit clear.
 */
/*! \brief The bits of the hours register that hold the hour. */
#define DS3231_HOURS_VALUE_MASK                         0b00111111u
/*! \brief The bits of the month register that hold the month. */
#define DS3231_MONTH_VALUE_MASK                         0b00011111u

// Status register, whose top bit says the oscillator stopped - which is how a chip that
// ran out of battery says its time is not to be believed.
/*! \brief The status register of the chip. */
#define DS3231_REG_STATUS                               0x0Fu
/*! \brief The bit of the status register that says the oscillator stopped. */
#define DS3231_STATUS_OSCILLATOR_STOPPED_MASK           0b10000000u

/*!
 * \brief The DS3231 clock chip, as far as the core reaches it: the temperature it measures, in
 *        tenths of a degree Celsius.
 *
 * The core owns one, inside Temperature, and reaches it through Task() and GetTemperature(). There
 * is no reading until the chip has answered once, which a clock built without the chip never
 * does.
 *
 * On the ESP32 the chip is reached over I2C through `Wire`, on DS3231_I2C_PIN_SDA and
 * DS3231_I2C_PIN_SCL at DS3231_I2C_FREQUENCY_HZ, the bus it shares with the BH1750. The same class
 * also reads and writes the time, with GetDateTime() and SetDateTime(), for RealTimeClock, which
 * keeps an instance of its own.
 */
class DS3231
{
  public:
    /*!
     * \brief A temperature in tenths of a degree Celsius, signed.
     *
     * Tenths rather than the chip's own quarters because that is what a display shows, and signed
     * because the chip reads below zero and the overlay has to say so.
     */
    using TemperatureType = int16_t;

    /*! \brief The tenths in a degree, the unit of a TemperatureType. */
    static constexpr TemperatureType kTenthsPerDegree{10};

  private:
    static constexpr byte kTaskCycle{DS3231_TASK_CYCLE};
    static constexpr TemperatureType kQuartersPerDegree{4};

    TemperatureType temperature_{0};
    // Until the chip has answered once there is nothing to show, and the overlay stays
    // away rather than showing a zero that looks like a reading.
    bool temperature_valid_{false};
    bool bus_started_{false};

    StdReturnType StartBus();
    StdReturnType ReadTemperature();

    static StdReturnType ReadRegisters(byte register_address, byte* values, byte length);
    static StdReturnType WriteRegisters(byte register_address, const byte* values, byte length);
    static StdReturnType ClearOscillatorStopped();
    static bool HasOscillatorStopped();

  public:
    /*! \brief Makes a chip that has no reading yet. */
    constexpr DS3231() { }

    /*! \brief Destroys the chip. */
    ~DS3231() = default;

    /*!
     * \brief Returns the temperature the chip's register pair holds, in tenths of a degree Celsius.
     *
     * The quarters sit in the top two bits, and the whole degrees are signed - so the two halves
     * are combined in quarters, where the fraction is a plain addition even below zero, and only
     * then scaled. Truncates towards zero, which loses at most a tenth.
     *
     * Public because it is the one part of this driver that can be checked without a bus, and the
     * assertions below do exactly that.
     *
     * \param[in] degrees The whole degrees, the register DS3231_REG_TEMPERATURE_MSB as a signed
     *                    byte.
     * \param[in] fraction_register The register behind it, which holds the quarters in its top two
     *                              bits.
     * \return The temperature, in tenths of a degree Celsius.
     */
    static constexpr TemperatureType ToTenths(int8_t degrees, byte fraction_register) {
        const TemperatureType quarters = static_cast<TemperatureType>(degrees * kQuartersPerDegree) +
                                         Bit<byte>::ReadBitGroup(fraction_register, DS3231_TEMPERATURE_FRACTION_GM, DS3231_TEMPERATURE_FRACTION_GP);
        return static_cast<TemperatureType>((quarters * kTenthsPerDegree) / kQuartersPerDegree);
    }

    /*!
     * \brief Returns the number a register holds in BCD, two decimal digits per byte.
     *
     * The registers hold their fields in BCD. Public for the same reason as ToTenths(): it is
     * checkable without a bus, and the assertions below do it. It does not check that each digit
     * is below ten.
     *
     * \param[in] value The register, in BCD.
     * \return The number, the high digit times ten plus the low digit.
     */
    static constexpr byte FromBcd(byte value) { return static_cast<byte>(((value >> 4u) * 10u) + (value & 0x0Fu)); }

    /*!
     * \brief Returns `value` in BCD, two decimal digits per byte, as the registers hold it.
     *
     * Public for the same reason as ToTenths(): it is checkable without a bus, and the assertions
     * below do it.
     *
     * \param[in] value The number, from 0 to 99.
     * \return The number in BCD: its tens in the high four bits, its ones in the low four.
     */
    static constexpr byte ToBcd(byte value) { return static_cast<byte>(((value / 10u) << 4u) | (value % 10u)); }

    /*! \brief Returns the scheduler ticks between two calls of Task(), DS3231_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*!
     * \brief Gives the temperature last read.
     *
     * kENotOk while no reading has arrived yet, which is what a caller has to tell apart from a
     * reading of zero degrees. Once there is a reading it stays, through any later failed read.
     *
     * \param[out] temperature The temperature, in tenths of a degree Celsius.
     * \return kEOk, or kENotOk while there is no reading; `temperature` is then left as it was.
     */
    StdReturnType GetTemperature(TemperatureType& temperature) const {
        if(!temperature_valid_) { return kENotOk; }

        temperature = temperature_;
        return kEOk;
    }

    /*!
     * \brief Reads the date and time the chip counted, in UTC.
     *
     * RealTimeClock writes the chip in UTC; this class converts nothing, and hands back what the
     * registers hold. It opens the bus first, unless that has been done.
     *
     * The stopped flag is asked first and not afterwards: a chip whose oscillator stood still has
     * registers that still read like a date, and taking that date would set the clock to whenever
     * the battery gave out.
     *
     * What the setters reject is what decides whether the block is a date at all - a fresh chip
     * reads 0x00 everywhere, which is month zero and day zero, and a transfer that lost its way
     * reads anything. Hence the value is assembled through ClockDateTime's own setters and their
     * return codes rather than trusted. The weekday register is ignored.
     *
     * \param[out] date_time The date and time, in UTC.
     * \return kEOk, or kENotOk if the bus did not open, the chip does not answer, it reports that
     *         its oscillator stopped (a battery that ran out, so what it counted since is nothing)
     *         or the registers do not form a date; `date_time` is then left as it was.
     */
    StdReturnType GetDateTime(ClockDateTime& date_time);

    /*!
     * \brief Writes `date_time` to the chip, in UTC, and clears its stopped flag with it.
     *
     * The value just written is by definition believable again, hence the flag. The time is
     * written as it is given, with no conversion between zones: RealTimeClock hands it UTC. It
     * opens the bus first, unless that has been done.
     *
     * The seven registers go out in one transfer, the hours in 24-hour mode and the century bit
     * clear. The seconds register goes out with the rest rather than being written last: writing
     * it is what restarts the chip's own second, so the value lands as a whole.
     *
     * The weekday register is filled although nothing here reads it back - the date carries the
     * weekday anyway, and a register left at zero is one that another reader of this chip would be
     * misled by.
     *
     * \param[in] date_time The date and time, in UTC.
     * \return kEOk, or kENotOk if the bus did not open, the chip did not take the date and time
     *         whole or its stopped flag was not cleared; the chip may then hold the date and time
     *         or part of it, and its stopped flag is left as it was.
     */
    StdReturnType SetDateTime(const ClockDateTime& date_time);

    /*!
     * \brief Brings the temperature up to date.
     *
     * It opens the bus if that has not been done, and reads the temperature registers. A run on
     * which the bus does not open reads nothing, and a failed read keeps the reading there was.
     *
     * Set up from here rather than from an Init() call, the same way the light sensor is: there is
     * no place in the core to call an Init() from, and retrying from the task means a chip that is
     * plugged in later, or a bus that was busy at boot, still comes up.
     */
    void Task();
};

/* The register pair, read off the datasheet's own examples. */
static_assert(DS3231::kTenthsPerDegree == 10, "the conversion below is written in tenths");

/*
 * The datasheet's own two examples, which are the pair that matters: one above zero, and
 * one below where the fraction counts upwards from a whole degree more negative than the
 * result. Outside the class because a member function cannot be called in a constant
 * expression until the class is complete.
 */
static_assert(DS3231::ToTenths(0x19, 0x40u) == 252, "+25.25 degrees must read as 25.2");
static_assert(DS3231::ToTenths(static_cast<int8_t>(0xFE), 0x80u) == -15, "-1.5 degrees must read as -1.5");
static_assert(DS3231::ToTenths(0, 0x00u) == 0, "zero must stay zero");

/* Both ends of what the registers carry, and the round trip in between. */
static_assert(DS3231::FromBcd(0x59u) == 59u, "the tens digit sits in the upper nibble");
static_assert(DS3231::ToBcd(59u) == 0x59u, "and goes back there");
static_assert(DS3231::FromBcd(DS3231::ToBcd(0u)) == 0u, "zero must round-trip");
static_assert(DS3231::FromBcd(DS3231::ToBcd(99u)) == 99u, "the largest two-digit value must round-trip");

#endif // _DS3231_H_
