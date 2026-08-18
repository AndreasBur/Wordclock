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
 *      \brief      The ambient-light sensor the display dims itself by, see BH1750.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"
#include "BH1750.h"
#include "Twi.h"

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
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
 *                  Assigning first marks a sensor that never took the command as ready, and
 *                  the retry in task() then never runs again - which leaves the sensor
 *                  powered on but converting nothing, and every later reading is the empty
 *                  data register.
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
 *  \details        The register is split over two commands, and in both of them the value
 *                  bits sit below the opcode bits - so the value is ORed in rather than
 *                  shifted into place.
******************************************************************************************************************************************************/
StdReturnType BH1750::changeMeasurementTime(byte MTRegValue)
{
    if(!isMTRegValueInRange(MTRegValue)) { return E_NOT_OK; }

    const byte HighBits = static_cast<byte>(BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS |
                                            READ_BIT_GROUP(MTRegValue, BH1750_MT_REG_VALUE_HIGH_BITS_GM, BH1750_MT_REG_VALUE_HIGH_BITS_GP));
    const byte LowBits = static_cast<byte>(BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS |
                                           READ_BIT_GROUP(MTRegValue, BH1750_MT_REG_VALUE_LOW_BITS_GM, BH1750_MT_REG_VALUE_LOW_BITS_GP));

    if(sendCommand(HighBits) == E_NOT_OK) { return E_NOT_OK; }
    if(sendCommand(LowBits) == E_NOT_OK) { return E_NOT_OK; }

    /* The mode has to be sent again: the sensor takes the new measurement time only with
       the next measurement command. */
    if(sendMode() == E_NOT_OK) { return E_NOT_OK; }

    /* Kept last, and only once all three went through, because this is what
       convertRawToLux() divides by: a value stored after a failed transfer would scale every
       later reading by a measurement time the sensor is not using. */
    MeasurementTime = MTRegValue;
    return E_OK;
} /* changeMeasurementTime */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          Reads the sensor, setting it up first if that has not worked yet
 *  \details        Set up from here rather than from an init() call, and retried until it
 *                  works: the core keeps its sensor private and has no place to call init()
 *                  from. The retry costs one failing transfer per cycle on a board with no
 *                  sensor, which the bounded timeouts keep short.
******************************************************************************************************************************************************/
void BH1750::task()
{
    /* Nothing is read on the run that sets the sensor up: the first high-resolution
       conversion needs 120 ms and the data register reads zero until it is over, so reading
       on would hand the brightness automatic a darkness that was never measured. The next
       run is a second away, which is well past that. */
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
StdReturnType BH1750::startBus()
{
    Twi::getInstance().init();
    BusStarted = true;

    return E_OK;
} /* startBus */


/******************************************************************************************************************************************************
  sendCommand()
******************************************************************************************************************************************************/
/*! \brief          Sends one opcode to the sensor
 *  \details        The sensor has no register pointer: every command is a single byte, and
 *                  a measurement is read straight back without addressing anything.
******************************************************************************************************************************************************/
StdReturnType BH1750::sendCommand(byte Command)
{
    return Twi::getInstance().write(BH1750_I2C_ADDR, &Command, 1u);
} /* sendCommand */


/******************************************************************************************************************************************************
  readIlluminance()
******************************************************************************************************************************************************/
/*! \brief          Reads the last measurement and converts it to lux
 *  \details        The reading is kept as it was when the sensor does not answer, rather
 *                  than falling to zero: the brightness automatic works on this value, so a
 *                  dropped reading would otherwise dim the display for a second. What bounds
 *                  that is the caller - see countReadFailure().
******************************************************************************************************************************************************/
StdReturnType BH1750::readIlluminance()
{
    byte RawValue[BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES]{};

    if(Twi::getInstance().read(BH1750_I2C_ADDR, RawValue, BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES) == E_NOT_OK) { return E_NOT_OK; }

    Illuminance = convertRawToLux(combineRawValueParts(RawValue[0u], RawValue[1u]));

    return E_OK;
} /* readIlluminance */


/******************************************************************************************************************************************************
  sendModeForOneTimeMode()
******************************************************************************************************************************************************/
/*! \brief          Starts the next measurement when the sensor only makes one at a time
 *  \details        A one-time measurement leaves the sensor powered down, so the next task
 *                  would read the same value again unless the mode is sent afresh - and it
 *                  takes the power-on command before it listens to that mode, which is the
 *                  same departure from the xmega driver init() makes and which the other two
 *                  backends make here.
******************************************************************************************************************************************************/
void BH1750::sendModeForOneTimeMode()
{
    if(!isOneTimeMode()) { return; }

    sendCommand(BH1750_CMD_POWER_ON);
    sendMode();
} /* sendModeForOneTimeMode */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
