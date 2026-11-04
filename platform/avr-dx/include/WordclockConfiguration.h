// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What is set for this board rather than for the firmware.
 *
 * Only what the application itself needs. Anything a peripheral owns - the strip's pin, the bus
 * frequency, the chip addresses - is configured in the driver that owns it, so that a pin can be
 * moved without a second file having to agree.
 */

#ifndef _WORDCLOCK_CONFIGURATION_H_
#define _WORDCLOCK_CONFIGURATION_H_

// The same rate the other backends' consoles run at, so one terminal setting fits all
// three.
#define WORDCLOCK_SERIAL_BAUDRATE                       115200uL

#endif // _WORDCLOCK_CONFIGURATION_H_
