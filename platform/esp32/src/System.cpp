// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock knows about the machine it runs on.
 */

/* Ahead of System.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <WiFi.h>

#include <Preferences.h>

#include "System.h"
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
 * Beside the network's own keys, in the same namespace: this is how the clock is reached as
 * well, which is what that namespace holds and what keeps it out of the settings blob the
 * core persists.
 */
constexpr const char* kConsolePasswordKey{"console-pass"};
// Fixed, and printed in the 401's realm so nobody has to guess it. Not a secret: what is
// secret is the password beside it.
constexpr char kConsoleUserName[]{"wordclock"};

constexpr uint32_t kBytesPerKibibyte{1024u};

bool IsNetworkJoined()
{
    return WiFi.status() == WL_CONNECTED;
}

/*
 * Base64, written here rather than taken from the core's own. Two reasons, and the second is
 * the better one: the core offers only an encoder anyway, and its encoder answers an Arduino
 * String - a heap allocation on a path that runs once per HTTP request, for twenty bytes that
 * fit on the stack. Encoding is also all this needs, since what it compares against is what a
 * browser would have sent.
 *
 * Padding included, because a browser's header carries it and a comparison of the two has to
 * see the same string.
 */
void ToBase64(const char* plain, size_t length, char* target, size_t capacity)
{
    static constexpr char kAlphabet[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    size_t out = 0u;

    for(size_t in = 0u; in < length; in += 3u) {
        const uint32_t byte0 = static_cast<uint8_t>(plain[in]);
        const uint32_t byte1 = ((in + 1u) < length) ? static_cast<uint8_t>(plain[in + 1u]) : 0u;
        const uint32_t byte2 = ((in + 2u) < length) ? static_cast<uint8_t>(plain[in + 2u]) : 0u;
        const uint32_t triple = (byte0 << 16u) | (byte1 << 8u) | byte2;

        if((out + 4u) >= capacity) { break; }

        target[out++] = kAlphabet[(triple >> 18u) & 0x3Fu];
        target[out++] = kAlphabet[(triple >> 12u) & 0x3Fu];
        target[out++] = ((in + 1u) < length) ? kAlphabet[(triple >> 6u) & 0x3Fu] : '=';
        target[out++] = ((in + 2u) < length) ? kAlphabet[triple & 0x3Fu] : '=';
    }
    target[out] = STD_NULL_CHARACTER;
}

/*
 * Reads one stored string into a buffer that has room for it, terminator included. Only a
 * missing key, or one too long for the buffer, answers kENotOk: the framework's getString()
 * counts the terminator, so an empty value answers 1 and reads as stored - an open network's
 * empty pass phrase does. ReadStoredSsid() takes an empty name for none.
 */
StdReturnType ReadStoredString(const char* key, char* string, size_t capacity)
{
    string[0u] = STD_NULL_CHARACTER;

    Preferences store;
    if(!store.begin(kNetworkNamespace, true)) { return kENotOk; }

    const size_t length = store.getString(key, string, capacity);
    store.end();

    if(length == 0u) { return kENotOk; }

    return kEOk;
}

// What was compiled in, which is what a clock that was flashed with a WordclockSecrets.h
// still comes up with. Stored credentials win over it: they are the later word.
/*
 * The stored name, if there is one that names a network. Empty counts as none: the framework's
 * getString() answers an empty value with 1, as it counts the terminator, and a name stored
 * empty by a firmware before SetNetworkCredentials() removed it would otherwise be joined.
 */
StdReturnType ReadStoredSsid(char* ssid)
{
    if(ReadStoredString(kSsidKey, ssid, System::kSsidStringLength) == kENotOk) { return kENotOk; }
    if(ssid[0u] == STD_NULL_CHARACTER) { return kENotOk; }

    return kEOk;
}

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
    free_memory = static_cast<uint16_t>(ESP.getFreeHeap() / kBytesPerKibibyte);
    return kEOk;
}

StdReturnType System::GetNetworkAddress(char* string) const
{
    string[0u] = STD_NULL_CHARACTER;

    if(!IsNetworkJoined()) { return kENotOk; }

    return CopyIfItFits(WiFi.localIP().toString().c_str(), string, kAddressStringLength);
}

StdReturnType System::GetLinkQuality(char* string) const
{
    string[0u] = STD_NULL_CHARACTER;

    if(!IsNetworkJoined()) { return kENotOk; }

    itoa(static_cast<int>(WiFi.RSSI()), string, 10);
    return kEOk;
}

StdReturnType System::GetNetworkSsid(char* ssid) const
{
    if(ReadStoredSsid(ssid) == kEOk) { return kEOk; }

    if(IsSsidCompiledIn()) { return CopyIfItFits(WORDCLOCK_WIFI_SSID, ssid, kSsidStringLength); }

    ssid[0u] = STD_NULL_CHARACTER;
    return kENotOk;
}

bool System::IsNetworkConfigured() const
{
    char ssid[kSsidStringLength]{};

    return GetNetworkSsid(ssid) == kEOk;
}

