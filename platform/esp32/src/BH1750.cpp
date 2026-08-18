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
/**     \file       BH1750.cpp
 *      \brief      BH1750 ambient light sensor driver over I2C
 *
 *      \details    Follows the xmega driver's register handling, with two departures
 *                  noted at the places they happen: the power-on command, and the
 *                  measurement mode being sent once instead of on every reading.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of BH1750.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Wire.h>

#include "BH1750.h"

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
  init()
******************************************************************************************************************************************************/
/*! \brief          Opens the bus and puts the sensor into a measurement mode
 *
 *  \return         E_OK if the sensor acknowledged both commands
******************************************************************************************************************************************************/
StdReturnType BH1750::init(ModeType sMode)
{
    if(startBus() == E_NOT_OK) { return E_NOT_OK; }

    /* Sent explicitly, unlike in the xmega driver: a sensor that is still in power-down -
       which is where it sits after reset, and where a one-time measurement leaves it -
       ignores a mode command. */
    if(sendCommand(BH1750_CMD_POWER_ON) == E_NOT_OK) { return E_NOT_OK; }

    return setMode(sMode);
} /* init */


/******************************************************************************************************************************************************
  setMode()
******************************************************************************************************************************************************/
/*! \brief          Puts the sensor into a measurement mode
 *  \details        Mode is assigned only once the sensor has acknowledged, because it is
 *                  also what isReady() reads to decide whether the sensor is set up.
 *                  Assigning first marks a sensor that never took the command as ready,
 *                  and the retry in task() then never runs again - which leaves the sensor
 *                  powered on but converting nothing, and every later reading is the empty
 *                  data register.
 *
 *  \return         E_OK if the sensor acknowledged the mode
******************************************************************************************************************************************************/
StdReturnType BH1750::setMode(ModeType sMode)
{
    if(sMode == MODE_NONE) { return E_NOT_OK; }

    if(sendCommand(sMode) == E_NOT_OK) { return E_NOT_OK; }

    Mode = sMode;
    return E_OK;
} /* setMode */


/******************************************************************************************************************************************************
  changeMeasurementTime()
******************************************************************************************************************************************************/
/*! \brief          Changes the sensor's measurement time register
 *  \details        The register is split over two commands, and in both of them the
 *                  value bits sit below the opcode bits - so the value is ORed in rather
 *                  than shifted into place.
 *
 *  \return         E_OK if the value was in range and the sensor took all three commands
******************************************************************************************************************************************************/
StdReturnType BH1750::changeMeasurementTime(byte MTRegValue)
{
    if(!isMTRegValueInRange(MTRegValue)) { return E_NOT_OK; }

    const byte HighBits = READ_BIT_GROUP(MTRegValue, BH1750_MT_REG_VALUE_HIGH_BITS_GM, BH1750_MT_REG_VALUE_HIGH_BITS_GP);
    const byte LowBits = READ_BIT_GROUP(MTRegValue, BH1750_MT_REG_VALUE_LOW_BITS_GM, BH1750_MT_REG_VALUE_LOW_BITS_GP);

    if(sendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS | HighBits) == E_NOT_OK) { return E_NOT_OK; }
    if(sendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS | LowBits) == E_NOT_OK) { return E_NOT_OK; }
    /* The measurement time only takes effect with the next mode command. */
    if(sendMode() == E_NOT_OK) { return E_NOT_OK; }

    /* Kept last, and only once all three went through, because this is what
       convertRawToLux() divides by: a value stored after a failed transfer would scale
       every later reading by a measurement time the sensor is not using. */
    MeasurementTime = MTRegValue;
    return E_OK;
} /* changeMeasurementTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          Takes one reading from the sensor
 *  \details        Runs every BH1750_TASK_CYCLE ticks, so once a second. That is what
 *                  lets a one-time measurement be triggered at the end and collected on
 *                  the next run: the 120 ms it needs are long gone by then, and nothing
 *                  here has to wait for the conversion.
 *
 *                  In continuous mode the mode is not re-sent on every run, unlike in the
 *                  xmega driver - the sensor keeps converting on its own, and resending
 *                  restarts the integration for no gain.
******************************************************************************************************************************************************/
void BH1750::task()
{
    /* Set up from here rather than from an init() call, and retried until it works: see
       the file's header for why there is no place in the core to call init() from.
       Nothing is read on the run that does it - the first high-resolution conversion needs
       120 ms and the data register reads zero until it is over, so reading on would hand
       the brightness automatic a darkness that was never measured. The next run is a
       second away, which is well past that. */
    if(!isReady()) {
        init(BH1750_DEFAULT_MODE);
        return;
    }

    if(readIlluminance() == E_NOT_OK) {
        countReadFailure();
        return;
    }

    ReadFailures = 0u;
    sendModeForOneTimeMode();
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  startBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the I2C bus, once
 *
 *  \return         E_OK if the bus is available
******************************************************************************************************************************************************/
StdReturnType BH1750::startBus()
{
    if(BusStarted) { return E_OK; }

    if(!Wire.begin(BH1750_I2C_PIN_SDA, BH1750_I2C_PIN_SCL, BH1750_I2C_FREQUENCY_HZ)) { return E_NOT_OK; }

    BusStarted = true;
    return E_OK;
} /* startBus */


/******************************************************************************************************************************************************
  readIlluminance()
******************************************************************************************************************************************************/
/*! \brief          Reads the last measurement and converts it to lux
 *  \details        The reading is kept as it was when the sensor does not answer, rather
 *                  than falling to zero: the brightness automatic works on this value, so
 *                  a dropped reading would otherwise dim the display for a second. What
 *                  bounds that is the caller - see countReadFailure().
 *
 *  \return         E_OK if both bytes arrived
******************************************************************************************************************************************************/
StdReturnType BH1750::readIlluminance()
{
    if(Wire.requestFrom(static_cast<uint8_t>(BH1750_I2C_ADDR),
                        static_cast<size_t>(BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES)) != BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES) {
        return E_NOT_OK;
    }

    const byte HighByte = static_cast<byte>(Wire.read());
    const byte LowByte = static_cast<byte>(Wire.read());

    Illuminance = convertRawToLux(combineRawValueParts(HighByte, LowByte));
    return E_OK;
} /* readIlluminance */


/******************************************************************************************************************************************************
  sendModeForOneTimeMode()
******************************************************************************************************************************************************/
/*! \brief          Starts the next conversion when the mode does not repeat by itself
 *  \details        A one-time measurement leaves the sensor powered down, so it takes the
 *                  power-on command again before it listens to the mode.
******************************************************************************************************************************************************/
void BH1750::sendModeForOneTimeMode()
{
    if(!isOneTimeMode()) { return; }

    sendCommand(BH1750_CMD_POWER_ON);
    sendMode();
} /* sendModeForOneTimeMode */


/******************************************************************************************************************************************************
  sendCommand()
******************************************************************************************************************************************************/
/*! \brief          Writes one command byte to the sensor
 *
 *  \return         E_OK if the sensor acknowledged
******************************************************************************************************************************************************/
StdReturnType BH1750::sendCommand(byte Command)
{
    Wire.beginTransmission(static_cast<uint8_t>(BH1750_I2C_ADDR));
    Wire.write(Command);

    if(Wire.endTransmission() != WireTransmissionSuccess) { return E_NOT_OK; }

    return E_OK;
} /* sendCommand */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
