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
/**     \file       main.cpp
 *      \brief      Entry point of the AVR Dx application
 *
 *      \details    The counterpart of the simulator's WordclockApp: bring the runtime up,
 *                  bring the firmware up, then tick it at the scheduler's interval.
 *
 *                  The interval is read from the scheduler rather than repeated here. Every
 *                  module's task cycle counts in it - animation speeds, the sensor's sampling
 *                  rate, the console's poll rate - so a tick at another rate would rescale
 *                  all of them at once, and silently.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"

#include "Pixels.h"
#include "Scheduler.h"
#include "System.h"
#include "WordclockConfiguration.h"
#include "WordclockMain.h"

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

WordclockMain Wordclock;

} // namespace

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  main()
******************************************************************************************************************************************************/
/*! \brief          Brings the clock up and runs it
 *  \details        Timed against an absolute wake-up rather than a delay after the work, so
 *                  that what a tick costs does not stretch the interval. A frame takes about
 *                  3.3 ms of the ten, and adding the two would run the clock a third slow.
******************************************************************************************************************************************************/
int main()
{
    initRuntime();

    Serial.begin(WORDCLOCK_SERIAL_BAUDRATE);
    Serial.println(F("Wordclock"));

    Wordclock.init();

    uint32_t NextTick = millis();

    for(;;) {
        /* Subtraction rather than a comparison of the two, so that the wrap of the
           millisecond counter passes without a tick that never comes due. */
        if(static_cast<int32_t>(millis() - NextTick) < 0) { continue; }

        NextTick += Scheduler::getTaskIntervalMs();

        Wordclock.task();

        /* After the firmware, not from inside its pixel writes: the strip gets whatever the
           buffer holds once the whole pass over the tasks is done. */
        Pixels::getInstance().render();

        /* Last, so a restart asked for by a command takes place once that command's answer
           has reached the wire. */
        System::getInstance().performPendingRestart();
    }
} /* main */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