StdReturnType System::SetNetworkCredentials(const char* ssid, const char* password)
{
    if((ssid == nullptr) || (password == nullptr)) { return kENotOk; }
    if(strlen(ssid) >= kSsidStringLength) { return kENotOk; }
    if(strlen(password) >= kPasswordStringLength) { return kENotOk; }

    Preferences store;
    if(!store.begin(kNetworkNamespace, false)) { return kENotOk; }

    /*
     * An empty name forgets the network: both keys go, as an empty value written would read
     * back as stored, and 13 -S -P joined a network with no name instead of opening the access
     * point. An empty pass phrase is kept, as it is an open network's.
     */
    bool stored;
    if(ssid[0u] == STD_NULL_CHARACTER) {
        store.remove(kSsidKey);
        store.remove(kPasswordKey);
        stored = !store.isKey(kSsidKey);
    } else {
        stored = (store.putString(kSsidKey, ssid) == strlen(ssid)) &&
                 (store.putString(kPasswordKey, password) == strlen(password));
    }
    store.end();

    if(!stored) { return kENotOk; }

    StartNetwork();
    return kEOk;
}

/*
 * Without the access point, "configurable at runtime" would still mean a cable for the first
 * time. A pass phrase shared by every clock of this firmware would protect nothing, and one
 * printed on the case is a support case of its own.
 */
void System::StartNetwork()
{
    char ssid[kSsidStringLength]{};
    char password[kPasswordStringLength]{};

    // The pass phrase from where the name came from: a stored one, which may be empty for an
    // open network, beside a stored name; the compiled-in one beside the compiled-in name.
    if(ReadStoredSsid(ssid) == kEOk) {
        (void)ReadStoredString(kPasswordKey, password, kPasswordStringLength);
    } else if(GetNetworkSsid(ssid) == kEOk) {
        (void)CopyIfItFits(WORDCLOCK_WIFI_PASSWORD, password, kPasswordStringLength);
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(SYSTEM_ACCESS_POINT_SSID);
        return;
    }

    WiFi.mode(WIFI_STA);
    // Reconnecting is left to the core, so a router that reboots does not take the clock's
    // time source with it for good.
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(WORDCLOCK_WIFI_MODEM_SLEEP);
    WiFi.begin(ssid, password);
}

void System::PerformPendingRestart()
{
    if(!restart_pending_) { return; }

    Serial.flush();
    ESP.restart();
}

/*
 * The same call the application makes at start-up rather than a reach into SNTP: installing
 * the servers again is what restarts the query, and it keeps the zone rule in one place - a
 * second installation of the servers alone would leave the rule to be repeated here.
 */
StdReturnType System::ResynchroniseTime()
{
    if(!IsNetworkJoined()) { return kENotOk; }

    configTzTime(WORDCLOCK_TIMEZONE, WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
    return kEOk;
}

/*
 * Meant to be refused where no network was ever configured, which is a clock running on the
 * serial line alone - there is nothing to reconnect to, and answering kEOk would say there
 * was. What it asks is narrower than that: see the warning in System.h.
 */
StdReturnType System::ReconnectNetwork()
{
    // A stored network as well as a compiled-in one: asked for the compiled-in SSID alone, as
    // this was, it refused a network entered with command 13.
    if(!IsNetworkConfigured()) { return kENotOk; }

    if(!WiFi.reconnect()) { return kENotOk; }

    return kEOk;
}

bool System::IsConsoleProtected() const
{
    char password[kPasswordStringLength]{};

    return ReadStoredString(kConsolePasswordKey, password, kPasswordStringLength) == kEOk;
}

/*
 * Encoded once and compared as a string, because the core here can encode base64 and not
 * decode it - so the cheaper direction is to build the value a browser would send and compare
 * that, rather than to unpack what it did send.
 */
bool System::IsConsoleCredentialValid(const char* credential) const
{
    if(credential == nullptr) { return false; }

    char password[kPasswordStringLength]{};
    if(ReadStoredString(kConsolePasswordKey, password, kPasswordStringLength) == kENotOk) { return true; }

    char expected[kPasswordStringLength + sizeof(kConsoleUserName) + 1u]{};
    const int length = snprintf(expected, sizeof(expected), "%s:%s", kConsoleUserName, password);
    if(length <= 0) { return false; }

    char encoded[(sizeof(expected) + 2u) / 3u * 4u + 1u]{};
    ToBase64(expected, static_cast<size_t>(length), encoded, sizeof(encoded));

    // Same length first, then the whole of it: strncmp on the shorter of two strings would
    // accept a prefix, which for a password is the one comparison that must not be lenient.
    if(strlen(credential) != strlen(encoded)) { return false; }

    return strcmp(credential, encoded) == 0;
}

StdReturnType System::SetConsolePassword(const char* password)
{
    if(password == nullptr) { return kENotOk; }
    if(strlen(password) >= kPasswordStringLength) { return kENotOk; }

    Preferences store;
    if(!store.begin(kNetworkNamespace, false)) { return kENotOk; }

    // An empty password removes the key rather than storing nothing under it, so that
    // IsConsoleProtected() has one state to recognise instead of two.
    const bool stored = (password[0u] == STD_NULL_CHARACTER)
                      ? store.remove(kConsolePasswordKey)
                      : (store.putString(kConsolePasswordKey, password) == strlen(password));
    store.end();

    return stored ? kEOk : kENotOk;
}
// NOLINTEND(readability-convert-member-functions-to-static, readability-make-member-function-const)
