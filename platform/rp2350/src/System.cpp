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
 *      \brief      What the clock knows about the machine it runs on
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of System.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <WiFi.h>

#include <LittleFS.h>
#include <WiFiNTP.h>

#include "System.h"
#include "TimeSync.h"
#include "WordclockConfiguration.h"

#include <string.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* Its own namespace, not the one the settings blob lives in: a settings reset must not
   take the network with it, or a clock is left that nobody can reach without a cable.
   Both are limited to 15 characters by NVS. */
constexpr const char* NetworkNamespace{"wc-network"};
constexpr const char* SsidKey{"ssid"};
constexpr const char* PasswordKey{"password"};
/* Two names, two separators and the terminator, which the longest of the pair fits in
   several times over. */
constexpr size_t CredentialPathLength{32u};

constexpr uint32_t BytesPerKibibyte{1024u};

bool isNetworkJoined()
{
    return WiFi.status() == WL_CONNECTED;
}

/* One file per key under a directory named after the namespace, which is what this core
   offers in place of the IDF's key-value store. The two names the RP2350 backend uses are
   kept so that both read the same way, and Storage does the same with its own pair. */
bool isCredentialStoreReady()
{
    static bool WasTried{false};
    static bool IsMounted{false};

    if(!WasTried) {
        WasTried = true;
        IsMounted = LittleFS.begin();
    }

    return IsMounted;
}

void buildCredentialPath(const char* Key, char* Path, size_t Capacity)
{
    snprintf(Path, Capacity, "/%s/%s", NetworkNamespace, Key);
}

/* Reads one stored string into a buffer that has room for it, terminator included. An
   empty answer and a missing key are the same thing here: a network nobody entered. */
StdReturnType readStoredString(const char* Key, char* String, size_t Capacity)
{
    String[0u] = STD_NULL_CHARACTER;

    if(!isCredentialStoreReady()) { return E_NOT_OK; }

    char Path[CredentialPathLength]{};
    buildCredentialPath(Key, Path, sizeof(Path));

    File Stored = LittleFS.open(Path, "r");
    if(!Stored) { return E_NOT_OK; }

    /* One short of the capacity, so the terminator this writes is always inside the
       buffer however long the file turned out to be. */
    const size_t Length = Stored.read(reinterpret_cast<uint8_t*>(String), Capacity - 1u);
    Stored.close();

    String[Length] = STD_NULL_CHARACTER;

    if(Length == 0u) { return E_NOT_OK; }

    return E_OK;
}

/* Replaces one stored string. Opened for writing rather than appended to, so a shorter
   pass phrase cannot leave the tail of a longer one behind it. */
bool writeStoredString(const char* Key, const char* String)
{
    if(!isCredentialStoreReady()) { return false; }

    char Path[CredentialPathLength]{};
    LittleFS.mkdir(NetworkNamespace);
    buildCredentialPath(Key, Path, sizeof(Path));

    File Stored = LittleFS.open(Path, "w");
    if(!Stored) { return false; }

    const size_t Length = strlen(String);
    const size_t Written = Stored.write(reinterpret_cast<const uint8_t*>(String), Length);
    Stored.close();

    return Written == Length;
}

/* What was compiled in, which is what a clock that was flashed with a WordclockSecrets.h
   still comes up with. Stored credentials win over it: they are the later word. */
constexpr bool isSsidCompiledIn()
{
    return sizeof(WORDCLOCK_WIFI_SSID) > 1u;
}

/* Copies into the caller's buffer only if the whole string fits, so a value that grew
   past what the field was sized for is reported as no value rather than as half of one. */
