// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock knows about the machine it runs on.
 */

/* Ahead of System.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <WiFi.h>

#include <LittleFS.h>
#include <WiFiNTP.h>

#include "System.h"
#include "TimeSync.h"
#include "WordclockConfiguration.h"

#include <string.h>

namespace {
/*
 * Its own namespace, not the one the settings blob lives in: a settings reset must not
 * take the network with it, or a clock is left that nobody can reach without a cable.
 * Both are limited to 15 characters by NVS.
 */
constexpr const char* kNetworkNamespace{"wc-network"};
constexpr const char* kSsidKey{"ssid"};
constexpr const char* kPasswordKey{"password"};
/*
 * Beside the network's own keys and in the same directory: this is how the clock is reached
 * as well, which is what that namespace holds and what keeps it out of the settings blob the
 * core persists.
 */
constexpr const char* kConsolePasswordKey{"console-pass"};
// Two names, two separators and the terminator, which the longest of the pair fits in
// several times over.
constexpr size_t kCredentialPathLength{32u};

constexpr uint32_t kBytesPerKibibyte{1024u};

bool IsNetworkJoined()
{
    return WiFi.status() == WL_CONNECTED;
}

/*
 * One file per key under a directory named after the namespace, which is what this core
 * offers in place of the IDF's key-value store. The two names the RP2350 backend uses are
 * kept so that both read the same way, and Storage does the same with its own pair.
 */
bool IsCredentialStoreReady()
{
    static bool was_tried{false};
    static bool is_mounted{false};

    if(!was_tried) {
        was_tried = true;
        is_mounted = LittleFS.begin();
    }

    return is_mounted;
}

void BuildCredentialPath(const char* key, char* path, size_t capacity)
{
    // The namespace and the keys are this file's own constants, and the longest pair fits
    // kCredentialPathLength with room to spare.
    (void)snprintf(path, capacity, "/%s/%s", kNetworkNamespace, key);
}

// Reads one stored string into a buffer that has room for it, terminator included. An
// empty answer and a missing key are the same thing here: a network nobody entered.
StdReturnType ReadStoredString(const char* key, char* string, size_t capacity)
{
    string[0u] = STD_NULL_CHARACTER;

    if(!IsCredentialStoreReady()) { return kENotOk; }

    char path[kCredentialPathLength]{};
    BuildCredentialPath(key, path, sizeof(path));

    File stored = LittleFS.open(path, "r");
    if(!stored) { return kENotOk; }

    // One short of the capacity, so the terminator this writes is always inside the
    // buffer however long the file turned out to be.
    const size_t length = stored.read(reinterpret_cast<uint8_t*>(string), capacity - 1u);
    stored.close();

    string[length] = STD_NULL_CHARACTER;

    if(length == 0u) { return kENotOk; }

    return kEOk;
}

// Replaces one stored string. Opened for writing rather than appended to, so a shorter
// pass phrase cannot leave the tail of a longer one behind it.
bool WriteStoredString(const char* key, const char* string)
{
    if(!IsCredentialStoreReady()) { return false; }

    char path[kCredentialPathLength]{};
    LittleFS.mkdir(kNetworkNamespace);
    BuildCredentialPath(key, path, sizeof(path));

    File stored = LittleFS.open(path, "w");
    if(!stored) { return false; }

    const size_t length = strlen(string);
    const size_t written = stored.write(reinterpret_cast<const uint8_t*>(string), length);
    stored.close();

    return written == length;
}

// What was compiled in, which is what a clock that was flashed with a WordclockSecrets.h
// still comes up with. Stored credentials win over it: they are the later word.
constexpr bool IsSsidCompiledIn()
{
    return sizeof(WORDCLOCK_WIFI_SSID) > 1u;
}

