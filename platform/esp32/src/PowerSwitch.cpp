// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The supply port, driven through the core's own GPIO calls, see PowerSwitch.h.
 */

#include "Arduino.h"
#include "PowerSwitch.h"

void PowerSwitch::Init() {
  if (!IsFitted()) {
    return;
  }

  pinMode(POWER_SWITCH_PIN, OUTPUT);
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

  digitalWrite(POWER_SWITCH_PIN, on ? HIGH : LOW);
}
