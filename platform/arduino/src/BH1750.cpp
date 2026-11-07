// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief BH1750 ambient light sensor driver over I2C.
 *
 * Follows the xmega driver's register handling, with two departures noted at the places they
 * happen: the power-on command, and the measurement mode being sent once instead of on every
 * reading.
 */

/* Ahead of BH1750.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Wire.h>

#include "BH1750.h"
#include "Bit.h"
#include "I2cBus.h"

namespace {
/* Wire reports zero from endTransmission() when the slave acknowledged. */
constexpr uint8_t kWireTransmissionSuccess{0u};

} // namespace

StdReturnType BH1750::Init(ModeType mode)
{
    if(StartBus() == kENotOk) { return kENotOk; }

    /*
     * Sent explicitly, unlike in the xmega driver: a sensor that is still in power-down -
     * which is where it sits after reset, and where a one-time measurement leaves it -
     * ignores a mode command.
     */
    if(SendCommand(BH1750_CMD_POWER_ON) == kENotOk) { return kENotOk; }

    return SetMode(mode);
}

/*
 * The mode is assigned only once the sensor has acknowledged, because it is also what
 * IsReady() reads to decide whether the sensor is set up. Assigning first marks a sensor
 * that never took the command as ready, and the retry in Task() then never runs again -
 * which leaves the sensor powered on but converting nothing, and every later reading is the
 * empty data register.
 */
StdReturnType BH1750::SetMode(ModeType mode)
{
    if(mode == ModeType::kNone) { return kENotOk; }

    if(SendCommand(ToUnderlying(mode)) == kENotOk) { return kENotOk; }

    mode_ = mode;
    return kEOk;
}

/*
 * The register is split over two commands, and in both of them the value bits sit below
 * the opcode bits - so the value is ORed in rather than shifted into place.
 */
StdReturnType BH1750::ChangeMeasurementTime(byte mt_reg_value)
{
    if(!IsMtRegValueInRange(mt_reg_value)) { return kENotOk; }

    const byte high_bits = Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_HIGH_BITS_GM, BH1750_MT_REG_VALUE_HIGH_BITS_GP);
    const byte low_bits = Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_LOW_BITS_GM, BH1750_MT_REG_VALUE_LOW_BITS_GP);

    if(SendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS | high_bits) == kENotOk) { return kENotOk; }
    if(SendCommand(BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS | low_bits) == kENotOk) { return kENotOk; }
    /* The measurement time only takes effect with the next mode command. */
    if(SendMode() == kENotOk) { return kENotOk; }

    /*
     * Kept last, and only once all three went through, because this is what
     * ConvertRawToLux() divides by: a value stored after a failed transfer would scale
     * every later reading by a measurement time the sensor is not using.
     */
    measurement_time_ = mt_reg_value;
    return kEOk;
}

/*
 * In continuous mode the mode is not re-sent on every run, unlike in the xmega driver - the
 * sensor keeps converting on its own, and resending restarts the integration for no gain.
 */
void BH1750::Task()
{
    /*
     * Set up from here rather than from an Init() call, and retried until it works: see
     * the file's header for why there is no place in the core to call Init() from.
     * Nothing is read on the run that does it - the first high-resolution conversion needs
     * 120 ms and the data register reads zero until it is over, so reading on would hand
     * the brightness automatic a darkness that was never measured. The next run is a
     * second away, which is well past that.
     */
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
}

// Opens the I2C bus, once; kENotOk while I2cBus::Open() refuses, which the next call retries.
StdReturnType BH1750::StartBus()
{
    if(bus_started_) { return kEOk; }

    if(I2cBus::Open(BH1750_I2C_PIN_SDA, BH1750_I2C_PIN_SCL, BH1750_I2C_FREQUENCY_HZ) == kENotOk) { return kENotOk; }

    bus_started_ = true;
    return kEOk;
}

/*
 * Reads the last measurement and converts it to lux; kENotOk unless both bytes arrived. The
 * reading is kept as it was when the sensor does not answer, rather than falling to zero:
 * the brightness automatic works on this value, so a dropped reading would otherwise dim the
 * display for a second. What bounds that is the caller - see CountReadFailure().
 */
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
}

/*
 * Starts the next conversion when the mode does not repeat by itself. A one-time measurement
 * leaves the sensor powered down, so it takes the power-on command again before it listens
 * to the mode.
 */
void BH1750::SendModeForOneTimeMode()
{
    if(!IsOneTimeMode()) { return; }

    SendCommand(BH1750_CMD_POWER_ON);
    SendMode();
}

// Writes one command byte to the sensor; kEOk if the sensor acknowledged it.
StdReturnType BH1750::SendCommand(byte command)
{
    Wire.beginTransmission(static_cast<uint8_t>(BH1750_I2C_ADDR));
    Wire.write(command);

    if(Wire.endTransmission() != kWireTransmissionSuccess) { return kENotOk; }

    return kEOk;
}
