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
StdReturnType BH1750::setMode(ModeType sMode)
{
    Mode = sMode;

    if(Mode == MODE_NONE) { return E_NOT_OK; }

    return sendMode();
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
    return sendMode();
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
    if(!BusStarted || (Mode == MODE_NONE)) {
        if(init(BH1750_DEFAULT_MODE) == E_NOT_OK) { return; }
    }

    readIlluminance();
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
 *                  would read the same value again unless the mode is sent afresh.
******************************************************************************************************************************************************/
void BH1750::sendModeForOneTimeMode()
{
    if(!isOneTimeMode()) { return; }

    sendMode();
} /* sendModeForOneTimeMode */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
