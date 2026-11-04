// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The supply port, driven through the port registers, see PowerSwitch.h.
 */

#include <avr/io.h>

#include "Arduino.h"
#include "PowerSwitch.h"

#define POWER_SWITCH_PIN_MASK           static_cast<uint8_t>(1u << POWER_SWITCH_PIN)

/*! \brief          Makes the port an output and brings the supply up
 *  \details        Switched on rather than left where the reset found it, because the reset
 *                  found it off: the pull down at the gate holds the stage off while the port
 *                  is still an input. A clock whose strip never gets its 5 V shows nothing at
 *                  all, so coming up means asking for it.
 *
 *                  Has to run before the strip's data line is set up, which is why the
 *                  application calls it first: a frame reaching DIN before the supply is up
 *                  pushes current into a dead rail through the LED's own protection diode.
 *
 *                  Does nothing where the switch is not fitted. The pin then belongs to
 *                  whatever else the board put there, and driving it would be the one way this
 *                  can do damage rather than nothing.
 */
void PowerSwitch::Init()
{
    if(!IsFitted()) { return; }

    POWER_SWITCH_PORT.DIRSET = POWER_SWITCH_PIN_MASK;
    SwitchSupplyOn();
}

/*! \brief          Puts the port high or low
 *  \details        Active high, so on is high. OUTSET and OUTCLR rather than a read, a mask
 *                  and a write back: the other pins of the port are none of this function's
 *                  business, and the pair does it in one instruction each without a window in
 *                  which an interrupt could write the register in between.
 */
void PowerSwitch::DriveSupply(bool on)
{
    if(!IsFitted()) { return; }

    if(on) { POWER_SWITCH_PORT.OUTSET = POWER_SWITCH_PIN_MASK; }
    else   { POWER_SWITCH_PORT.OUTCLR = POWER_SWITCH_PIN_MASK; }
}
