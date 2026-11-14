// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The supply port, driven through the SDK's GPIO calls, see PowerSwitch.h.
 *
 * The SDK rather than the core's pinMode and digitalWrite, as the strip's driver reaches its pin
 * through the SDK too.
 */

#include "PowerSwitch.h"

#include <hardware/gpio.h>

#include "Arduino.h"

void PowerSwitch::Init() {
  if (!IsFitted()) {
    return;
  }

  gpio_init(POWER_SWITCH_PIN);
  // GPIO_OUT is the SDK's own spelling of the direction, though its parameter is a bool.
  // NOLINTNEXTLINE(readability-implicit-bool-conversion)
  gpio_set_dir(POWER_SWITCH_PIN, GPIO_OUT);
  SwitchSupplyOn();
}

/*
 * Puts the port high or low, and does nothing where the switch is not fitted. Active high,
 * so on is high - the level reaches the gate of the small-signal MOSFET, not the high side
 * one, which is why the sense is not inverted here.
 */
void PowerSwitch::DriveSupply(bool on) {
  if (!IsFitted()) {
    return;
  }

  gpio_put(POWER_SWITCH_PIN, on);
}
