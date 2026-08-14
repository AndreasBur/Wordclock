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
 *      \brief      What the clock knows about the machine it runs on
 *
 *      \details    The things that are neither display nor sensor: how long it has been
 *                  running, what address it answers on, how much memory is left, and the
 *                  three actions that only the platform can carry out - restarting,
 *                  asking the network for the time again, and joining the network again.
 *
 *                  Every getter answers with a return code rather than a value alone,
 *                  because on every one of them "there is no answer" is a real state: a
 *                  clock with no network has no address and no link quality, and a
 *                  simulator has no heap worth reporting. A number that means "unknown"
 *                  would be a number a reader has to know about; a return code is one the
 *                  compiler makes them handle.
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
/* "255.255.255.255" and its terminator. IPv6 does not fit and is not offered: the clock
   joins a home network over IPv4, and the field says what to type into a browser. */
#define SYSTEM_ADDRESS_STRING_LENGTH                    16u

/* "-100" and its terminator. */
#define SYSTEM_LINK_QUALITY_STRING_LENGTH               5u

/* What 802.11 allows: 32 characters of network name, and 63 of a WPA2 pass phrase. Both
   with room for the terminator, because they are handed over as strings. */
#define SYSTEM_SSID_STRING_LENGTH                       33u
#define SYSTEM_PASSWORD_STRING_LENGTH                   64u

/* The network the clock opens when it has none to join, so that the console it is
   configured through can be reached at all. Open, and only while there is nothing stored:
   a pass phrase every clock of this firmware shares would protect nothing, and the window
   closes as soon as the first network is entered. */
#define SYSTEM_ACCESS_POINT_SSID                        "Wordclock"

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
    /* A restart that was asked for and not carried out yet. The command that asks for one
       is answered first, and the answer travels over a UART and a web socket that a
       controller restarting inside the command would cut off mid-line. */
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
    /* Minutes since the last start. In minutes because that is the unit the question is
       asked in - "has it been up since I flashed it" - and because a uint16 of them lasts
       forty-five days, where seconds would wrap in eighteen hours. */
    StdReturnType getUptimeInMinutes(uint16_t&) const;

    /* Kibibytes of free heap. Says whether the clock is leaking, which is the one thing a
       device that runs for months has to be watched for. */
    StdReturnType getFreeMemoryInKibibytes(uint16_t&) const;

    /* The address to type into a browser, into a buffer of AddressStringLength.
       E_NOT_OK while the clock has not joined a network, which is when there is none. */
    StdReturnType getNetworkAddress(char*) const;

    /* The received signal strength in dBm, into a buffer of LinkQualityStringLength. A
       string rather than a number because it is negative, and the answer format carries
       unsigned values. */
    StdReturnType getLinkQuality(char*) const;

    /* The network the clock is configured for, into a buffer of SsidStringLength.
       E_NOT_OK when there is none, which is what a clock that was never told answers - and
       what the access point stands in for. The pass phrase is deliberately not readable:
       it goes in and is never handed back out. */
    StdReturnType getNetworkSsid(char*) const;
    bool isNetworkConfigured() const;

    // set methods
    /* Stores what the clock is to join, and joins it. Kept apart from the settings the
       core persists: those are what the clock shows, this is how it is reached, and a
       settings reset that threw the network away would leave a clock nobody can talk to
       without a cable. */
    StdReturnType setNetworkCredentials(const char*, const char*);

    // methods
    /* Brings the network up as configured: the stored credentials, or the access point
       when there are none. Called once at start-up and again whenever the credentials
       change. */
    void startNetwork();

    /* Asks for a restart, which the application carries out on its next tick through
       performPendingRestart(). Deferred rather than done here so that the answer to the
       command that asked for it still goes out. */
    StdReturnType restart() { RestartPending = true; return E_OK; }

    /* Called from the application's tick, after everything that had something to send has
       sent it. Does not return when a restart was asked for. */
    void performPendingRestart();

    /* Asks the network for the time again. What it is for is the clock that came up while
       the time server was unreachable and has been showing nothing since. */
    StdReturnType resynchroniseTime();

    /* Joins the network again, for the clock whose access point was away longer than the
       core's own retries lasted. */
    StdReturnType reconnectNetwork();
};

#endif // _SYSTEM_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
