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
/**     \file       WordclockMain.cpp
 *      \brief      Adapts the ESP32 application to the firmware core
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "WordclockMain.h"

#include "Display.h"
#include "Persistence.h"
#include "PowerSwitch.h"
#include "RealTimeClock.h"

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Brings the display up
 *  \details        The simulator gets away without this call because its stand-ins need
 *                  no setting up, which is why nothing in the core makes it. On hardware
 *                  it is what claims the RMT channel for the strip's data pin, and
 *                  without it every show() is quietly dropped.
 *
 *                  The light sensor is not initialised here on purpose: Illuminance keeps
 *                  its BH1750 private and has no defined init(), so the driver sets itself
 *                  up from its first task().
******************************************************************************************************************************************************/
void WordclockMain::init()
{
    /* Before the display, and that order is the hardware's rather than a preference: the call
       below claims the strip's data line, and a frame reaching DIN before the 5 V is up pushes
       current into a dead rail through the LED's own protection diode. The reset leaves the
       supply off, so this is the call that asks for it. */
    PowerSwitch::getInstance().init();
    Display::getInstance().init();
    /* After the display, because restoring the brightness recalculates what reaches the
       LEDs, and before the first task, so the strip's first frame already carries the
       stored colour. */
    Persistence::getInstance().load();
} /* init */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void WordclockMain::task()
{
    RealTimeClock::getInstance().task();
    wcScheduler.task();
} /* task */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
