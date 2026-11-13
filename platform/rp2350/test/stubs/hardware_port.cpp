// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * The hardware port itself, plus the one C function the core would provide.
 *
 * Deliberately kept out of any file that includes the platform's Arduino.h: there the name
 * Serial is the multiplexer, so an object of that name could not be defined. This file
 * includes the stub directly, by the path the build passes in.
 */
#include WORDCLOCK_CORE_ARDUINO_H

#include <hardware/gpio.h>
#include <stdio.h>

Stream Serial;

/*
 * The supply switch's three calls. Here rather than in pio_stubs.cpp, which frame_test.cpp
 * replaces with its own: every test links this file, and a switch that cannot link would take
 * the whole backend with it. Empty rather than recording, because a host has no port to drive
 * and what a case would ask about is the state PowerSwitch keeps.
 */
void gpio_init(uint) {}
void gpio_set_dir(uint, bool) {}
void gpio_put(uint, bool) {}

char* itoa(int value, char* buffer, int) {
  // The caller's buffer, whose size itoa() is not told: 12 holds every int with its sign.
  snprintf(buffer, 12, "%d", value);  // NOLINT(runtime/printf)
  return buffer;
}
