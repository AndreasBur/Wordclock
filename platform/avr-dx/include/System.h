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
 *                  calls answer kENotOk, which is not a stub standing in for something
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
// The buffer sizes the status command hands in. Kept identical to the other backends, so
// the command's own buffers do not have to change with the platform.
#define SYSTEM_ADDRESS_STRING_LENGTH                    16u
#define SYSTEM_LINK_QUALITY_STRING_LENGTH               5u
#define SYSTEM_SSID_STRING_LENGTH                       33u
#define SYSTEM_PASSWORD_STRING_LENGTH                   64u

/* System parameter */

/******************************************************************************************************************************************************
 *  C L A S S   S Y S T E M
******************************************************************************************************************************************************/
class System
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    static constexpr byte kAddressStringLength{SYSTEM_ADDRESS_STRING_LENGTH};
    static constexpr byte kLinkQualityStringLength{SYSTEM_LINK_QUALITY_STRING_LENGTH};
    static constexpr byte kSsidStringLength{SYSTEM_SSID_STRING_LENGTH};
    static constexpr byte kPasswordStringLength{SYSTEM_PASSWORD_STRING_LENGTH};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    bool restart_pending_{false};

    System() { }
    ~System() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static System& GetInstance() {
        static System singleton_instance;
        return singleton_instance;
    }

    // get methods
    StdReturnType GetFreeMemoryInKibibytes(uint16_t&) const;

    /*
     * Instance methods, though none of the stubs below reads a member: they are the
     * interface the firmware core calls through GetInstance(), and the backends that have a
     * network answer them from instance state. A backend without one keeps the shape.
     */
    // NOLINTBEGIN(readability-convert-member-functions-to-static)
    // No network, so no address, no name and no quality to report. kENotOk is the answer
    // the command is built to receive, not a gap.
    StdReturnType GetNetworkAddress(char* string) const { string[0u] = STD_NULL_CHARACTER; return kENotOk; }
    StdReturnType GetNetworkSsid(char* string) const { string[0u] = STD_NULL_CHARACTER; return kENotOk; }
    StdReturnType GetLinkQuality(char* string) const { string[0u] = STD_NULL_CHARACTER; return kENotOk; }
    bool IsNetworkConfigured() const { return false; }

    // set methods
    StdReturnType SetNetworkCredentials(const char* ssid, const char* password) { UNUSED(ssid); UNUSED(password); return kENotOk; }
    // No console to protect: this build has no web interface, so the password has nowhere to
    // be asked for and the answer is the same "no" the network methods above give.
    bool IsConsoleProtected() const { return false; }
    StdReturnType SetConsolePassword(const char* password) { UNUSED(password); return kENotOk; }


    // methods
    void StartNetwork() { }
    StdReturnType ReconnectNetwork() { return kENotOk; }

    // The clock chip is the time source here and needs no synchronising - it is asked
    // afresh once a second anyway.
    StdReturnType ResynchroniseTime() { return kENotOk; }
    // NOLINTEND(readability-convert-member-functions-to-static)

    // Only asks. The application carries it out on its next tick, once the command's answer
    // has reached the wire - a reset from here would cut the answer off mid-sentence.
    StdReturnType Restart() {
        restart_pending_ = true;
        return kEOk;
    }

    void PerformPendingRestart();
};

#endif // _SYSTEM_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
