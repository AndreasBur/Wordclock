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

/* An instance method, though it reads no member: it is part of the interface the
   firmware core calls through getInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
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
void WordclockMain::Init()
{
    /* Before the display, and that order is the hardware's rather than a preference: the call
       below claims the strip's data line, and a frame reaching DIN before the 5 V is up pushes
       current into a dead rail through the LED's own protection diode. The reset leaves the
       supply off, so this is the call that asks for it. */
    PowerSwitch::GetInstance().Init();
    Display::GetInstance().Init();
    /* After the display, because restoring the brightness recalculates what reaches the
       LEDs, and before the first task, so the strip's first frame already carries the
       stored colour. */
    Persistence::GetInstance().Load();
} /* init */
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void WordclockMain::Task()
{
    RealTimeClock::GetInstance().Task();
    wc_scheduler_.Task();
} /* task */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
