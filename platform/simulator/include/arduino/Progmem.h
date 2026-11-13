// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino flash-memory accessors, as plain memory access for the simulator.
 *
 * On the simulator everything lives in normal RAM, so the flash accessors collapse to plain memory
 * access.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_PROGMEM_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_PROGMEM_H_

#include <string.h>

/*!
 * \brief Puts a variable into flash, as Arduino's PROGMEM; here it is empty, and the variable stays
 *        in RAM.
 */
#define PROGMEM

/*!
 * \brief Keeps a string literal in flash, as Arduino's F(); here it is the literal itself.
 *
 * On Arduino the result is a `const __FlashStringHelper*`, which print() reads from flash; here it
 * is a `const char*`, which SerialShim::print() takes.
 *
 * \param[in] x The string literal.
 */
#define F(x) x

/*!
 * \brief Reads the byte at `x` in flash, as Arduino's pgm_read_byte(); here it reads what `x`
 *        points to.
 *
 * \param[in] x A pointer to the byte.
 * \warning On Arduino `x` is an address and one byte is read. Here `x` has to be a pointer, and
 *          what is read is one element of the type it points to, which is more than a byte if the
 *          type is wider.
 */
#define pgm_read_byte(x) (*(x))  // NOLINT(readability-identifier-naming)

/*!
 * \brief Copies `z` bytes from `y` in flash to `x` in RAM, as Arduino's memcpy_P(); here it is
 *        memcpy().
 *
 * \param[out] x Where the bytes go.
 * \param[in] y Where the bytes come from.
 * \param[in] z The number of bytes.
 */
#define memcpy_P(x, y, z) memcpy((x), (y), (z))  // NOLINT(readability-identifier-naming)

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_PROGMEM_H_
