// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The switch in the strip's supply, as a flag rather than a port.
 *
 * A desktop has no 5 V rail to cut, so what is left of the hardware is the one bit of state:
 * whether the supply is on. That is not a stub standing in for something missing - the sequence
 * that uses this switch is what has to be got right, and it is the same sequence on every backend.
 *
 * Reported as fitted, unlike the hardware backends, which come up with the switch declared absent
 * until a pin is chosen for them. Here there is no pin to choose and nothing to damage, so the
 * procedures answer for real and the tests can drive them.
 *
 * The supply starts on. On a board the gate's pull down leaves it off until Init() asks, and there
 * is nothing to hold here.
 */

#ifndef _POWER_SWITCH_H_
#define _POWER_SWITCH_H_

#include "StandardTypes.h"

/* PowerSwitch configuration parameter */
/*
 * Whether the switch is built at all. It is optional hardware - the board it is read from
 * says so and bridges the high side MOSFET when it is left off - so the firmware has to
 * answer for a clock that has none.
 */
#define POWER_SWITCH_IS_FITTED                          STD_ON

class PowerSwitch
{
  private:
    bool supply_on_{true};

    PowerSwitch() { }
    ~PowerSwitch() { }

  public:
    static PowerSwitch& GetInstance() {
        static PowerSwitch singleton_instance;
        return singleton_instance;
    }

    // get methods
    static constexpr bool IsFitted() { return POWER_SWITCH_IS_FITTED == STD_ON; }
    bool IsSupplyOn() const { return supply_on_; }

    // methods
    /*
     * Nothing to set up: there is no port to make an output, and the supply already reads
     * on. It exists so that the call in the application's start-up is the same one on every
     * backend.
     */
    void Init() { }

    void SwitchSupplyOn() { supply_on_ = true; }
    void SwitchSupplyOff() { supply_on_ = false; }
};

#endif // _POWER_SWITCH_H_