// Copies into the caller's buffer only if the whole string fits, so a value that grew
// past what the field was sized for is reported as no value rather than as half of one.
StdReturnType CopyIfItFits(const char* value, char* string, size_t capacity)
{
    string[0u] = STD_NULL_CHARACTER;

    const size_t length = strlen(value);
    if(length >= capacity) { return kENotOk; }

    memcpy(string, value, length + 1u);
    return kEOk;
}

} // namespace

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 * The ones it would make const drive the hardware, which a const would deny.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static, readability-make-member-function-const)
StdReturnType System::GetFreeMemoryInKibibytes(uint16_t& free_memory) const
{
    free_memory = static_cast<uint16_t>(rp2040.getFreeHeap() / kBytesPerKibibyte);
    return kEOk;
}

/*! \brief          The address the clock answers on
 *
 *  \return         kEOk if the clock has joined a network and the address fits the buffer
 */
StdReturnType System::GetNetworkAddress(char* string) const
{
    string[0u] = STD_NULL_CHARACTER;

    if(!IsNetworkJoined()) { return kENotOk; }

    return CopyIfItFits(WiFi.localIP().toString().c_str(), string, kAddressStringLength);
}

/*! \brief          The received signal strength in dBm
 *
 *  \return         kEOk if the clock has joined a network
 */
StdReturnType System::GetLinkQuality(char* string) const
{
    string[0u] = STD_NULL_CHARACTER;

    if(!IsNetworkJoined()) { return kENotOk; }

    itoa(static_cast<int>(WiFi.RSSI()), string, 10);
    return kEOk;
}

/*! \brief          The network the clock is configured for
 *  \details        What was stored, and what was compiled in when nothing was stored -
 *                  in that order, because the stored one is the later word.
 *
 *  \return         kEOk if there is a network to name
 */
StdReturnType System::GetNetworkSsid(char* ssid) const
{
    if(ReadStoredString(kSsidKey, ssid, kSsidStringLength) == kEOk) { return kEOk; }

    if(IsSsidCompiledIn()) { return CopyIfItFits(WORDCLOCK_WIFI_SSID, ssid, kSsidStringLength); }

    ssid[0u] = STD_NULL_CHARACTER;
    return kENotOk;
}

bool System::IsNetworkConfigured() const
{
    char ssid[kSsidStringLength]{};

    return GetNetworkSsid(ssid) == kEOk;
}

/*! \brief          Stores what the clock is to join, and joins it
 *  \details        Both halves are written before anything is joined, so that a clock that
 *                  loses power in the middle of this comes back with either the old pair
 *                  or the new one rather than with one of each.
 *
 *                  An empty name is how a network is taken away again, which is what puts
 *                  the access point back and is the way out of a typo that cannot be
 *                  corrected over the network it broke.
 *
 *  \return         kEOk if the pair was stored
 */
StdReturnType System::SetNetworkCredentials(const char* ssid, const char* password)
{
    if((ssid == nullptr) || (password == nullptr)) { return kENotOk; }
    if(strlen(ssid) >= kSsidStringLength) { return kENotOk; }
    if(strlen(password) >= kPasswordStringLength) { return kENotOk; }

    if(!WriteStoredString(kSsidKey, ssid)) { return kENotOk; }
    if(!WriteStoredString(kPasswordKey, password)) { return kENotOk; }

    StartNetwork();
    return kEOk;
}

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
 */
void System::StartNetwork()
{
    char ssid[kSsidStringLength]{};
    char password[kPasswordStringLength]{};

    if(GetNetworkSsid(ssid) == kENotOk) {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(SYSTEM_ACCESS_POINT_SSID);
        return;
    }

    /* The stored pass phrase, or the compiled-in one when the name came from there. */
    if(ReadStoredString(kPasswordKey, password, kPasswordStringLength) == kENotOk) {
        CopyIfItFits(WORDCLOCK_WIFI_PASSWORD, password, kPasswordStringLength);
    }

    WiFi.mode(WIFI_STA);
    /*
     * Power saving is a mode on this core rather than a flag: the default one is the modem
     * sleep the ESP32 backend asks for by name, and the aggressive one adds latency a
     * console typed into would notice.
     */
    if(WORDCLOCK_WIFI_MODEM_SLEEP) { WiFi.defaultLowPowerMode(); }
    else                           { WiFi.noLowPowerMode(); }

    /*
     * No setAutoReconnect() here: this core has none, and joining again is what stands in
     * for it - see ReconnectNetwork(), which the command calls and which this function is
     * the whole of. A router that reboots therefore needs that command, where the ESP32
     * recovers on its own.
     */
    WiFi.begin(ssid, password);
}

