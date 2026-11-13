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

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_POWERSWITCH_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_POWERSWITCH_H_

#include "StandardTypes.h"

/*!
 * \brief STD_ON where the board has the switch, STD_OFF where it has none.
 *
 * Whether the switch is built at all. It is optional hardware - the board it is read from says so
 * and bridges the high side MOSFET when it is left off - so the firmware has to answer for a clock
 * that has none.
 */
#define POWER_SWITCH_IS_FITTED STD_ON

/*!
 * \brief The switch in the strip's 5 V supply.
 *
 * The core's Power switches the supply off and on through it, in the order the strip needs, and
 * asks it whether the supply is on. A board without the switch has its strip powered all the
 * time, and says so.
 *
 * The simulator has no supply to switch. What is left is a flag, which starts on and which the
 * two switches set.
 */
class PowerSwitch {
 private:
  bool supply_on_{true};

  PowerSwitch() {}
  ~PowerSwitch() = default;

 public:
  /*! \brief Returns the one switch. */
  static PowerSwitch& GetInstance() {
    static PowerSwitch singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns whether the board has the switch, as POWER_SWITCH_IS_FITTED says. */
  static constexpr bool IsFitted() { return POWER_SWITCH_IS_FITTED == STD_ON; }

  /*!
   * \brief Returns whether the strip's supply is on; always true where no switch is fitted.
   *
   * In the simulator, the flag the two switches set, which starts true.
   *
   * \return True while the supply is on.
   */
  bool IsSupplyOn() const { return supply_on_; }

  /*!
   * \brief Sets the switch up and switches the supply on.
   *
   * The application calls it once at start, before the strip's data line is claimed.
   *
   * Nothing to set up: there is no port to make an output, and the supply already reads on. It
   * exists so that the call in the application's start-up is the same one on every backend. It
   * does nothing, so a supply switched off before it stays off.
   */
  void Init() {}

  /*! \brief Switches the strip's supply on. */
  void SwitchSupplyOn() { supply_on_ = true; }

  /*! \brief Switches the strip's supply off. */
  void SwitchSupplyOff() { supply_on_ = false; }
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_POWERSWITCH_H_
