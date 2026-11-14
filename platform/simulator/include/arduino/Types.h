// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino types byte and boolean, for the simulator.
 *
 * Must be visible before Pixels.h (and its Pixel.h) are pulled in, because those headers use
 * `byte`. Arduino.h therefore includes this header first.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_TYPES_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_TYPES_H_

#include <stdint.h>

/*! \brief An unsigned 8-bit number, as Arduino's byte. */
using byte = uint8_t;  // NOLINT(readability-identifier-naming)
/*! \brief A truth value, as Arduino's boolean. */
using boolean = bool;  // NOLINT(readability-identifier-naming)

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_TYPES_H_
