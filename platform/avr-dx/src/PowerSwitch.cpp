// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The supply port, driven through the port registers, see PowerSwitch.h.
 */

#include <avr/io.h>

#include "Arduino.h"
#include "PowerSwitch.h"

/*! \brief The bit of POWER_SWITCH_PIN in the registers of POWER_SWITCH_PORT. */
#define POWER_SWITCH_PIN_MASK static_cast<uint8_t>(1u << POWER_SWITCH_PIN)

void PowerSwitch::Init() {
  if (!IsFitted()) {
    return;
  }

  POWER_SWITCH_PORT.DIRSET = POWER_SWITCH_PIN_MASK;
  SwitchSupplyOn();
}

/*
 * Puts the port high or low. Active high, so on is high. OUTSET and OUTCLR rather than a
 * read, a mask and a write back: the other pins of the port are none of this function's
 * business, and the pair does it in one instruction each without a window in which an
 * interrupt could write the register in between.
 */
void PowerSwitch::DriveSupply(bool on) {
  if (!IsFitted()) {
    return;
  }

  if (on) {
    POWER_SWITCH_PORT.OUTSET = POWER_SWITCH_PIN_MASK;
  } else {
    POWER_SWITCH_PORT.OUTCLR = POWER_SWITCH_PIN_MASK;
  }
}
