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
/**     \file       System.h
 *      \brief      The machine the clock runs on, as the status command asks about it
 *
 *      \details    Half of this seam is about a network, and this board has none. Those
 *                  calls answer E_NOT_OK, which is not a stub standing in for something
 *                  missing - it is the defined answer, and the status command sends an empty
 *                  field rather than a made-up one. The simulator answers the same way.
 *
 *                  What is left is real here: how long the clock has been running, how much
 *                  memory is still free, and the reset.
 *
******************************************************************************************************************************************************/
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* System configuration parameter */
/* The buffer sizes the status command hands in. Kept identical to the other backends, so
   the command's own buffers do not have to change with the platform. */
#define SYSTEM_ADDRESS_STRING_LENGTH                    16u
#define SYSTEM_LINK_QUALITY_STRING_LENGTH               5u
#define SYSTEM_SSID_STRING_LENGTH                       33u
#define SYSTEM_PASSWORD_STRING_LENGTH                   64u

/* System parameter */
#define SYSTEM_MILLISECONDS_PER_MINUTE                  60000uL

/******************************************************************************************************************************************************
 *  C L A S S   S Y S T E M
******************************************************************************************************************************************************/
class System
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    static constexpr byte AddressStringLength{SYSTEM_ADDRESS_STRING_LENGTH};
    static constexpr byte LinkQualityStringLength{SYSTEM_LINK_QUALITY_STRING_LENGTH};
    static constexpr byte SsidStringLength{SYSTEM_SSID_STRING_LENGTH};
    static constexpr byte PasswordStringLength{SYSTEM_PASSWORD_STRING_LENGTH};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Taken when the singleton is first asked for, which is the first status command rather
       than the start of the application - close enough for a field that answers "has this
       been running since I started it". */
    const uint32_t Started{millis()};

    bool RestartPending{false};

    System() { }
    ~System() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static System& getInstance() {
        static System SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    StdReturnType getUptimeInMinutes(uint16_t& Uptime) const {
        Uptime = static_cast<uint16_t>((millis() - Started) / SYSTEM_MILLISECONDS_PER_MINUTE);
        return E_OK;
    }

    StdReturnType getFreeMemoryInKibibytes(uint16_t&) const;

    /* No network, so no address, no name and no quality to report. E_NOT_OK is the answer
       the command is built to receive, not a gap. */
    StdReturnType getNetworkAddress(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }
    StdReturnType getNetworkSsid(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }
    StdReturnType getLinkQuality(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }
    bool isNetworkConfigured() const { return false; }

    // set methods
    StdReturnType setNetworkCredentials(const char* Ssid, const char* Password) { UNUSED(Ssid); UNUSED(Password); return E_NOT_OK; }

    // methods
    void startNetwork() { }
    StdReturnType reconnectNetwork() { return E_NOT_OK; }

    /* The clock chip is the time source here and needs no synchronising - it is asked
       afresh once a second anyway. */
    StdReturnType resynchroniseTime() { return E_NOT_OK; }

    /* Only asks. The application carries it out on its next tick, once the command's answer
       has reached the wire - a reset from here would cut the answer off mid-sentence. */
    StdReturnType restart() {
        RestartPending = true;
        return E_OK;
    }

    void performPendingRestart();
};

#endif // _SYSTEM_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
