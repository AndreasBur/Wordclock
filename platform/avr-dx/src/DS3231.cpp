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
constexpr byte RegisterSeconds{0u};
constexpr byte RegisterMinutes{1u};
constexpr byte RegisterHours{2u};
constexpr byte RegisterWeekday{3u};
constexpr byte RegisterDay{4u};
constexpr byte RegisterMonth{5u};
constexpr byte RegisterYear{6u};

/* The chip keeps two year digits and a century bit; ClockDate covers 2000 to 2099, so the
   bit is always clear and the century is this constant. */
constexpr ClockDateTime::YearType YearOfCentury{2000u};

/* The chip counts weekdays from one, ClockDate from zero. Neither says which day is
   first, and nothing reads the register back, so only the range has to match. */
constexpr byte WeekdayOffset{1u};

} // namespace

/******************************************************************************************************************************************************
 *  P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  startBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the bus this chip shares with the light sensor
 *  \details        Idempotent, so whichever driver runs first opens it and the other finds
 *                  it open. Neither owns the bus, and neither can be relied on to run at all
 *                  on a board where only one of the two is fitted.
******************************************************************************************************************************************************/
StdReturnType DS3231::startBus()
{
    Twi::getInstance().init();

    return E_OK;
} /* startBus */


/******************************************************************************************************************************************************
  readRegisters()
******************************************************************************************************************************************************/
StdReturnType DS3231::readRegisters(byte Register, byte* Values, byte Length)
{
    return Twi::getInstance().readRegister(DS3231_I2C_ADDR, Register, Values, Length);
} /* readRegisters */


/******************************************************************************************************************************************************
  writeRegisters()
******************************************************************************************************************************************************/
/*! \brief          Writes a block of registers starting at one address
 *  \details        The block goes out in a single transfer rather than register by register:
 *                  the chip counts on while it is written, and seven separate transfers
 *                  would let the seconds roll over between two of them and leave the chip
 *                  holding a time assembled from two different ones.
******************************************************************************************************************************************************/
StdReturnType DS3231::writeRegisters(byte Register, const byte* Values, byte Length)
{
    byte Buffer[DS3231_DATE_TIME_NUMBER_OF_BYTES + 1u]{};

    if(Length > DS3231_DATE_TIME_NUMBER_OF_BYTES) { return E_NOT_OK; }

    Buffer[0u] = Register;
    for(byte Index = 0u; Index < Length; Index++) { Buffer[Index + 1u] = Values[Index]; }

    return Twi::getInstance().write(DS3231_I2C_ADDR, Buffer, static_cast<byte>(Length + 1u));
} /* writeRegisters */


/******************************************************************************************************************************************************
  hasOscillatorStopped()
******************************************************************************************************************************************************/
/*! \brief          Whether the chip admits it lost track of time
 *  \details        A chip that cannot be read is treated as stopped: both mean there is no
 *                  time to be had from it, and the caller has one case to handle instead of
 *                  two.
******************************************************************************************************************************************************/
bool DS3231::hasOscillatorStopped()
{
    byte Status{0u};

    if(readRegisters(DS3231_REG_STATUS, &Status, 1u) == E_NOT_OK) { return true; }

    return (Status & DS3231_STATUS_OSCILLATOR_STOPPED_MASK) != 0u;
} /* hasOscillatorStopped */


/******************************************************************************************************************************************************
  clearOscillatorStopped()
******************************************************************************************************************************************************/
StdReturnType DS3231::clearOscillatorStopped()
{
    byte Status{0u};

    if(readRegisters(DS3231_REG_STATUS, &Status, 1u) == E_NOT_OK) { return E_NOT_OK; }

    Status &= static_cast<byte>(~DS3231_STATUS_OSCILLATOR_STOPPED_MASK);

    return writeRegisters(DS3231_REG_STATUS, &Status, 1u);
} /* clearOscillatorStopped */


/******************************************************************************************************************************************************
  readTemperature()
******************************************************************************************************************************************************/
StdReturnType DS3231::readTemperature()
{
    byte Registers[DS3231_TEMPERATURE_NUMBER_OF_BYTES]{};

    if(readRegisters(DS3231_REG_TEMPERATURE_MSB, Registers, DS3231_TEMPERATURE_NUMBER_OF_BYTES) == E_NOT_OK) { return E_NOT_OK; }

    Temperature = toTenths(static_cast<int8_t>(Registers[0u]), Registers[1u]);
    TemperatureValid = true;

    return E_OK;
} /* readTemperature */


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void DS3231::task()
{
    if(startBus() == E_NOT_OK) { return; }

    readTemperature();
} /* task */


/******************************************************************************************************************************************************
  getDateTime()
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
StdReturnType DS3231::getDateTime(ClockDateTime& DateTime)
{
    if(startBus() == E_NOT_OK) { return E_NOT_OK; }
    if(hasOscillatorStopped()) { return E_NOT_OK; }

    byte Registers[DS3231_DATE_TIME_NUMBER_OF_BYTES]{};
    if(readRegisters(DS3231_REG_SECONDS, Registers, DS3231_DATE_TIME_NUMBER_OF_BYTES) == E_NOT_OK) { return E_NOT_OK; }

    ClockDateTime Value;
    StdReturnType ReturnValue{E_OK};

    if(Value.setTimeSecond(fromBcd(Registers[RegisterSeconds])) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    if(Value.setTimeMinute(fromBcd(Registers[RegisterMinutes])) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    if(Value.setTimeHour(fromBcd(Registers[RegisterHours] & DS3231_HOURS_VALUE_MASK)) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    if(Value.setDateDay(fromBcd(Registers[RegisterDay])) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    if(Value.setDateMonth(fromBcd(Registers[RegisterMonth] & DS3231_MONTH_VALUE_MASK)) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    if(Value.setDateYear(static_cast<ClockDateTime::YearType>(YearOfCentury + fromBcd(Registers[RegisterYear]))) == E_NOT_OK) { ReturnValue = E_NOT_OK; }

    if(ReturnValue == E_NOT_OK) { return E_NOT_OK; }

    DateTime = Value;
    return E_OK;
} /* getDateTime */


/******************************************************************************************************************************************************
  setDateTime()
******************************************************************************************************************************************************/
StdReturnType DS3231::setDateTime(const ClockDateTime& DateTime)
{
    if(startBus() == E_NOT_OK) { return E_NOT_OK; }

    byte Registers[DS3231_DATE_TIME_NUMBER_OF_BYTES]{};

    Registers[RegisterSeconds] = toBcd(DateTime.getTimeSecond());
    Registers[RegisterMinutes] = toBcd(DateTime.getTimeMinute());
    Registers[RegisterHours] = toBcd(DateTime.getTimeHour());
    Registers[RegisterWeekday] = toBcd(static_cast<byte>(DateTime.getDateWeekday() + WeekdayOffset));
    Registers[RegisterDay] = toBcd(DateTime.getDateDay());
    Registers[RegisterMonth] = toBcd(DateTime.getDateMonth());
    Registers[RegisterYear] = toBcd(static_cast<byte>(DateTime.getDateYear() - YearOfCentury));

    if(writeRegisters(DS3231_REG_SECONDS, Registers, DS3231_DATE_TIME_NUMBER_OF_BYTES) == E_NOT_OK) { return E_NOT_OK; }

    return clearOscillatorStopped();
} /* setDateTime */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
