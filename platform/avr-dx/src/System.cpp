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
/**     \file       System.cpp
 *      \brief      The machine the clock runs on, see System.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <avr/io.h>

#include "Arduino.h"
#include "System.h"

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

/* Where the linker put the end of the statically allocated data, and where the allocator
   has grown to if it was ever used. Declared by avr-libc rather than defined here. */
extern "C" char __heap_start;
extern "C" char* __brkval;

constexpr uint16_t BytesPerKibibyte{1024u};

} // namespace

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/* Instance methods, though some of them read no member: they are the interface the
   firmware core calls through getInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface.
   The ones it would make const drive the hardware, which a const would deny. */
// NOLINTBEGIN(readability-convert-member-functions-to-static, readability-make-member-function-const)
/******************************************************************************************************************************************************
  getFreeMemoryInKibibytes()
******************************************************************************************************************************************************/
/*! \brief          What is left between the stack and everything below it
 *  \details        The gap the stack still has to grow into, measured from a local variable
 *                  because that is where the stack currently is. Nothing here allocates, so
 *                  the lower edge is normally the end of the static data; __brkval is
 *                  consulted anyway, so that the figure stays honest if something ever does.
 *
 *                  Reported in kibibytes because that is the seam's unit, which on a part
 *                  with sixteen of them costs the last three digits - enough to see the
 *                  headroom, not enough to watch it move.
******************************************************************************************************************************************************/
StdReturnType System::getFreeMemoryInKibibytes(uint16_t& FreeMemory) const
{
    char StackMark;
    /* As addresses rather than pointers: the stack and the heap are not one array, and
       comparing or subtracting pointers into two different objects is undefined - an
       optimiser is entitled to fold the comparison away. Integers have no such rule. */
    const uintptr_t StackAddress = reinterpret_cast<uintptr_t>(&StackMark);
    const uintptr_t LowerEdge = reinterpret_cast<uintptr_t>((__brkval == nullptr) ? &__heap_start : __brkval);

    if(StackAddress < LowerEdge) { return E_NOT_OK; }

    FreeMemory = static_cast<uint16_t>(static_cast<uint16_t>(StackAddress - LowerEdge) / BytesPerKibibyte);

    return E_OK;
} /* getFreeMemoryInKibibytes */


/******************************************************************************************************************************************************
  performPendingRestart()
******************************************************************************************************************************************************/
/*! \brief          Resets the controller, if a command asked for one
 *  \details        The console is drained first: the command that asked for this has already
 *                  printed its answer into the transmit queue, and resetting with the queue
 *                  still full would throw it away - the clock would come back up having
 *                  never confirmed what it was told to do.
******************************************************************************************************************************************************/
void System::performPendingRestart()
{
    if(!RestartPending) { return; }

    Serial.flush();

    /* The software reset request is protected, like every register that can change how the
       device runs. */
    _PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRST_bm);
} /* performPendingRestart */
// NOLINTEND(readability-convert-member-functions-to-static, readability-make-member-function-const)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
