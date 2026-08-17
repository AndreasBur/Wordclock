/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       PowerSwitch.h
 *      \brief      The port that switches the strip's 5 V
 *
 *      \details    One port, and active high. It drives the gate of an N-channel small-signal
 *                  MOSFET through 82 ohm, whose drain pulls the gate of a P-channel high side
 *                  down through 3k3; that MOSFET's source is on the incoming 5 V and its drain
 *                  feeds the strip. The N-channel stage is what makes a 3.3 V part able to
 *                  switch it at all: pulling the high side's gate to ground is easy, and
 *                  pushing it back up to 5 V is what this chip's output cannot do.
 *
 *                  The 100 kohm at that gate is why the polarity is worth having: while the
 *                  port is an input, which is what it is from reset until init() runs, the
 *                  stage is held off and the strip has no supply. Nothing lights before the
 *                  firmware says so - and nothing is left lit by a reset either.
 *
 *      \attention  Declared **not fitted** here, and the pin below is a placeholder. The
 *                  switch is optional hardware - the board this is read off bridges the high
 *                  side MOSFET when it is left unbuilt - so driving a port that may belong to
 *                  something else is the one thing this must not do on a board nobody has
 *                  confirmed. Set POWER_SWITCH_IS_FITTED once the port is known, and check the
 *                  pin against the board first.
 *
 *                  The full circuit, and what to build around it, is in this backend's
 *                  README.md under "Switching the strip's supply".
 *
******************************************************************************************************************************************************/
#ifndef _POWER_SWITCH_H_
#define _POWER_SWITCH_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* PowerSwitch configuration parameter */
#define POWER_SWITCH_IS_FITTED                          STD_OFF
/* Beside the strip's own GPIO 10 and clear of the range this part reserves for its flash and
   PSRAM, which starts at 26. */
#define POWER_SWITCH_PIN                                11u

/******************************************************************************************************************************************************
 *  C L A S S   P O W E R   S W I T C H
******************************************************************************************************************************************************/
class PowerSwitch
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* What the port was last driven to, rather than the port read back. A board with no
       switch fitted never has it driven at all, and then this is not what the answer is
       taken from - see isSupplyOn(). */
    bool SupplyOn{false};

    PowerSwitch() { }
    ~PowerSwitch() { }

    static void driveSupply(bool On);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static PowerSwitch& getInstance() {
        static PowerSwitch SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr bool isFitted() { return POWER_SWITCH_IS_FITTED == STD_ON; }

    /* A board built without the switch has the high side MOSFET bridged, so its strip is
       powered and cannot be anything else. Answering from the flag there would report a
       supply that is off while the LEDs are lit, which is worse than not having the switch. */
    bool isSupplyOn() const {
        if(!isFitted()) { return true; }

        return SupplyOn;
    }

    // methods
    void init();

    void switchSupplyOn() { driveSupply(true); SupplyOn = true; }
    void switchSupplyOff() { driveSupply(false); SupplyOn = false; }
};

#endif // _POWER_SWITCH_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
