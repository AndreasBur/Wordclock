// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the clock knows about the machine it runs on.
 *
 * The things that are neither display nor sensor: what address it answers on, how much memory is
 * left, the network it joins and the console's password, and the three actions that only the
 * platform can carry out - restarting, asking the network for the time again, and joining the
 * network again. The uptime is not asked of this class at all: the core counts it in Uptime.
 *
 * Every getter answers with a return code rather than a value alone, because on every one of them
 * "there is no answer" is a real state: a clock with no network has no address and no link quality,
 * and a simulator has no heap worth reporting. A number that means "unknown" would be a number a
 * reader has to know about; a return code is one the compiler makes them handle.
 */

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief The length of a buffer for the network address, its terminator included.
 *
 * "255.255.255.255" and its terminator. IPv6 does not fit and is not offered: the clock joins a
 * home network over IPv4, and the field says what to type into a browser.
 */
#define SYSTEM_ADDRESS_STRING_LENGTH                    16u

/*!
 * \brief The length of a buffer for the link quality, its terminator included.
 *
 * "-100" and its terminator.
 */
#define SYSTEM_LINK_QUALITY_STRING_LENGTH               5u

/*!
 * \brief The length of a buffer for a network name, its terminator included.
 *
 * What 802.11 allows, 32 characters of network name, with room for the terminator, because the
 * name is handed over as a string.
 */
#define SYSTEM_SSID_STRING_LENGTH                       33u
/*!
 * \brief The length of a buffer for a pass phrase or a password, its terminator included.
 *
 * What 802.11 allows, 63 characters of a WPA2 pass phrase, with room for the terminator, because
 * the pass phrase is handed over as a string. The console's password is held to the same length.
 */
#define SYSTEM_PASSWORD_STRING_LENGTH                   64u

/*!
 * \brief The name of the open access point the clock opens while no network is configured.
 *
 * The network the clock opens when it has none to join, so that the console it is configured
 * through can be reached at all. Open, and only while there is nothing stored: a pass phrase every
 * clock of this firmware shares would protect nothing, and the window closes as soon as the first
 * network is entered.
 */
#define SYSTEM_ACCESS_POINT_SSID                        "Wordclock"

/*!
 * \brief What the clock knows about the machine it runs on, and what it can ask of it: the free
 *        memory, the network, the console's password and a restart.
 *
 * The status command, the network command, the console command and the procedures call it. Every
 * function that answers a StdReturnType may answer kENotOk, and a backend that cannot do a thing
 * has to: a clock with no network has no address, and the status command sends an empty field
 * rather than a zero.
 *
 * On the ESP32 the free memory is the free heap, and the network is WiFi. The network's name, its
 * pass phrase and the console's password are kept with Preferences in an NVS namespace of their
 * own, `wc-network`, apart from the settings the core persists, so that a settings reset leaves
 * them alone. While no network is configured, StartNetwork() opens the open access point
 * SYSTEM_ACCESS_POINT_SSID instead. A restart is asked for and carried out by
 * PerformPendingRestart() at the end of the application's tick.
 */
class System
{
  public:
    /*! \brief The length of the buffer GetNetworkAddress() writes to. */
    static constexpr byte kAddressStringLength{SYSTEM_ADDRESS_STRING_LENGTH};
    /*! \brief The length of the buffer GetLinkQuality() writes to. */
    static constexpr byte kLinkQualityStringLength{SYSTEM_LINK_QUALITY_STRING_LENGTH};
    /*! \brief The length of the buffer GetNetworkSsid() writes to. */
    static constexpr byte kSsidStringLength{SYSTEM_SSID_STRING_LENGTH};
    /*! \brief The length of a buffer for a pass phrase or for the console's password. */
    static constexpr byte kPasswordStringLength{SYSTEM_PASSWORD_STRING_LENGTH};

  private:
    /*
     * A restart that was asked for and not carried out yet. The command that asks for one
     * is answered first, and the answer travels over a UART and a web socket that a
     * controller restarting inside the command would cut off mid-line.
     */
    bool restart_pending_{false};

    System() { }
    ~System() { }