StdReturnType copyIfItFits(const char* Value, char* String, size_t Capacity)
{
    String[0u] = STD_NULL_CHARACTER;

    if(strlen(Value) >= Capacity) { return E_NOT_OK; }

    strcpy(String, Value);
    return E_OK;
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getFreeMemoryInKibibytes()
******************************************************************************************************************************************************/
StdReturnType System::getFreeMemoryInKibibytes(uint16_t& FreeMemory) const
{
    FreeMemory = static_cast<uint16_t>(rp2040.getFreeHeap() / BytesPerKibibyte);
    return E_OK;
} /* getFreeMemoryInKibibytes */


/******************************************************************************************************************************************************
  getNetworkAddress()
******************************************************************************************************************************************************/
/*! \brief          The address the clock answers on
 *
 *  \return         E_OK if the clock has joined a network and the address fits the buffer
******************************************************************************************************************************************************/
StdReturnType System::getNetworkAddress(char* String) const
{
    String[0u] = STD_NULL_CHARACTER;

    if(!isNetworkJoined()) { return E_NOT_OK; }

    return copyIfItFits(WiFi.localIP().toString().c_str(), String, AddressStringLength);
} /* getNetworkAddress */


/******************************************************************************************************************************************************
  getLinkQuality()
******************************************************************************************************************************************************/
/*! \brief          The received signal strength in dBm
 *
 *  \return         E_OK if the clock has joined a network
******************************************************************************************************************************************************/
StdReturnType System::getLinkQuality(char* String) const
{
    String[0u] = STD_NULL_CHARACTER;

    if(!isNetworkJoined()) { return E_NOT_OK; }

    itoa(static_cast<int>(WiFi.RSSI()), String, 10);
    return E_OK;
} /* getLinkQuality */



/******************************************************************************************************************************************************
  getNetworkSsid()
******************************************************************************************************************************************************/
/*! \brief          The network the clock is configured for
 *  \details        What was stored, and what was compiled in when nothing was stored -
 *                  in that order, because the stored one is the later word.
 *
 *  \return         E_OK if there is a network to name
******************************************************************************************************************************************************/
StdReturnType System::getNetworkSsid(char* Ssid) const
{
    if(readStoredString(SsidKey, Ssid, SsidStringLength) == E_OK) { return E_OK; }

    if(isSsidCompiledIn()) { return copyIfItFits(WORDCLOCK_WIFI_SSID, Ssid, SsidStringLength); }

    Ssid[0u] = STD_NULL_CHARACTER;
    return E_NOT_OK;
}  /* getNetworkSsid */


/******************************************************************************************************************************************************
  isNetworkConfigured()
******************************************************************************************************************************************************/
bool System::isNetworkConfigured() const
{
    char Ssid[SsidStringLength]{};

    return getNetworkSsid(Ssid) == E_OK;
} /* isNetworkConfigured */


/******************************************************************************************************************************************************
  setNetworkCredentials()
******************************************************************************************************************************************************/
/*! \brief          Stores what the clock is to join, and joins it
 *  \details        Both halves are written before anything is joined, so that a clock that
 *                  loses power in the middle of this comes back with either the old pair
 *                  or the new one rather than with one of each.
 *
 *                  An empty name is how a network is taken away again, which is what puts
 *                  the access point back and is the way out of a typo that cannot be
 *                  corrected over the network it broke.
 *
 *  \return         E_OK if the pair was stored
******************************************************************************************************************************************************/
StdReturnType System::setNetworkCredentials(const char* Ssid, const char* Password)
{
    if((Ssid == nullptr) || (Password == nullptr)) { return E_NOT_OK; }
    if(strlen(Ssid) >= SsidStringLength) { return E_NOT_OK; }
    if(strlen(Password) >= PasswordStringLength) { return E_NOT_OK; }

    if(!writeStoredString(SsidKey, Ssid)) { return E_NOT_OK; }
    if(!writeStoredString(PasswordKey, Password)) { return E_NOT_OK; }

    startNetwork();
    return E_OK;
} /* setNetworkCredentials */


/******************************************************************************************************************************************************
  startNetwork()
******************************************************************************************************************************************************/
/*! \brief          Brings the network up as configured
 *  \details        A clock with credentials joins that network. A clock without opens one
 *                  of its own instead, because the console it is configured through is
 *                  reachable over the network and nowhere else - without the access point,
 *                  "configurable at runtime" would still mean a cable for the first time.
 *
 *                  The access point is open, and closes as soon as a network is entered.
 *                  A pass phrase shared by every clock of this firmware would protect
 *                  nothing, and one printed on the case is a support case of its own.
 *
 *                  Nothing is printed from here. This runs from the command that changes
 *                  the credentials as well as from start-up, and a line printed in the
 *                  middle of a command lands inside that command\'s answer - which is how
 *                  this was found. The application says what happened instead.
******************************************************************************************************************************************************/
void System::startNetwork()
{
    char Ssid[SsidStringLength]{};
    char Password[PasswordStringLength]{};

    if(getNetworkSsid(Ssid) == E_NOT_OK) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(SYSTEM_ACCESS_POINT_SSID);
        return;
    }

    /* The stored pass phrase, or the compiled-in one when the name came from there. */
    if(readStoredString(PasswordKey, Password, PasswordStringLength) == E_NOT_OK) {
        copyIfItFits(WORDCLOCK_WIFI_PASSWORD, Password, PasswordStringLength);
    }

    WiFi.mode(WIFI_STA);
    /* Power saving is a mode on this core rather than a flag: the default one is the modem
       sleep the ESP32 backend asks for by name, and the aggressive one adds latency a
       console typed into would notice. */
    if(WORDCLOCK_WIFI_MODEM_SLEEP) { WiFi.defaultLowPowerMode(); }
    else                           { WiFi.noLowPowerMode(); }

    /* No setAutoReconnect() here: this core has none, and joining again is what stands in
       for it - see reconnectNetwork(), which the command calls and which this function is
       the whole of. A router that reboots therefore needs that command, where the ESP32
       recovers on its own. */
    WiFi.begin(Ssid, Password);
} /* startNetwork */


