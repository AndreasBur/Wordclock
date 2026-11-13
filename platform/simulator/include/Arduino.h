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

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_H_

// clang-format off
/*
 * Re-exported, all of them: this header exists to pull the pieces together, and what includes it
 * wants lives in them - `byte` above all. Without the pragma an include-cleaner credits `byte` to
 * arduino/Types.h, finds nothing of this header used directly, and offers to drop the include.
 * Taking that offer compiles, because the neighbouring headers include this one too - and leaves
 * the file resting on somebody else's include until the day that one goes.
 */
// IWYU pragma: begin_exports
#include "arduino/Types.h"      // byte, boolean  (before the sim headers!)

#include "sim/SerialShim.h"

#include "arduino/Progmem.h"    // PROGMEM, F, pgm_read_byte, memcpy_P
#include "arduino/Bits.h"       // bitRead
#include "arduino/Itoa.h"       // itoa()
// clang-format on
// IWYU pragma: end_exports

/*!
 * \brief The serial port, as Arduino's Serial: the one SerialShim, which writes into the window.
 *
 * Serial I/O goes to SerialShim, which writes it into the text controls of the PixelsFrame window
 * and hands typed lines back. Kept here rather than in a sub-header because it is specific to this
 * simulator, not to the Arduino API.
 *
 * On Arduino, Serial is an object of the core; here it is a macro for SerialShim::GetInstance(),
 * and it has only the functions the firmware calls, without begin().
 */
#define Serial SerialShim::GetInstance()  // NOLINT(readability-identifier-naming)

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_H_
