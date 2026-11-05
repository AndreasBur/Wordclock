// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino bit helper bitRead(), for the simulator.
 */

#ifndef _ARDUINO_BITS_H_
#define _ARDUINO_BITS_H_

/*!
 * \brief Returns bit `bit` of `value`, 0 or 1, as Arduino's bitRead() does.
 *
 * \param[in] value The value.
 * \param[in] bit The number of the bit, counted from 0 at the least significant one.
 */
#define bitRead(value, bit)     (((value) >> (bit)) & 0x01) // NOLINT(readability-identifier-naming)

#endif // _ARDUINO_BITS_H_
