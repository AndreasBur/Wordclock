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
/**     \file       PowerSwitch.cpp
 *      \brief      The supply port, driven through the SDK's GPIO calls, see PowerSwitch.h
 *
 *      \details    The SDK rather than the core's pinMode and digitalWrite, for the reason the
 *                  strip's driver uses it: this backend talks to the hardware directly
 *                  everywhere else, and one pin reached through the Arduino layer would be the
 *                  odd one out.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <hardware/gpio.h>

#include "Arduino.h"
#include "PowerSwitch.h"

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
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
******************************************************************************************************************************************************/
void PowerSwitch::init()
{
    if(!isFitted()) { return; }

    gpio_init(POWER_SWITCH_PIN);
    gpio_set_dir(POWER_SWITCH_PIN, GPIO_OUT);
    switchSupplyOn();
} /* init */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  driveSupply()
******************************************************************************************************************************************************/
/*! \brief          Puts the port high or low
 *  \details        Active high, so on is high - the level reaches the gate of the small-signal
 *                  MOSFET, not the high side one, which is why the sense is not inverted here.
******************************************************************************************************************************************************/
void PowerSwitch::driveSupply(bool On)
{
    if(!isFitted()) { return; }

    gpio_put(POWER_SWITCH_PIN, On);
} /* driveSupply */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
