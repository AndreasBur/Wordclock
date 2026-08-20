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

#include <Preferences.h>

#include "System.h"
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
/* Beside the network's own keys, in the same namespace: this is how the clock is reached as
   well, which is what that namespace holds and what keeps it out of the settings blob the
   core persists. */
constexpr const char* ConsolePasswordKey{"console-pass"};
/* Fixed, and printed in the 401's realm so nobody has to guess it. Not a secret: what is
   secret is the password beside it. */
constexpr char ConsoleUserName[]{"wordclock"};

constexpr uint32_t BytesPerKibibyte{1024u};

bool isNetworkJoined()
{
    return WiFi.status() == WL_CONNECTED;
}

/* Reads one stored string into a buffer that has room for it, terminator included. An
   empty answer and a missing key are the same thing here: a network nobody entered. */
/* Base64, written here rather than taken from the core's own. Two reasons, and the second is
   the better one: the core offers only an encoder anyway, and its encoder answers an Arduino
   String - a heap allocation on a path that runs once per HTTP request, for twenty bytes that
   fit on the stack. Encoding is also all this needs, since what it compares against is what a
   browser would have sent.

   Padding included, because a browser's header carries it and a comparison of the two has to
   see the same string. */
void toBase64(const char* Plain, size_t Length, char* Target, size_t Capacity)
{
    static constexpr char Alphabet[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    size_t Out = 0u;

    for(size_t In = 0u; In < Length; In += 3u) {
        const uint32_t Byte0 = static_cast<uint8_t>(Plain[In]);
        const uint32_t Byte1 = ((In + 1u) < Length) ? static_cast<uint8_t>(Plain[In + 1u]) : 0u;
        const uint32_t Byte2 = ((In + 2u) < Length) ? static_cast<uint8_t>(Plain[In + 2u]) : 0u;
        const uint32_t Triple = (Byte0 << 16u) | (Byte1 << 8u) | Byte2;

        if((Out + 4u) >= Capacity) { break; }

        Target[Out++] = Alphabet[(Triple >> 18u) & 0x3Fu];
        Target[Out++] = Alphabet[(Triple >> 12u) & 0x3Fu];
        Target[Out++] = ((In + 1u) < Length) ? Alphabet[(Triple >> 6u) & 0x3Fu] : '=';
        Target[Out++] = ((In + 2u) < Length) ? Alphabet[Triple & 0x3Fu] : '=';
    }
    Target[Out] = STD_NULL_CHARACTER;
}


StdReturnType readStoredString(const char* Key, char* String, size_t Capacity)
{
    String[0u] = STD_NULL_CHARACTER;

    Preferences Store;
    if(!Store.begin(NetworkNamespace, true)) { return E_NOT_OK; }

    const size_t Length = Store.getString(Key, String, Capacity);
    Store.end();

    if(Length == 0u) { return E_NOT_OK; }

    return E_OK;
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
    FreeMemory = static_cast<uint16_t>(ESP.getFreeHeap() / BytesPerKibibyte);
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

    Preferences Store;
    if(!Store.begin(NetworkNamespace, false)) { return E_NOT_OK; }

    const bool Stored = (Store.putString(SsidKey, Ssid) == strlen(Ssid)) &&
                        (Store.putString(PasswordKey, Password) == strlen(Password));
    Store.end();

    if(!Stored) { return E_NOT_OK; }

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
    /* Reconnecting is left to the core, so a router that reboots does not take the clock's
       time source with it for good. */
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(WORDCLOCK_WIFI_MODEM_SLEEP);
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
    ESP.restart();
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

    configTzTime(WORDCLOCK_TIMEZONE, WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
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
    /* An unset SSID is a configuration that was never filled in, the same test the
       application makes before it joins at all. */
    if(sizeof(WORDCLOCK_WIFI_SSID) <= 1u) { return E_NOT_OK; }

    if(!WiFi.reconnect()) { return E_NOT_OK; }

    return E_OK;
} /* reconnectNetwork */

/******************************************************************************************************************************************************
  isConsoleProtected() / isConsoleCredentialValid() / setConsolePassword()
******************************************************************************************************************************************************/
/*! \brief          The password the console asks for, and whether what arrived matches
 *
 *  \details        Encoded once and compared as a string, because the core here can encode
 *                  base64 and not decode it - so the cheaper direction is to build the value
 *                  a browser would send and compare that, rather than to unpack what it did
 *                  send. The user name is fixed by the same decision and is not a secret.
 *
 *                  Read from the store on every request rather than cached. A request is a
 *                  network round trip and this is a flash read of sixty bytes, so the copy
 *                  that could fall behind is not worth having - and a password changed by
 *                  command takes effect on the next request rather than on the next restart.
******************************************************************************************************************************************************/
bool System::isConsoleProtected() const
{
    char Password[PasswordStringLength]{};

    return readStoredString(ConsolePasswordKey, Password, PasswordStringLength) == E_OK;
}


bool System::isConsoleCredentialValid(const char* Credential) const
{
    if(Credential == nullptr) { return false; }

    char Password[PasswordStringLength]{};
    if(readStoredString(ConsolePasswordKey, Password, PasswordStringLength) == E_NOT_OK) { return true; }

    char Expected[PasswordStringLength + sizeof(ConsoleUserName) + 1u]{};
    const int Length = snprintf(Expected, sizeof(Expected), "%s:%s", ConsoleUserName, Password);
    if(Length <= 0) { return false; }

    char Encoded[(sizeof(Expected) + 2u) / 3u * 4u + 1u]{};
    toBase64(Expected, static_cast<size_t>(Length), Encoded, sizeof(Encoded));

    /* Same length first, then the whole of it: strncmp on the shorter of two strings would
       accept a prefix, which for a password is the one comparison that must not be lenient. */
    if(strlen(Credential) != strlen(Encoded)) { return false; }

    return strcmp(Credential, Encoded) == 0;
}


StdReturnType System::setConsolePassword(const char* Password)
{
    if(Password == nullptr) { return E_NOT_OK; }
    if(strlen(Password) >= PasswordStringLength) { return E_NOT_OK; }

    Preferences Store;
    if(!Store.begin(NetworkNamespace, false)) { return E_NOT_OK; }

    /* An empty password removes the key rather than storing nothing under it, so that
       isConsoleProtected() has one state to recognise instead of two. */
    const bool Stored = (Password[0u] == STD_NULL_CHARACTER)
                      ? Store.remove(ConsolePasswordKey)
                      : (Store.putString(ConsolePasswordKey, Password) == strlen(Password));
    Store.end();

    return Stored ? E_OK : E_NOT_OK;
} /* setConsolePassword */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
