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
#include "Bit.h"

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* Wire reports zero from endTransmission() when the slave acknowledged. */
constexpr uint8_t kWireTransmissionSuccess{0u};

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
/*! \brief          Opens the bus and puts the sensor into a measurement mode
 *
 *  \return         kEOk if the sensor acknowledged both commands
******************************************************************************************************************************************************/
StdReturnType BH1750::Init(ModeType mode)
{
    if(StartBus() == kENotOk) { return kENotOk; }

    /* Sent explicitly, unlike in the xmega driver: a sensor that is still in power-down -
       which is where it sits after reset, and where a one-time measurement leaves it -
       ignores a mode command. */
    if(SendCommand(BH1750_CMD_POWER_ON) == kENotOk) { return kENotOk; }

    return SetMode(mode);
} /* Init */


/******************************************************************************************************************************************************
  SetMode()
******************************************************************************************************************************************************/
/*! \brief          Puts the sensor into a measurement mode
 *  \details        Mode is assigned only once the sensor has acknowledged, because it is
 *                  also what IsReady() reads to decide whether the sensor is set up.
 *                  Assigning first marks a sensor that never took the command as ready,
 *                  and the retry in Task() then never runs again - which leaves the sensor
 *                  powered on but converting nothing, and every later reading is the empty
 *                  data register.
 *
 *  \return         kEOk if the sensor acknowledged the mode
******************************************************************************************************************************************************/
StdReturnType BH1750::SetMode(ModeType mode)
{
    if(mode == kModeNone) { return kENotOk; }

    if(SendCommand(mode) == kENotOk) { return kENotOk; }

    mode_ = mode;
    return kEOk;
} /* SetMode */


/******************************************************************************************************************************************************
  ChangeMeasurementTime()
******************************************************************************************************************************************************/
/*! \brief          Changes the sensor's measurement time register
 *  \details        The register is split over two commands, and in both of them the
 *                  value bits sit below the opcode bits - so the value is ORed in rather
 *                  than shifted into place.
 *
 *  \return         kEOk if the value was in range and the sensor took all three commands
******************************************************************************************************************************************************/
StdReturnType BH1750::ChangeMeasurementTime(byte mt_reg_value)
{
    if(!IsMtRegValueInRange(mt_reg_value)) { return kENotOk; }

    const byte high_bits = Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_HIGH_BITS_GM, BH1750_MT_REG_VALUE_HIGH_BITS_GP);
    const byte low_bits = Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_LOW_BITS_GM, BH1750_MT_REG_VALUE_LOW_BITS_GP);

    if(SendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS | high_bits) == kENotOk) { return kENotOk; }
    if(SendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS | low_bits) == kENotOk) { return kENotOk; }
    /* The measurement time only takes effect with the next mode command. */
    if(SendMode() == kENotOk) { return kENotOk; }

    /* Kept last, and only once all three went through, because this is what
       ConvertRawToLux() divides by: a value stored after a failed transfer would scale
       every later reading by a measurement time the sensor is not using. */
    measurement_time_ = mt_reg_value;
    return kEOk;
} /* ChangeMeasurementTime */


/******************************************************************************************************************************************************
  Task()
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
void BH1750::Task()
{
    /* Set up from here rather than from an Init() call, and retried until it works: see
       the file's header for why there is no place in the core to call Init() from.
       Nothing is read on the run that does it - the first high-resolution conversion needs
       120 ms and the data register reads zero until it is over, so reading on would hand
       the brightness automatic a darkness that was never measured. The next run is a
       second away, which is well past that. */
    if(!IsReady()) {
        Init(BH1750_DEFAULT_MODE);
        return;
    }

    if(ReadIlluminance() == kENotOk) {
        CountReadFailure();
        return;
    }

    read_failures_ = 0u;
    SendModeForOneTimeMode();
} /* Task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  StartBus()
******************************************************************************************************************************************************/
/*! \brief          Opens the I2C bus, once
 *
 *  \return         kEOk if the bus is available
******************************************************************************************************************************************************/
StdReturnType BH1750::StartBus()
{
    if(bus_started_) { return kEOk; }

    if(!Wire.begin(BH1750_I2C_PIN_SDA, BH1750_I2C_PIN_SCL, BH1750_I2C_FREQUENCY_HZ)) { return kENotOk; }

    bus_started_ = true;
    return kEOk;
} /* StartBus */


/******************************************************************************************************************************************************
  ReadIlluminance()
******************************************************************************************************************************************************/
/*! \brief          Reads the last measurement and converts it to lux
 *  \details        The reading is kept as it was when the sensor does not answer, rather
 *                  than falling to zero: the brightness automatic works on this value, so
 *                  a dropped reading would otherwise dim the display for a second. What
 *                  bounds that is the caller - see CountReadFailure().
 *
 *  \return         kEOk if both bytes arrived
******************************************************************************************************************************************************/
StdReturnType BH1750::ReadIlluminance()
{
    if(Wire.requestFrom(static_cast<uint8_t>(BH1750_I2C_ADDR),
                        static_cast<size_t>(BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES)) != BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES) {
        return kENotOk;
    }

    const byte high_byte = static_cast<byte>(Wire.read());
    const byte low_byte = static_cast<byte>(Wire.read());

    illuminance_ = ConvertRawToLux(CombineRawValueParts(high_byte, low_byte));
    return kEOk;
} /* ReadIlluminance */


/******************************************************************************************************************************************************
  SendModeForOneTimeMode()
******************************************************************************************************************************************************/
/*! \brief          Starts the next conversion when the mode does not repeat by itself
 *  \details        A one-time measurement leaves the sensor powered down, so it takes the
 *                  power-on command again before it listens to the mode.
******************************************************************************************************************************************************/
void BH1750::SendModeForOneTimeMode()
{
    if(!IsOneTimeMode()) { return; }

    SendCommand(BH1750_CMD_POWER_ON);
    SendMode();
} /* SendModeForOneTimeMode */


/******************************************************************************************************************************************************
  SendCommand()
******************************************************************************************************************************************************/
/*! \brief          Writes one command byte to the sensor
 *
 *  \return         kEOk if the sensor acknowledged
******************************************************************************************************************************************************/
StdReturnType BH1750::SendCommand(byte command)
{
    Wire.beginTransmission(static_cast<uint8_t>(BH1750_I2C_ADDR));
    Wire.write(command);

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    return kEOk;
} /* SendCommand */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
