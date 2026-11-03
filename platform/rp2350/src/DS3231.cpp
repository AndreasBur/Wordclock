/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       DS3231.cpp
 *      \brief      Temperature of the DS3231 real time clock chip
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of DS3231.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Wire.h>

#include "DS3231.h"

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
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

/* The chip keeps two year digits and a century bit; ClockDate covers 2000 to 2099, so the
   bit is always clear and the century is this constant. */
constexpr ClockDateTime::YearType kYearOfCentury{2000u};

/* The chip counts weekdays from one, ClockDate from zero. Neither says which day is
   first, and nothing reads the register back, so only the range has to match. */
constexpr byte kWeekdayOffset{1u};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
/*! \brief          Takes one reading from the chip
 *  \details        Set up from here rather than from an Init() call, the same way the light
 *                  sensor is: there is no place in the core to call an Init() from, and
 *                  retrying from the task means a chip that is plugged in later, or a bus
 *                  that was busy at boot, still comes up.
******************************************************************************************************************************************************/
void DS3231::Task()
{
    if(StartBus() == kENotOk) { return; }

    ReadTemperature();
} /* Task */


/******************************************************************************************************************************************************
  GetDateTime()
******************************************************************************************************************************************************/
/*! \brief          Reads what the chip counted, in UTC
 *  \details        The stopped flag is asked first and not afterwards: a chip whose
 *                  oscillator stood still has registers that still read like a date, and
 *                  taking that date would set the clock to whenever the battery gave out.
 *
 *                  What the setters reject is what decides whether the block is a date at
 *                  all - a fresh chip reads 0x00 everywhere, which is month zero and day
 *                  zero, and a transfer that lost its way reads anything. Hence the value
 *                  is assembled through ClockDateTime's own setters and their return
 *                  codes rather than trusted.
 *
 *  \return         kEOk if the chip answered with a believable date and time
******************************************************************************************************************************************************/
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
} /* GetDateTime */


/******************************************************************************************************************************************************
  SetDateTime()
******************************************************************************************************************************************************/
/*! \brief          Writes the time to the chip, in UTC
 *  \details        The seconds register goes out with the rest rather than being written
 *                  last: writing it is what restarts the chip's own second, so the value
 *                  lands as a whole.
 *
 *                  The weekday register is filled although nothing here reads it back -
 *                  the date carries the weekday anyway, and a register left at zero is one
 *                  that another reader of this chip would be misled by.
 *
 *  \return         kEOk if the chip took the whole block and the stopped flag is cleared
******************************************************************************************************************************************************/
StdReturnType DS3231::SetDateTime(const ClockDateTime& date_time)
{
    if(StartBus() == kENotOk) { return kENotOk; }

    byte registers[DS3231_DATE_TIME_NUMBER_OF_BYTES]{};

    registers[kRegisterSeconds] = ToBcd(date_time.GetTimeSecond());
    registers[kRegisterMinutes] = ToBcd(date_time.GetTimeMinute());
    registers[kRegisterHours] = ToBcd(date_time.GetTimeHour());
    registers[kRegisterWeekday] = ToBcd(static_cast<byte>(date_time.GetDateWeekday() + kWeekdayOffset));
    registers[kRegisterDay] = ToBcd(date_time.GetDateDay());
    registers[kRegisterMonth] = ToBcd(date_time.GetDateMonth());
    registers[kRegisterYear] = ToBcd(static_cast<byte>(date_time.GetDateYear() - kYearOfCentury));

    if(WriteRegisters(DS3231_REG_SECONDS, registers, DS3231_DATE_TIME_NUMBER_OF_BYTES) == kENotOk) { return kENotOk; }

    return ClearOscillatorStopped();
} /* SetDateTime */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  StartBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the I2C bus, once
 *  \details        Shared with the light sensor, and opening it twice is what the Wire
 *                  library allows for - whichever of the two runs first does it.
 *
 *  \return         kEOk, which is all this core can tell us
******************************************************************************************************************************************************/
StdReturnType DS3231::StartBus()
{
    if(bus_started_) { return kEOk; }

    /* Four calls where the ESP32 has one: this core's begin() takes no arguments and
       returns nothing, and the clock rate is set afterwards because begin() would otherwise
       put the default back over it. The pin setters are not the check that replaces
       begin()'s return value, although they read like one - see BH1750::StartBus() for why
       an kENotOk branch on them cannot be reached, and what carries the case instead. */
    Wire.setSDA(DS3231_I2C_PIN_SDA);
    Wire.setSCL(DS3231_I2C_PIN_SCL);

    Wire.begin();
    Wire.setClock(DS3231_I2C_FREQUENCY_HZ);

    bus_started_ = true;
    return kEOk;
} /* StartBus */


