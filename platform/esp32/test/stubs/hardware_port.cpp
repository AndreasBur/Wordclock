// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * The hardware port itself, plus the two C functions the core would provide.
 *
 * Deliberately kept out of any file that includes the platform's Arduino.h: there the name
 * Serial is the multiplexer, so an object of that name could not be defined. This file
 * includes the stub directly, by the path the build passes in.
 */
#include WORDCLOCK_CORE_ARDUINO_H

#include <stdio.h>

Stream Serial;

void configTzTime(char const*, char const*, char const*) {}

/*
 * The supply switch's two calls. Empty rather than recording: a host has no port to drive, and
 * what the tests ask about is the state the switch keeps, not the write that would have gone
 * out.
 */
void pinMode(uint8_t, uint8_t) {}
void digitalWrite(uint8_t, uint8_t) {}

char* itoa(int value, char* buffer, int) {
  // The caller's buffer, whose size itoa() is not told: 12 holds every int with its sign.
  snprintf(buffer, 12, "%d", value);  // NOLINT(runtime/printf)
  return buffer;
}
