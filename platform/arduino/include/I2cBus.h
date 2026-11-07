// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The I2C bus the light sensor and the clock chip share, opened the way the backend's
 *        Arduino core opens it.
 *
 * Both drivers are the same on every Arduino core this repository builds for; how `Wire` takes
 * its pins and its clock is not. Each backend defines Open() in its own I2cBus.cpp, and that
 * difference is all the file holds.
 */

#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The I2C bus, as far as opening it goes. */
class I2cBus
{
  public:
    /*!
     * \brief Opens the bus on `pin_sda` and `pin_scl` at `frequency_hz`.
     *
     * Each driver calls it until it has succeeded once, so whichever runs first opens the bus and
     * the other finds it running. There is one bus, so both have to name the same pins.
     *
     * On the ESP32, `Wire.begin()` takes all three and answers whether the bus opened. On the
     * RP2350 the pins and the clock are set one call each and none of them answers, so the bus
     * counts as open once they have run; a pin the core will not take, or one the second driver
     * names differently on the running bus, ends in the SDK's `panic()`, which halts the chip.
     *
     * \param[in] pin_sda The pin of the data line.
     * \param[in] pin_scl The pin of the clock line.
     * \param[in] frequency_hz The clock of the bus, in Hz.
     * \return kEOk, or kENotOk if the core says the bus did not open - which the RP2350's never
     *         does.
     */
    static StdReturnType Open(byte pin_sda, byte pin_scl, uint32_t frequency_hz);
};

#endif // _I2C_BUS_H_
