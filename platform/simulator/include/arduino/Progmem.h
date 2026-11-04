// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino flash-memory accessors, as plain memory access for the simulator.
 *
 * On the simulator everything lives in normal RAM, so the flash accessors collapse to plain memory
 * access.
 */

#ifndef _ARDUINO_PROGMEM_H_
#define _ARDUINO_PROGMEM_H_

#include <string.h>

#define PROGMEM
#define F(x)                    x
#define pgm_read_byte(x)        (*(x)) // NOLINT(readability-identifier-naming)
#define memcpy_P(x, y, z)       memcpy((x), (y), (z)) // NOLINT(readability-identifier-naming)

#endif // _ARDUINO_PROGMEM_H_