  public:
    /*! \brief Returns the one system. */
    static System& GetInstance() {
        static System singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Gives the memory that is still free, in kibibytes.
     *
     * On the ESP32, the free heap as `ESP.getFreeHeap()` reports it; never refused. It says whether
     * the clock is leaking, which is the one thing a device that runs for months has to be watched
     * for.
     *
     * \param[out] free_memory The free memory, in kibibytes, rounded down.
     * \return kEOk, or kENotOk if the backend cannot tell; `free_memory` is then left as it was.
     */
    StdReturnType GetFreeMemoryInKibibytes(uint16_t& free_memory) const;

    /*!
     * \brief Writes the address the clock answers on in its network.
     *
     * On the ESP32, the IPv4 address WiFi was given in the network it joined, the address to type
     * into a browser. The clock is on a network only while WiFi reports it joined: while it joins,
     * after it lost the network, and while the access point is open, this is refused. It is also
     * refused if the address does not fit `string`.
     *
     * \param[out] string A buffer of kAddressStringLength.
     * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
     */
    StdReturnType GetNetworkAddress(char* string) const;

    /*!
     * \brief Writes the received signal strength, in dBm.
     *
     * On the ESP32, `WiFi.RSSI()` in decimal, refused while WiFi does not report the network
     * joined. A string rather than a number because it is negative, and the answer format carries
     * unsigned values.
     *
     * \param[out] string A buffer of kLinkQualityStringLength.
     * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
     */
    StdReturnType GetLinkQuality(char* string) const;

    /*!
     * \brief Writes the name of the network the clock is configured for.
     *
     * On the ESP32, the name stored with SetNetworkCredentials(), or, where none is stored, the one
     * compiled in as WORDCLOCK_WIFI_SSID - in that order, because the stored one is the later word.
     * A compiled-in name too long for `ssid` is refused as well.
     *
     * The pass phrase is deliberately not readable: it goes in and is never handed back out.
     *
     * \param[out] ssid A buffer of kSsidStringLength.
     * \return kEOk, or kENotOk if no network is configured; `ssid` is then an empty string.
     */
    StdReturnType GetNetworkSsid(char* ssid) const;

    /*!
     * \brief Returns whether a network is configured for the clock to join.
     *
     * On the ESP32, whether GetNetworkSsid() names one: stored, or compiled in. It reads the store
     * on every call.
     *
     * \return True if a network is configured.
     */
    bool IsNetworkConfigured() const;

    /*!
     * \brief Stores the network the clock is to join, and joins it.
     *
     * On the ESP32 it refuses a null or too long name or pass phrase before storing anything, then
     * writes the name and the pass phrase to the store, and calls StartNetwork() once both are
     * written. An empty name removes the stored network instead, so that StartNetwork() joins the
     * compiled-in network or opens the access point: the way out of a typo that cannot be
     * corrected over the network it broke. An empty pass phrase is stored as it is, an open
     * network's.
     *
     * Kept apart from the settings the core persists: those are what the clock shows, this is how
     * it is reached, and a settings reset that threw the network away would leave a clock nobody
     * can talk to without a cable.
     *
     * \param[in] ssid The name of the network, terminated.
     * \param[in] password The pass phrase, terminated.
     * \return kEOk, or kENotOk if the pair was not stored; the clock then stays on the network it
     *         was on.
     * \warning The two are written one after the other. A store that fails after the name was
     *          written, or a power cut between the two writes, can leave the new name beside the
     *          old pass phrase.
     */
    StdReturnType SetNetworkCredentials(const char* ssid, const char* password);

    /*!
     * \brief Returns whether the web console asks for a password.
     *
     * On the ESP32, read from the store on every call rather than cached. A request is a network
     * round trip and this is a flash read of one short string, so a copy that could fall behind is
     * not worth having - and a password changed by command takes effect on the next request rather
     * than on the next restart.
     *
     * The password itself is never handed back out, the same way the WiFi pass phrase is not: what
     * a caller may learn is yes or no.
     *
     * \return True if a password is stored.
     */
    bool IsConsoleProtected() const;

    /*!
     * \brief Returns whether `credential` is what a browser sends for the console's password.
     *
     * The candidate is the base64 text out of an Authorization header, after `Basic `, not the
     * password - and that is a decision about flash rather than about layering. Basic sends
     * base64("user:password"), the ESP32 core can only *encode*, and encoding the expected value
     * once to compare strings is a decoder's worth of code less than decoding what arrived. The
     * user name is therefore fixed, `wordclock`, and is not a secret; the realm in the 401 says
     * which one to type. The stored password is read on every call.
     *
     * Not in the platform contract, and deliberately: the core has no business checking a
     * credential, it only needs to answer whether one is asked for. This is between System and
     * this backend's own WebInterface, which is why the RP2350's has a different shape - its
     * server library does the comparison itself and wants the password, not a blob.
     *
     * \param[in] credential The base64 text, with its padding, terminated.
     * \return True if it matches, or if no password is stored; false for a null `credential` or one
     *         that does not match.
     */
    bool IsConsoleCredentialValid(const char* credential) const;

    /*!
     * \brief Stores the password the web console asks for, or clears it for an empty one.
     *
     * On the ESP32 it refuses a null or too long password before touching the store. A password is
     * stored beside the network's name and pass phrase; an empty one removes it from the store.
     * Without one the console asks for nothing, which is the state a clock ships in, so an update
     * introduces no lock and the access point of a first setup still answers. A password that is
     * forgotten is undone from the serial line, which is the one way in that does not go through
     * the console.
     *
     * \param[in] password The password, terminated; an empty one clears it.
     * \return kEOk, or kENotOk if it was not stored; the console then asks for what it asked for
     *         before. On the ESP32 an empty `password` while none is stored answers kENotOk as
     *         well, as there is nothing to remove.
     */
    StdReturnType SetConsolePassword(const char* password);

    /*!
     * \brief Brings the network up as configured.
     *
     * On the ESP32 it joins the network GetNetworkSsid() names, with the pass phrase from the same
     * place - the stored one, an empty one for an open network included, beside a stored name, and
     * WORDCLOCK_WIFI_PASSWORD beside the compiled-in name: in station mode, with WiFi's
     * own reconnecting on and modem sleep as WORDCLOCK_WIFI_MODEM_SLEEP says. Where no network is
     * configured it opens the open access point SYSTEM_ACCESS_POINT_SSID instead, because the
     * console the clock is configured through is reachable over the network and nowhere else. It
     * returns at once; joining goes on in the background.
     *
     * The application calls it at start, and SetNetworkCredentials() after storing. Nothing is
     * printed from here: a line printed in the middle of a command lands inside that command's
     * answer, which is how this was found. The application says what happened instead.
     */
    void StartNetwork();

    /*!
     * \brief Asks for a restart of the controller.
     *
     * Only asks, and on the ESP32 always. PerformPendingRestart() carries it out at the end of the
     * application's tick, so that the answer to the command that asked for it still goes out: a
     * command asks within the tick, and the restart follows at its end; the firmware update asks
     * from the web server's task, and the restart follows at the end of the next tick.
     *
     * \return kEOk if the restart was asked for, which the application carries out at the end of
     *         its tick; or kENotOk if the backend cannot restart, and then nothing happens.
     */
    StdReturnType Restart() { restart_pending_ = true; return kEOk; }

    /*!
     * \brief Restarts the controller, if Restart() asked for it.
     *
     * The application calls it at the end of every tick, after everything that had something to
     * send has sent it. Serial is flushed first: the UART writes in the background, and a
     * controller that restarts with bytes still in that buffer cuts its own answer in half. The
     * restart is `ESP.restart()`, from which the call does not return.
     */
    void PerformPendingRestart();

    /*!
     * \brief Asks the network for the time again.
     *
     * On the ESP32 it installs the zone rule and the time servers again with `configTzTime()`, the
     * same call the application makes at start, which is what restarts the SNTP query. kEOk says
     * the query was started, not that an answer arrived. What it is for is the clock that came up
     * while the time server was unreachable.
     *
     * \return kEOk if the time was asked for, or kENotOk if there is no network to ask over; the
     *         time is then left as it was.
     */
    StdReturnType ResynchroniseTime();

    /*!
     * \brief Joins the network again.
     *
     * On the ESP32 it asks WiFi to reconnect with `WiFi.reconnect()`, for the clock whose access
     * point was away longer than WiFi's own retries lasted. Refused while no network is configured,
     * IsNetworkConfigured(), and when `WiFi.reconnect()` answers false.
     *
     * \return kEOk if an attempt to join was started, or kENotOk if none was.
     */
    StdReturnType ReconnectNetwork();
};

#endif // _SYSTEM_H_
