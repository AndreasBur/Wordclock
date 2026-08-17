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
constexpr uint8_t WireTransmissionSuccess{0u};

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
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          Takes one reading from the chip
 *  \details        Set up from here rather than from an init() call, the same way the light
 *                  sensor is: there is no place in the core to call an init() from, and
 *                  retrying from the task means a chip that is plugged in later, or a bus
 *                  that was busy at boot, still comes up.
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
 *                  zero, and a transfer that lost its way reads anything. Hence the value
 *                  is assembled through ClockDateTime's own setters and their return
 *                  codes rather than trusted.
 *
 *  \return         E_OK if the chip answered with a believable date and time
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
/*! \brief          Writes the time to the chip, in UTC
 *  \details        The seconds register goes out with the rest rather than being written
 *                  last: writing it is what restarts the chip's own second, so the value
 *                  lands as a whole.
 *
 *                  The weekday register is filled although nothing here reads it back -
 *                  the date carries the weekday anyway, and a register left at zero is one
 *                  that another reader of this chip would be misled by.
 *
 *  \return         E_OK if the chip took the whole block and the stopped flag is cleared
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
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  startBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the I2C bus, once
 *  \details        Shared with the light sensor, and opening it twice is what the Wire
 *                  library allows for - whichever of the two runs first does it.
 *
 *  \return         E_OK if the bus is available
******************************************************************************************************************************************************/
StdReturnType DS3231::startBus()
{
    if(BusStarted) { return E_OK; }

    /* Three calls where the ESP32 has one: this core's begin() takes no arguments and
       returns nothing. The pins are what can be refused - setSDA() and setSCL() fail if
       the bus is already running or the pin cannot carry that signal - so they carry the
       check that begin()'s return value carries there. The clock rate is set afterwards
       because begin() would otherwise put the default back over it. */
    if(!Wire.setSDA(DS3231_I2C_PIN_SDA)) { return E_NOT_OK; }
    if(!Wire.setSCL(DS3231_I2C_PIN_SCL)) { return E_NOT_OK; }

    Wire.begin();
    Wire.setClock(DS3231_I2C_FREQUENCY_HZ);

    BusStarted = true;
    return E_OK;
} /* startBus */


/******************************************************************************************************************************************************
  readRegisters()
******************************************************************************************************************************************************/
/*! \brief          Reads a block of registers, starting at one of them
 *  \details        Two transfers rather than one with a repeated start: the chip's address
 *                  pointer keeps what was written to it, so a write of the register number
 *                  followed by a plain read lands in the same place, and this way the code
 *                  does not depend on the Wire implementation's repeated-start support.
 *
 *  \return         E_OK if the chip acknowledged and handed over every byte
******************************************************************************************************************************************************/
StdReturnType DS3231::readRegisters(byte Register, byte* Values, byte Length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(Register));

    if(Wire.endTransmission() != WireTransmissionSuccess) { return E_NOT_OK; }

    if(Wire.requestFrom(static_cast<uint8_t>(DS3231_I2C_ADDR), static_cast<size_t>(Length)) != Length) { return E_NOT_OK; }

    for(byte Index = 0u; Index < Length; Index++) { Values[Index] = static_cast<byte>(Wire.read()); }

    return E_OK;
} /* readRegisters */


/******************************************************************************************************************************************************
  writeRegisters()
******************************************************************************************************************************************************/
/*! \brief          Writes a block of registers, starting at one of them
 *
 *  \return         E_OK if the chip acknowledged the whole transfer
******************************************************************************************************************************************************/
StdReturnType DS3231::writeRegisters(byte Register, const byte* Values, byte Length)
{
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(Register));

    for(byte Index = 0u; Index < Length; Index++) { Wire.write(static_cast<uint8_t>(Values[Index])); }

    if(Wire.endTransmission() != WireTransmissionSuccess) { return E_NOT_OK; }

    return E_OK;
} /* writeRegisters */


/******************************************************************************************************************************************************
  hasOscillatorStopped()
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
bool DS3231::hasOscillatorStopped()
{
    byte Status{0u};

    if(readRegisters(DS3231_REG_STATUS, &Status, 1u) == E_NOT_OK) { return true; }

    return (Status & DS3231_STATUS_OSCILLATOR_STOPPED_MASK) != 0u;
} /* hasOscillatorStopped */


/******************************************************************************************************************************************************
  clearOscillatorStopped()
******************************************************************************************************************************************************/
/*! \brief          Declares what the chip counts believable again
 *  \details        Read, modify, write rather than a plain write: the register carries the
 *                  alarm flags and the square wave setting beside this bit, and a clock
 *                  that clobbered them would switch off somebody else's alarm.
 *
 *  \return         E_OK if the flag is cleared
******************************************************************************************************************************************************/
StdReturnType DS3231::clearOscillatorStopped()
{
    byte Status{0u};

    if(readRegisters(DS3231_REG_STATUS, &Status, 1u) == E_NOT_OK) { return E_NOT_OK; }

    Status = static_cast<byte>(Status & ~DS3231_STATUS_OSCILLATOR_STOPPED_MASK);

    return writeRegisters(DS3231_REG_STATUS, &Status, 1u);
} /* clearOscillatorStopped */


/******************************************************************************************************************************************************
  readTemperature()
******************************************************************************************************************************************************/
/*! \brief          Reads the temperature register pair
 *  \details        A chip that does not answer leaves the last reading in place, including
 *                  its validity: the overlay would otherwise disappear for one period
 *                  because of a single lost transfer, which is a worse answer than a
 *                  value that is a few minutes old on something that changes as slowly as
 *                  a room does.
 *
 *  \return         E_OK if both bytes arrived
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
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
