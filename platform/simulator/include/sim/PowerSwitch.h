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
 *      \brief      The switch in the strip's supply, as a flag rather than a port
 *
 *      \details    A desktop has no 5 V rail to cut, so what is left of the hardware is the
 *                  one bit of state: whether the supply is on. That is not a stub standing in
 *                  for something missing - the sequence that uses this switch is what has to
 *                  be got right, and it is the same sequence on every backend.
 *
 *                  Reported as fitted, unlike the hardware backends, which come up with the
 *                  switch declared absent until a pin is chosen for them. Here there is no
 *                  pin to choose and nothing to damage, so the procedures answer for real and
 *                  the tests can drive them.
 *
 *                  The supply starts on. On a board the gate's pull down leaves it off until
 *                  init() asks, and there is nothing to hold here.
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
/* Whether the switch is built at all. It is optional hardware - the board it is read from
   says so and bridges the high side MOSFET when it is left off - so the firmware has to
   answer for a clock that has none. */
#define POWER_SWITCH_IS_FITTED                          STD_ON

/******************************************************************************************************************************************************
 *  C L A S S   P O W E R   S W I T C H
******************************************************************************************************************************************************/
class PowerSwitch
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    bool SupplyOn{true};

    PowerSwitch() { }
    ~PowerSwitch() { }

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
    bool isSupplyOn() const { return SupplyOn; }

    // methods
    /* Nothing to set up: there is no port to make an output, and the supply already reads
       on. It exists so that the call in the application's start-up is the same one on every
       backend. */
    void init() { }

    void switchSupplyOn() { SupplyOn = true; }
    void switchSupplyOff() { SupplyOn = false; }
};

#endif // _POWER_SWITCH_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
