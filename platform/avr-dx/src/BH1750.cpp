// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The ambient-light sensor the display dims itself by, see BH1750.h.
 */

#include "Arduino.h"
#include "BH1750.h"
#include "Bit.h"
#include "Twi.h"

StdReturnType BH1750::Init(ModeType mode) {
  if (StartBus() == kENotOk) {
    return kENotOk;
  }

  /*
   * Sent explicitly, unlike in the xmega driver: a sensor that is still in power-down -
   * which is where it sits after reset, and where a one-time measurement leaves it -
   * ignores a mode command.
   */
  if (SendCommand(BH1750_CMD_POWER_ON) == kENotOk) {
    return kENotOk;
  }

  return SetMode(mode);
}

/*
 * The mode is assigned only once the sensor has acknowledged, because it is also what
 * IsReady() reads to decide whether the sensor is set up. Assigning first marks a sensor
 * that never took the command as ready, and the retry in Task() then never runs again -
 * which leaves the sensor powered on but converting nothing, and every later reading is the
 * empty data register.
 */
StdReturnType BH1750::SetMode(ModeType mode) {
  if (mode == ModeType::kNone) {
    return kENotOk;
  }

  if (SendCommand(ToUnderlying(mode)) == kENotOk) {
    return kENotOk;
  }

  mode_ = mode;
  return kEOk;
}

/*
 * The register is split over two commands, and in both of them the value bits sit below
 * the opcode bits - so the value is ORed in rather than shifted into place.
 */
StdReturnType BH1750::ChangeMeasurementTime(byte mt_reg_value) {
  if (!IsMtRegValueInRange(mt_reg_value)) {
    return kENotOk;
  }

  byte const high_bits =
      static_cast<byte>(BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS |
                        Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_HIGH_BITS_GM,
                                                BH1750_MT_REG_VALUE_HIGH_BITS_GP));
  byte const low_bits =
      static_cast<byte>(BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS |
                        Bit<byte>::ReadBitGroup(mt_reg_value, BH1750_MT_REG_VALUE_LOW_BITS_GM,
                                                BH1750_MT_REG_VALUE_LOW_BITS_GP));

  if (SendCommand(high_bits) == kENotOk) {
    return kENotOk;
  }
  if (SendCommand(low_bits) == kENotOk) {
    return kENotOk;
  }

  // The mode has to be sent again: the sensor takes the new measurement time only with
  // the next measurement command.
  if (SendMode() == kENotOk) {
    return kENotOk;
  }

  /*
   * Kept last, and only once all three went through, because this is what
   * ConvertRawToLux() divides by: a value stored after a failed transfer would scale every
   * later reading by a measurement time the sensor is not using.
   */
  measurement_time_ = mt_reg_value;
  return kEOk;
}

void BH1750::Task() {
  /*
   * Nothing is read on the run that sets the sensor up: the first high-resolution
   * conversion needs 120 ms and the data register reads zero until it is over, so reading
   * on would hand the brightness automatic a darkness that was never measured. The next
   * run is a second away, which is well past that.
   */
  if (!IsReady()) {
    Init(BH1750_DEFAULT_MODE);
    return;
  }

  if (ReadIlluminance() == kENotOk) {
    CountReadFailure();
    return;
  }

  read_failures_ = 0u;
  SendModeForOneTimeMode();
}

StdReturnType BH1750::StartBus() {
  Twi::GetInstance().Init();
  bus_started_ = true;

  return kEOk;
}

/*
 * Sends one opcode. The sensor has no register pointer: every command is a single byte,
 * and a measurement is read straight back without addressing anything.
 */
StdReturnType BH1750::SendCommand(byte command) {
  return Twi::GetInstance().Write(BH1750_I2C_ADDR, &command, 1u);
}

/*
 * Reads the last measurement and converts it to lux. The reading is kept as it was when
 * the sensor does not answer, rather than falling to zero: the brightness automatic works
 * on this value, so a dropped reading would otherwise dim the display for a second. What
 * bounds that is the caller - see CountReadFailure().
 */
StdReturnType BH1750::ReadIlluminance() {
  byte raw_value[BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES]{};

  if (Twi::GetInstance().Read(BH1750_I2C_ADDR, raw_value,
                              BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES) == kENotOk) {
    return kENotOk;
  }

  illuminance_ = ConvertRawToLux(CombineRawValueParts(raw_value[0u], raw_value[1u]));

  return kEOk;
}

/*
 * Starts the next measurement when the sensor only makes one at a time. A one-time
 * measurement leaves the sensor powered down, so the next task would read the same value
 * again unless the mode is sent afresh - and it takes the power-on command before it listens
 * to that mode, which is the same departure from the xmega driver Init() makes and which the
 * other two backends make here.
 */
void BH1750::SendModeForOneTimeMode() {
  if (!IsOneTimeMode()) {
    return;
  }

  SendCommand(BH1750_CMD_POWER_ON);
  SendMode();
}
