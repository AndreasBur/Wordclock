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
 *      \brief      The clock chip: the time it counted, and the temperature it measures, see DS3231.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"
#include "DS3231.h"
#include "Twi.h"

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

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

/******************************************************************************************************************************************************
 *  P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  StartBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the bus this chip shares with the light sensor
 *  \details        Idempotent, so whichever driver runs first opens it and the other finds
 *                  it open. Neither owns the bus, and neither can be relied on to run at all
 *                  on a board where only one of the two is fitted.
******************************************************************************************************************************************************/
StdReturnType DS3231::StartBus()
{
    Twi::GetInstance().Init();

    return kEOk;
} /* StartBus */


/******************************************************************************************************************************************************
  ReadRegisters()
******************************************************************************************************************************************************/
StdReturnType DS3231::ReadRegisters(byte register_address, byte* values, byte length)
{
    return Twi::GetInstance().ReadRegister(DS3231_I2C_ADDR, register_address, values, length);
} /* ReadRegisters */


/******************************************************************************************************************************************************
  WriteRegisters()
******************************************************************************************************************************************************/
/*! \brief          Writes a block of registers starting at one address
 *  \details        The block goes out in a single transfer rather than register by register:
 *                  the chip counts on while it is written, and seven separate transfers
 *                  would let the seconds roll over between two of them and leave the chip
 *                  holding a time assembled from two different ones.
******************************************************************************************************************************************************/
StdReturnType DS3231::WriteRegisters(byte register_address, const byte* values, byte length)
{
    byte buffer[DS3231_DATE_TIME_NUMBER_OF_BYTES + 1u]{};

    if(length > DS3231_DATE_TIME_NUMBER_OF_BYTES) { return kENotOk; }

    buffer[0u] = register_address;
    for(byte index = 0u; index < length; index++) { buffer[index + 1u] = values[index]; }

    return Twi::GetInstance().Write(DS3231_I2C_ADDR, buffer, static_cast<byte>(length + 1u));
} /* WriteRegisters */


/******************************************************************************************************************************************************
  HasOscillatorStopped()
******************************************************************************************************************************************************/
/*! \brief          Whether the chip admits it lost track of time
 *  \details        A chip that cannot be read is treated as stopped: both mean there is no
 *                  time to be had from it, and the caller has one case to handle instead of
 *                  two.
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
StdReturnType DS3231::ClearOscillatorStopped()
{
    byte status{0u};

    if(ReadRegisters(DS3231_REG_STATUS, &status, 1u) == kENotOk) { return kENotOk; }

    status &= static_cast<byte>(~DS3231_STATUS_OSCILLATOR_STOPPED_MASK);

    return WriteRegisters(DS3231_REG_STATUS, &status, 1u);
} /* ClearOscillatorStopped */


/******************************************************************************************************************************************************
  ReadTemperature()
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
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
void DS3231::Task()
{
    if(StartBus() == kENotOk) { return; }

    ReadTemperature();
} /* Task */


/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
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
 *                  zero, and a transfer that lost its way reads anything. Hence the value is
 *                  assembled through ClockDateTime's own setters and their return codes
 *                  rather than trusted.
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
} /* SetDateTime */
// NOLINTEND(readability-convert-member-functions-to-static)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
