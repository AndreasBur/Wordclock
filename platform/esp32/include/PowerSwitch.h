// Copyright 2017 Andreas Burnickl
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
 * The full circuit, and what to build around it, is in this backend's README.md under "Switching
 * the strip's supply".
 */

#ifndef WORDCLOCK_PLATFORM_ESP32_INCLUDE_POWERSWITCH_H_
#define WORDCLOCK_PLATFORM_ESP32_INCLUDE_POWERSWITCH_H_

#include "StandardTypes.h"

/*!
 * \brief STD_ON where the board has the switch, STD_OFF where it has none.
 *
 * Whether the switch is built at all. It is optional hardware - the board this is read off bridges
 * the high side MOSFET when it is left unbuilt - so the firmware has to answer for a clock that has
 * none. STD_OFF here until the pin is confirmed against the board, see the note on this file.
 */
#define POWER_SWITCH_IS_FITTED STD_OFF
/*!
 * \brief The GPIO the switch is on; a placeholder, see the note on this file.
 *
 * Beside the strip's own GPIO 10 and clear of the range this part reserves for its flash and
 * PSRAM, which starts at 26.
 */
#define POWER_SWITCH_PIN 11u

/*!
 * \brief The switch in the strip's 5 V supply.
 *
 * The core's Power switches the supply off and on through it, in the order the strip needs, and
 * asks it whether the supply is on. A board without the switch has its strip powered all the
 * time, and says so.
 *
 * On the ESP32 the switch is GPIO POWER_SWITCH_PIN, active high, driven through the Arduino core's
 * `pinMode()` and `digitalWrite()`, and it is declared not fitted.
 */
class PowerSwitch {
 private:
  /*
   * What the port was last driven to, rather than the port read back. A board with no
   * switch fitted never has it driven at all, and then this is not what the answer is
   * taken from - see IsSupplyOn().
   */
  bool supply_on_{false};

  PowerSwitch() {}
  ~PowerSwitch() = default;

  static void DriveSupply(bool on);

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
   * Where the switch is fitted, it answers what the port was last driven to: false until the
   * supply is first switched on.
   *
   * A board built without the switch has the high side MOSFET bridged, so its strip is powered
   * and cannot be anything else. Answering from the flag there would report a supply that is off
   * while the LEDs are lit, which is worse than not having the switch.
   *
   * \return True while the supply is on.
   */
  bool IsSupplyOn() const {
    if (!IsFitted()) {
      return true;
    }

    return supply_on_;
  }

  /*!
   * \brief Sets the switch up and switches the supply on.
   *
   * The application calls it once at start, before the strip's data line is claimed.
   *
   * Where the switch is fitted, it makes the port an output and switches the supply on, rather
   * than leaving it where the reset found it: the pull down at the gate holds the stage off
   * while the port is still an input, and a clock whose strip never gets its 5 V shows nothing
   * at all. It has to run before the strip's data line is set up, which is why the application
   * calls it first: a frame reaching DIN before the supply is up pushes current into a dead rail
   * through the LED's own protection diode.
   *
   * It does nothing where the switch is not fitted. The pin then belongs to whatever else the
   * board put there, and driving it would be the one way this can do damage rather than
   * nothing.
   */
  void Init();

  /*!
   * \brief Switches the strip's supply on.
   *
   * Where the switch is not fitted, it drives no port and only notes the state, which
   * IsSupplyOn() then does not read.
   */
  void SwitchSupplyOn() {
    DriveSupply(true);
    supply_on_ = true;
  }

  /*!
   * \brief Switches the strip's supply off.
   *
   * Where the switch is not fitted, it drives no port and only notes the state, which
   * IsSupplyOn() then does not read.
   */
  void SwitchSupplyOff() {
    DriveSupply(false);
    supply_on_ = false;
  }
};

#endif  // WORDCLOCK_PLATFORM_ESP32_INCLUDE_POWERSWITCH_H_
