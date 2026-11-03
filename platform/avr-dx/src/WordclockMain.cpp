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
 *      \brief      The firmware, brought up and ticked, see WordclockMain.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"
#include "Display.h"
#include "Persistence.h"
#include "PowerSwitch.h"
#include "RealTimeClock.h"
#include "WordclockMain.h"

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/* An instance method, though it reads no member: it is part of the interface the
   firmware core calls through GetInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
/*! \brief          Brings the supply and the display up and restores what was stored
 *  \details        The supply switch before the display, and that order is the hardware's
 *                  rather than a preference: the display's Init() claims the strip's data
 *                  line, and a frame reaching DIN before the 5 V is up pushes current into a
 *                  dead rail through the LED's own protection diode. The reset leaves the
 *                  supply off, so this is the call that asks for it.
 *
 *                  The display next, because it is what claims the strip; the settings after
 *                  it, because restoring the brightness recalculates what reaches the LEDs,
 *                  and before the first task, so the strip's first frame already carries the
 *                  stored colour.
 *
 *                  The light sensor and the clock chip are not initialised here: the core
 *                  keeps both inside singletons of its own with no defined Init(), and each
 *                  driver sets itself up from its first task.
******************************************************************************************************************************************************/
void WordclockMain::Init()
{
    PowerSwitch::GetInstance().Init();
    Display::GetInstance().Init();
    Persistence::GetInstance().Load();
} /* Init */
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
/*! \brief          One pass over the firmware
 *  \details        The clock before the scheduler, so that everything the tasks read in this
 *                  pass sees one and the same time rather than one that changes half way
 *                  through it.
******************************************************************************************************************************************************/
void WordclockMain::Task()
{
    RealTimeClock::GetInstance().Task();
    wc_scheduler_.Task();
} /* Task */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
