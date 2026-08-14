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

    if(!Wire.begin(DS3231_I2C_PIN_SDA, DS3231_I2C_PIN_SCL, DS3231_I2C_FREQUENCY_HZ)) { return E_NOT_OK; }

    BusStarted = true;
    return E_OK;
} /* startBus */


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
    Wire.beginTransmission(static_cast<uint8_t>(DS3231_I2C_ADDR));
    Wire.write(static_cast<uint8_t>(DS3231_REG_TEMPERATURE_MSB));

    if(Wire.endTransmission() != WireTransmissionSuccess) { return E_NOT_OK; }

    if(Wire.requestFrom(static_cast<uint8_t>(DS3231_I2C_ADDR),
                        static_cast<size_t>(DS3231_TEMPERATURE_NUMBER_OF_BYTES)) != DS3231_TEMPERATURE_NUMBER_OF_BYTES) {
        return E_NOT_OK;
    }

    const int8_t Degrees = static_cast<int8_t>(Wire.read());
    const byte FractionRegister = static_cast<byte>(Wire.read());

    Temperature = toTenths(Degrees, FractionRegister);
    TemperatureValid = true;
    return E_OK;
} /* readTemperature */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
