// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief I2cBus on the RP2350's Arduino core, which sets the pins and the clock one call each and
 *        answers none of them.
 */

#include "I2cBus.h"

#include <Wire.h>

/*
 * Four calls where the ESP32 has one: this core's begin() takes no arguments and returns nothing,
 * and the clock rate is set afterwards because begin() would otherwise put the default back over
 * it.
 *
 * The pin setters look like the check that replaces begin()'s return value. They are not one: a
 * pin they will not take ends in the SDK's panic(), which is declared noreturn, so a kENotOk branch
 * on them cannot be reached and only reads as if the case were handled. What makes the call safe
 * instead is that both drivers on this bus name the same two pins - the second one through hits
 * setSDA()'s "already this pin" case, and begin() returns early on a bus that is running. Giving
 * one driver another pin without the other therefore does not fail here, it halts the chip.
 */
StdReturnType I2cBus::Open(byte pin_sda, byte pin_scl, uint32_t frequency_hz) {
  Wire.setSDA(pin_sda);
  Wire.setSCL(pin_scl);

  Wire.begin();
  Wire.setClock(frequency_hz);
  return kEOk;
}