/******************************************************************************************************************************************************
  ReadRegisters()
******************************************************************************************************************************************************/
/*! \brief          Reads a block of registers, starting at one of them
 *  \details        Two transfers rather than one with a repeated start: the chip's address
 *                  pointer keeps what was written to it, so a write of the register number
 *                  followed by a plain read lands in the same place, and this way the code
 *                  does not depend on the Wire implementation's repeated-start support.
 *
 *  \return         kEOk if the chip acknowledged and handed over every byte
******************************************************************************************************************************************************/
StdReturnType DS3231::ReadRegisters(byte register_address, byte* values, byte length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(register_address));

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    if(Wire.requestFrom(static_cast<uint8_t>(DS3231_I2C_ADDR), static_cast<size_t>(length)) != length) { return kENotOk; }

    for(byte index = 0u; index < length; index++) { values[index] = static_cast<byte>(Wire.read()); }

    return kEOk;
} /* ReadRegisters */


/******************************************************************************************************************************************************
  WriteRegisters()
******************************************************************************************************************************************************/
/*! \brief          Writes a block of registers, starting at one of them
 *
 *  \return         kEOk if the chip acknowledged the whole transfer
******************************************************************************************************************************************************/
StdReturnType DS3231::WriteRegisters(byte register_address, const byte* values, byte length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(register_address));

    for(byte index = 0u; index < length; index++) { Wire.write(static_cast<uint8_t>(values[index])); }

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    return kEOk;
} /* WriteRegisters */


/******************************************************************************************************************************************************
  HasOscillatorStopped()
******************************************************************************************************************************************************/
/*! \brief          Tells whether the chip lost track of the time
 *  \details        The flag is set by the chip itself whenever its oscillator was not
 *                  running - a first power-up, or a battery that ran out - and it stays
 *                  set until somebody clears it. A chip that does not answer is treated
 *                  the same way, because a time that cannot be read is no better than one
 *                  that cannot be believed.
 *
 *  \return         true if what the chip counted is not to be used
******************************************************************************************************************************************************/
bool DS3231::HasOscillatorStopped()
{
    byte status{0u};

    if(ReadRegisters(DS3231_REG_STATUS, &status, 1u) == kENotOk) { return true; }

    return (status & DS3231_STATUS_OSCILLATOR_STOPPED_MASK) != 0u;
} /* HasOscillatorStopped */


/******************************************************************************************************************************************************
  ClearOscillatorStopped()
******************************************************************************************************************************************************/
/*! \brief          Declares what the chip counts believable again
 *  \details        Read, modify, write rather than a plain write: the register carries the
 *                  alarm flags and the square wave setting beside this bit, and a clock
 *                  that clobbered them would switch off somebody else's alarm.
 *
 *  \return         kEOk if the flag is cleared
******************************************************************************************************************************************************/
StdReturnType DS3231::ClearOscillatorStopped()
{
    byte status{0u};

    if(ReadRegisters(DS3231_REG_STATUS, &status, 1u) == kENotOk) { return kENotOk; }

    status = static_cast<byte>(status & ~DS3231_STATUS_OSCILLATOR_STOPPED_MASK);

    return WriteRegisters(DS3231_REG_STATUS, &status, 1u);
} /* ClearOscillatorStopped */


/******************************************************************************************************************************************************
  ReadTemperature()
******************************************************************************************************************************************************/
/*! \brief          Reads the temperature register pair
 *  \details        A chip that does not answer leaves the last reading in place, including
 *                  its validity: the overlay would otherwise disappear for one period
 *                  because of a single lost transfer, which is a worse answer than a
 *                  value that is a few minutes old on something that changes as slowly as
 *                  a room does.
 *
 *  \return         kEOk if both bytes arrived
******************************************************************************************************************************************************/
StdReturnType DS3231::ReadTemperature()
{
    byte registers[DS3231_TEMPERATURE_NUMBER_OF_BYTES]{};

    if(ReadRegisters(DS3231_REG_TEMPERATURE_MSB, registers, DS3231_TEMPERATURE_NUMBER_OF_BYTES) == kENotOk) { return kENotOk; }

    temperature_ = ToTenths(static_cast<int8_t>(registers[0u]), registers[1u]);
    temperature_valid_ = true;
    return kEOk;
} /* ReadTemperature */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
