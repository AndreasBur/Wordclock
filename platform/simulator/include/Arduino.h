// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino API for the simulator, which the firmware includes as Arduino.h.
 *
 * It pulls the individual pieces together. Include order matters: arduino/Types.h defines `byte`,
 * which the sim headers depend on, so it comes first. Serial is bound last because it needs
 * SerialShim.
 */

#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include "arduino/Types.h"      // byte, boolean  (before the sim headers!)

#include "sim/SerialShim.h"

#include "arduino/Progmem.h"    // PROGMEM, F, pgm_read_byte, memcpy_P
#include "arduino/Bits.h"       // bitRead
#include "arduino/Itoa.h"       // Itoa()

/* -------------------------------------------------------------------------- */
/*  Serial → simulator window                                                 */
/* -------------------------------------------------------------------------- */
/*  Serial I/O goes to SerialShim, which writes it into the text controls of  */
/*  the Pixels window and hands typed lines back. Kept here rather than in a  */
/*  sub-header because it is specific to this simulator, not to the Arduino   */
/*  API.                                                                      */
/* -------------------------------------------------------------------------- */
#define Serial                  SerialShim::GetInstance() // NOLINT(readability-identifier-naming)

#endif // _ARDUINO_H_
