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
 *      \brief      Simulator stand-in for what the clock knows about its machine
 *
 *      \details    The uptime is real - it is the one of these a host can answer honestly.
 *                  The rest is not: a simulator has no network to have an address on, and
 *                  its heap is the host's, where a number of free kibibytes says nothing
 *                  about the firmware.
 *
 *                  Those answer E_NOT_OK rather than a placeholder, which is also what
 *                  makes the "no answer" path of the status command reachable here - the
 *                  same reasoning as the clock chip's absent reading.
 *
 *                  The three actions are refused for the same kind of reason. A restart
 *                  would be the window closing and coming back, which is what the person
 *                  running the simulator does themselves; the other two need a network.
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
/* System parameter */
#define SYSTEM_ADDRESS_STRING_LENGTH                    16u
#define SYSTEM_LINK_QUALITY_STRING_LENGTH               5u
#define SYSTEM_SSID_STRING_LENGTH                       33u
#define SYSTEM_PASSWORD_STRING_LENGTH                   64u

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
    StdReturnType getFreeMemoryInKibibytes(uint16_t& FreeMemory) const { UNUSED(FreeMemory); return E_NOT_OK; }

    StdReturnType getNetworkAddress(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }

    /* There is no network here to be on or to join, so the command that configures one
       answers that it could not - which is the same answer a clock gives whose store
       refused the pair, and the one the simulator has to be able to produce. */
    StdReturnType getNetworkSsid(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }
    bool isNetworkConfigured() const { return false; }
    StdReturnType setNetworkCredentials(const char* Ssid, const char* Password) { UNUSED(Ssid); UNUSED(Password); return E_NOT_OK; }
    /* No console to protect: this build has no web interface, so the password has nowhere to
       be asked for and the answer is the same "no" the network methods above give. */
    bool isConsoleProtected() const { return false; }
    StdReturnType setConsolePassword(const char* Password) { UNUSED(Password); return E_NOT_OK; }

    void startNetwork() { }
    StdReturnType getLinkQuality(char* String) const { String[0u] = STD_NULL_CHARACTER; return E_NOT_OK; }

    // methods
    StdReturnType restart() { return E_NOT_OK; }
    StdReturnType resynchroniseTime() { return E_NOT_OK; }
    StdReturnType reconnectNetwork() { return E_NOT_OK; }
};

#endif // _SYSTEM_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
