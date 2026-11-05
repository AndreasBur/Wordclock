// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The supply port, driven through the SDK's GPIO calls, see PowerSwitch.h.
 *
 * The SDK rather than the core's pinMode and digitalWrite, for the reason the strip's driver uses
 * it: this backend talks to the hardware directly everywhere else, and one pin reached through the
 * Arduino layer would be the odd one out.
 */

#include <hardware/gpio.h>

#include "Arduino.h"
#include "PowerSwitch.h"

void PowerSwitch::Init()
{
    if(!IsFitted()) { return; }

    gpio_init(POWER_SWITCH_PIN);
    gpio_set_dir(POWER_SWITCH_PIN, GPIO_OUT);
    SwitchSupplyOn();
}

/*
 * Puts the port high or low, and does nothing where the switch is not fitted. Active high,
 * so on is high - the level reaches the gate of the small-signal MOSFET, not the high side
 * one, which is why the sense is not inverted here.
 */
void PowerSwitch::DriveSupply(bool on)
{
    if(!IsFitted()) { return; }

    gpio_put(POWER_SWITCH_PIN, on);
}