/******************************************************************************************************************************************************
  performPendingRestart()
******************************************************************************************************************************************************/
/*! \brief          Carries out a restart that was asked for
 *  \details        From the application's tick, which is after the command's answer has
 *                  been printed and after the web interface has sent it on. Serial is
 *                  flushed first: the UART writes in the background, and a controller that
 *                  restarts with bytes still in that buffer cuts its own answer in half.
******************************************************************************************************************************************************/
void System::performPendingRestart()
{
    if(!RestartPending) { return; }

    Serial.flush();
    rp2040.restart();
} /* performPendingRestart */


/******************************************************************************************************************************************************
  resynchroniseTime()
******************************************************************************************************************************************************/
/*! \brief          Asks the network for the time again
 *  \details        The same call the application makes at start-up rather than a reach
 *                  into SNTP: installing the servers again is what restarts the query, and
 *                  it keeps the zone rule in one place - a second installation of the
 *                  servers alone would leave the rule to be repeated here.
 *
 *  \return         E_OK if there is a network to ask over
******************************************************************************************************************************************************/
StdReturnType System::resynchroniseTime()
{
    if(!isNetworkJoined()) { return E_NOT_OK; }

    /* The rule is already in place from start-up; only the servers are asked again. */
    WordclockTime::startTimeServers();
    return E_OK;
} /* resynchroniseTime */


/******************************************************************************************************************************************************
  reconnectNetwork()
******************************************************************************************************************************************************/
/*! \brief          Joins the network again
 *  \details        Refused where no network was ever configured, which is a clock running
 *                  on the serial line alone - there is nothing to reconnect to, and
 *                  answering E_OK would say there was.
 *
 *  \return         E_OK if the attempt was started
******************************************************************************************************************************************************/
StdReturnType System::reconnectNetwork()
{
    /* Nothing configured is nothing to reconnect to. isNetworkConfigured() rather than the
       compiled-in name alone, because credentials entered over the console count too. */
    if(!isNetworkConfigured()) { return E_NOT_OK; }

    /* Joining again rather than a reconnect() this core does not have. startNetwork()
       re-reads the stored credentials on its way, so this also picks up a network that was
       entered while the old one was still being retried. */
    startNetwork();

    return E_OK;
} /* reconnectNetwork */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
