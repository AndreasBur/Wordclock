// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The port that switches the strip's 5 V.
 *
 * One port, and active high. It drives the gate of an N-channel small-signal MOSFET through 82 ohm,
 * whose drain pulls the gate of a P-channel high side down through 3k3; that MOSFET's source is on
 * the incoming 5 V and its drain feeds the strip. The N-channel stage is what makes a 3.3 V part
 * able to switch it at all: pulling the high side's gate to ground is easy, and pushing it back up
 * to 5 V is what this chip's output cannot do.
 *
 * The 100 kohm at that gate is why the polarity is worth having: while the port is an input, which
 * is what it is from reset until Init() runs, the stage is held off and the strip has no supply.
 * Nothing lights before the firmware says so - and nothing is left lit by a reset either.
 *
 * \attention Declared **not fitted** here, and the pin below is a placeholder. The switch is
 * optional hardware - the board this is read off bridges the high side MOSFET when it is left
 * unbuilt - so driving a port that may belong to something else is the one thing this must not do
 * on a board nobody has confirmed. Set POWER_SWITCH_IS_FITTED once the port is known, and check the
 * pin against the board first.
 *
 * The full circuit is in ../esp32/README.md under "Switching the strip's supply", where this
 * backend's hardware notes point for the data line too.
 */

#ifndef _POWER_SWITCH_H_
#define _POWER_SWITCH_H_

#include "StandardTypes.h"

/* PowerSwitch configuration parameter */
#define POWER_SWITCH_IS_FITTED                          STD_OFF
// Beside the strip's own GPIO 10. Any GPIO would do on this part - there is no range reserved
// for the flash chip to route around - so this is next to the one it belongs with.
#define POWER_SWITCH_PIN                                11u

class PowerSwitch
{
  private:
    /*
     * What the port was last driven to, rather than the port read back. A board with no
     * switch fitted never has it driven at all, and then this is not what the answer is
     * taken from - see IsSupplyOn().
     */
    bool supply_on_{false};

    PowerSwitch() { }
    ~PowerSwitch() { }

    static void DriveSupply(bool on);

  public:
    static PowerSwitch& GetInstance() {
        static PowerSwitch singleton_instance;
        return singleton_instance;
    }

    // get methods
    static constexpr bool IsFitted() { return POWER_SWITCH_IS_FITTED == STD_ON; }

    /*
     * A board built without the switch has the high side MOSFET bridged, so its strip is
     * powered and cannot be anything else. Answering from the flag there would report a
     * supply that is off while the LEDs are lit, which is worse than not having the switch.
     */
    bool IsSupplyOn() const {
        if(!IsFitted()) { return true; }

        return supply_on_;
    }

    // methods
    void Init();

    void SwitchSupplyOn() { DriveSupply(true); supply_on_ = true; }
    void SwitchSupplyOff() { DriveSupply(false); supply_on_ = false; }
};

#endif // _POWER_SWITCH_H_