/*! \brief          Carries out a restart that was asked for
 *  \details        From the application's tick, which is after the command's answer has
 *                  been printed and after the web interface has sent it on. Serial is
 *                  flushed first: the UART writes in the background, and a controller that
 *                  restarts with bytes still in that buffer cuts its own answer in half.
 */
void System::PerformPendingRestart()
{
    if(!restart_pending_) { return; }

    Serial.flush();
    rp2040.restart();
}

/*! \brief          Asks the network for the time again
 *  \details        The same call the application makes at start-up rather than a reach
 *                  into SNTP: installing the servers again is what restarts the query, and
 *                  it keeps the zone rule in one place - a second installation of the
 *                  servers alone would leave the rule to be repeated here.
 *
 *  \return         kEOk if there is a network to ask over
 */
StdReturnType System::ResynchroniseTime()
{
    if(!IsNetworkJoined()) { return kENotOk; }

    /* The rule is already in place from start-up; only the servers are asked again. */
    wordclock_time::StartTimeServers();
    return kEOk;
}

/*! \brief          Joins the network again
 *  \details        Refused where no network was ever configured, which is a clock running
 *                  on the serial line alone - there is nothing to reconnect to, and
 *                  answering kEOk would say there was.
 *
 *  \return         kEOk if the attempt was started
 */
StdReturnType System::ReconnectNetwork()
{
    // Nothing configured is nothing to reconnect to. IsNetworkConfigured() rather than the
    // compiled-in name alone, because credentials entered over the console count too.
    if(!IsNetworkConfigured()) { return kENotOk; }

    /*
     * Joining again rather than a reconnect() this core does not have. StartNetwork()
     * re-reads the stored credentials on its way, so this also picks up a network that was
     * entered while the old one was still being retried.
     */
    StartNetwork();

    return kEOk;
}

/*! \brief          The password the console asks for
 *
 *  \details        Handed out rather than compared here, because the server library on this
 *                  backend does the comparison itself - which is the whole reason this pair
 *                  looks different from the ESP32's, where the check is a base64 compare in
 *                  System because that core cannot decode.
 *
 *                  Read from the filesystem on every request rather than cached: a request is
 *                  a network round trip and this is a small file, so a copy that could fall
 *                  behind is not worth having, and a password changed by command takes effect
 *                  on the next request instead of on the next restart.
 */
bool System::IsConsoleProtected() const
{
    char password[kPasswordStringLength]{};

    return ReadStoredString(kConsolePasswordKey, password, kPasswordStringLength) == kEOk;
}

StdReturnType System::GetConsolePassword(char* password, size_t capacity) const
{
    if(password == nullptr) { return kENotOk; }

    return ReadStoredString(kConsolePasswordKey, password, capacity);
}

StdReturnType System::SetConsolePassword(const char* password)
{
    if(password == nullptr) { return kENotOk; }
    if(strlen(password) >= kPasswordStringLength) { return kENotOk; }

    /*
     * An empty password removes the file rather than writing an empty one, so that
     * IsConsoleProtected() has one state to recognise instead of two - and ReadStoredString()
     * answers kENotOk for an empty file as well as for a missing one, which would otherwise
     * make the two indistinguishable by accident rather than on purpose.
     */
    if(password[0u] == STD_NULL_CHARACTER) {
        if(!IsCredentialStoreReady()) { return kENotOk; }

        char path[kCredentialPathLength]{};
        BuildCredentialPath(kConsolePasswordKey, path, sizeof(path));
        // A file that was never there is the state being asked for, so its absence is not a
        // failure to report.
        LittleFS.remove(path);
        return kEOk;
    }

    return WriteStoredString(kConsolePasswordKey, password) ? kEOk : kENotOk;
}
// NOLINTEND(readability-convert-member-functions-to-static, readability-make-member-function-const)
