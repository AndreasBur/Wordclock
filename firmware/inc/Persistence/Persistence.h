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
/**     \file       Persistence.h
 *      \brief      Keeps the configuration across a restart
 *
 *      \details    Settings used to live in RAM only, which a simulator never reveals -
 *                  it starts fresh anyway - but a wall clock does: configure it, unplug
 *                  it, and it came back on its defaults.
 *
 *                  What is stored is gathered from the modules that own it rather than
 *                  duplicated here, and it is written when it differs from what was
 *                  written last. That is the same shape as DisplayManager's word-set
 *                  latch, and it is chosen for the same reason: there is no way to forget
 *                  to mark something dirty, because nothing marks anything. The
 *                  brightness automatic cannot trigger a write either, since what it
 *                  computes is never stored back into the setting it computes from.
 *
 *                  Not stored, and deliberately: the time and the date, which come from
 *                  the network, and the overlay configuration including its text, which
 *                  needs a variable-length field the format does not have yet.
 *
 *                  The format itself - the struct, its version and its checksum - lives
 *                  in the source, so that adding a field cannot silently change what the
 *                  platforms are compiled against.
 *
******************************************************************************************************************************************************/
#ifndef _PERSISTENCE_H_
#define _PERSISTENCE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Persistence configuration parameter */
/* In scheduler ticks, so 200 * 10 ms = 2 s between checks. It is the write that is being
   rationed, not the check: a burst of commands within one period collapses into a single
   write, and a period in which nothing changed costs a struct comparison. The scheduler
   counts cycles in a byte, so this cannot be raised past 255. */
#define PERSISTENCE_TASK_CYCLE                          200u

/******************************************************************************************************************************************************
 *  C L A S S   P E R S I S T E N C E
******************************************************************************************************************************************************/
class Persistence
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{PERSISTENCE_TASK_CYCLE};

    Persistence() { }
    ~Persistence() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Persistence& getInstance() {
        static Persistence SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    // methods
    /* Called once by the platform's entry point, after the display is up: the settings it
       restores reach the display through the modules' own setters. */
    StdReturnType load();
    void task();

    /* Writes now rather than within the next period. What that period is there for is
       rationing a burst of commands, which is exactly what a caller asking for this is
       not doing - it is about to pull the plug. */
    StdReturnType save();

    /* Every module back to what it starts with, and the store emptied. Emptied rather
       than written with the defaults: an empty store is what a clock that was never
       configured has, and load() already treats the two the same. */
    StdReturnType reset();
};

#endif // _PERSISTENCE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
