// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Temperature of the DS3231 real time clock chip.
 */

/* Ahead of DS3231.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Wire.h>

#include "DS3231.h"
#include "I2cBus.h"

namespace {
/* Wire reports zero from endTransmission() when the slave acknowledged. */
constexpr uint8_t kWireTransmissionSuccess{0u};

/* Where each field sits in the seven-register block that is read and written in one go. */
constexpr byte kRegisterSeconds{0u};
constexpr byte kRegisterMinutes{1u};
constexpr byte kRegisterHours{2u};
constexpr byte kRegisterWeekday{3u};
constexpr byte kRegisterDay{4u};
constexpr byte kRegisterMonth{5u};
constexpr byte kRegisterYear{6u};

// The chip keeps two year digits and a century bit; ClockDate covers 2000 to 2099, so the
// bit is always clear and the century is this constant.
constexpr ClockDateTime::YearType kYearOfCentury{2000u};

// The chip counts weekdays from one, ClockDate from zero. Neither says which day is
// first, and nothing reads the register back, so only the range has to match.
constexpr byte kWeekdayOffset{1u};

} // namespace

void DS3231::Task()
{
    if(StartBus() == kENotOk) { return; }

    ReadTemperature();
}

StdReturnType DS3231::GetDateTime(ClockDateTime& date_time)
{
    if(StartBus() == kENotOk) { return kENotOk; }
    if(HasOscillatorStopped()) { return kENotOk; }

    byte registers[DS3231_DATE_TIME_NUMBER_OF_BYTES]{};
    if(ReadRegisters(DS3231_REG_SECONDS, registers, DS3231_DATE_TIME_NUMBER_OF_BYTES) == kENotOk) { return kENotOk; }

    ClockDateTime value;
    StdReturnType return_value{kEOk};

    if(value.SetTimeSecond(FromBcd(registers[kRegisterSeconds])) == kENotOk) { return_value = kENotOk; }
    if(value.SetTimeMinute(FromBcd(registers[kRegisterMinutes])) == kENotOk) { return_value = kENotOk; }
    if(value.SetTimeHour(FromBcd(registers[kRegisterHours] & DS3231_HOURS_VALUE_MASK)) == kENotOk) { return_value = kENotOk; }
    if(value.SetDateDay(FromBcd(registers[kRegisterDay])) == kENotOk) { return_value = kENotOk; }
    if(value.SetDateMonth(FromBcd(registers[kRegisterMonth] & DS3231_MONTH_VALUE_MASK)) == kENotOk) { return_value = kENotOk; }
    if(value.SetDateYear(static_cast<ClockDateTime::YearType>(kYearOfCentury + FromBcd(registers[kRegisterYear]))) == kENotOk) { return_value = kENotOk; }

    if(return_value == kENotOk) { return kENotOk; }

    date_time = value;
    return kEOk;
}

StdReturnType DS3231::SetDateTime(const ClockDateTime& date_time)
{
    if(StartBus() == kENotOk) { return kENotOk; }

    byte registers[DS3231_DATE_TIME_NUMBER_OF_BYTES]{};

    registers[kRegisterSeconds] = ToBcd(date_time.GetTimeSecond());
    registers[kRegisterMinutes] = ToBcd(date_time.GetTimeMinute());
    registers[kRegisterHours] = ToBcd(date_time.GetTimeHour());
    registers[kRegisterWeekday] = ToBcd(static_cast<byte>(ToUnderlying(date_time.GetDateWeekday()) + kWeekdayOffset));
    registers[kRegisterDay] = ToBcd(date_time.GetDateDay());
    registers[kRegisterMonth] = ToBcd(date_time.GetDateMonth());
    registers[kRegisterYear] = ToBcd(static_cast<byte>(date_time.GetDateYear() - kYearOfCentury));

    if(WriteRegisters(DS3231_REG_SECONDS, registers, DS3231_DATE_TIME_NUMBER_OF_BYTES) == kENotOk) { return kENotOk; }

    return ClearOscillatorStopped();
}

/*
 * Opens the I2C bus, once; kEOk if the bus is available. Shared with the light sensor, and
 * whichever of the two runs first opens it.
 */
StdReturnType DS3231::StartBus()
{
    if(bus_started_) { return kEOk; }

    if(I2cBus::Open(DS3231_I2C_PIN_SDA, DS3231_I2C_PIN_SCL, DS3231_I2C_FREQUENCY_HZ) == kENotOk) { return kENotOk; }

    bus_started_ = true;
    return kEOk;
}

/*
 * Reads a block of registers, starting at one of them; kEOk if the chip acknowledged and
 * handed over every byte. Two transfers rather than one with a repeated start: the chip's
 * address pointer keeps what was written to it, so a write of the register number followed
 * by a plain read lands in the same place, and this way the code does not depend on the
 * Wire implementation's repeated-start support.
 */
StdReturnType DS3231::ReadRegisters(byte register_address, byte* values, byte length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(register_address));

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    if(Wire.requestFrom(static_cast<uint8_t>(DS3231_I2C_ADDR), static_cast<size_t>(length)) != length) { return kENotOk; }

    for(byte index = 0u; index < length; index++) { values[index] = static_cast<byte>(Wire.read()); }

    return kEOk;
}

// Writes a block of registers, starting at one of them; kEOk if the chip acknowledged the
// whole transfer.
StdReturnType DS3231::WriteRegisters(byte register_address, const byte* values, byte length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(register_address));

    for(byte index = 0u; index < length; index++) { Wire.write(static_cast<uint8_t>(values[index])); }

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    return kEOk;
}

/*
 * Tells whether the chip lost track of the time; true if what the chip counted is not to be
 * used. The flag is set by the chip itself whenever its oscillator was not running - a first
 * power-up, or a battery that ran out - and it stays set until somebody clears it. A chip
 * that does not answer is treated the same way, because a time that cannot be read is no
 * better than one that cannot be believed.
 */
bool DS3231::HasOscillatorStopped()
{
    byte status{0u};

    if(ReadRegisters(DS3231_REG_STATUS, &status, 1u) == kENotOk) { return true; }

    return (status & DS3231_STATUS_OSCILLATOR_STOPPED_MASK) != 0u;
}

/*
 * Declares what the chip counts believable again; kEOk if the flag is cleared. Read, modify,
 * write rather than a plain write: the register carries the alarm flags and the square wave
 * setting beside this bit, and a clock that clobbered them would switch off somebody else's
 * alarm.
 */
StdReturnType DS3231::ClearOscillatorStopped()
{
    byte status{0u};

    if(ReadRegisters(DS3231_REG_STATUS, &status, 1u) == kENotOk) { return kENotOk; }

    status = static_cast<byte>(status & ~DS3231_STATUS_OSCILLATOR_STOPPED_MASK);

    return WriteRegisters(DS3231_REG_STATUS, &status, 1u);
}

/*
 * Reads the temperature register pair; kEOk if both bytes arrived. A chip that does not
 * answer leaves the last reading in place, including its validity: the overlay would
 * otherwise disappear for one period because of a single lost transfer, which is a worse
 * answer than a value that is a few minutes old on something that changes as slowly as a
 * room does.
 */
StdReturnType DS3231::ReadTemperature()
{
    byte registers[DS3231_TEMPERATURE_NUMBER_OF_BYTES]{};

    if(ReadRegisters(DS3231_REG_TEMPERATURE_MSB, registers, DS3231_TEMPERATURE_NUMBER_OF_BYTES) == kENotOk) { return kENotOk; }

    temperature_ = ToTenths(static_cast<int8_t>(registers[0u]), registers[1u]);
    temperature_valid_ = true;
    return kEOk;
}
