// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief I2cBus on the ESP32's Arduino core, whose `Wire.begin()` takes the pins and the clock in
 *        one call and says whether the bus opened.
 */

#include "I2cBus.h"

#include <Wire.h>

StdReturnType I2cBus::Open(byte pin_sda, byte pin_scl, uint32_t frequency_hz)
{
    return Wire.begin(pin_sda, pin_scl, frequency_hz) ? kEOk : kENotOk;
}
