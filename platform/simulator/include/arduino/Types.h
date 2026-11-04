// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino types byte and boolean, for the simulator.
 *
 * Must be visible before Pixels.h (and its Pixel.h) are pulled in, because those headers use `byte`.
 * Arduino.h therefore includes this header first.
 */

#ifndef _ARDUINO_TYPES_H_
#define _ARDUINO_TYPES_H_

#include <stdint.h>

typedef uint8_t byte; // NOLINT(readability-identifier-naming)
typedef bool    boolean; // NOLINT(readability-identifier-naming)

#endif // _ARDUINO_TYPES_H_
