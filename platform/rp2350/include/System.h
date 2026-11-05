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
 * On the RP2350 the free memory is the free heap, and the network is the WiFi of the Pico 2 W's
 * CYW43. The network's name, its pass phrase and the console's password are kept in LittleFS, one
 * file each in a directory of their own, `/wc-network`, apart from the settings the core persists,
 * so that a settings reset leaves them alone. The filesystem is mounted on the first call that
 * needs it, and only that once: if it does not mount, nothing reads as stored and nothing is
 * stored until the next start. While no network is configured, StartNetwork() opens the open
 * access point SYSTEM_ACCESS_POINT_SSID instead. A restart is asked for and carried out by
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
     * On the RP2350, the free heap as `rp2040.getFreeHeap()` reports it; never refused. It says
     * whether the clock is leaking, which is the one thing a device that runs for months has to be
     * watched for.
     *
     * \param[out] free_memory The free memory, in kibibytes, rounded down.
     * \return kEOk, or kENotOk if the backend cannot tell; `free_memory` is then left as it was.
     */
    StdReturnType GetFreeMemoryInKibibytes(uint16_t& free_memory) const;

    /*!
     * \brief Writes the address the clock answers on in its network.
     *
     * On the RP2350, the IPv4 address `WiFi.localIP()` gives, the address of the station
     * interface and the address to type into a browser. The clock counts as on a network while
     * `WiFi.status()` answers `WL_CONNECTED`; otherwise this is refused. It is also refused if the
     * address does not fit `string`.
     *
     * \param[out] string A buffer of kAddressStringLength.
     * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
     * \warning arduino-pico's `WiFi.status()` answers `WL_CONNECTED` also while the access point is
     *          open. This then answers kEOk with the address of the station interface, which has
     *          joined nothing, rather than refusing or naming the access point's address.
     */
    StdReturnType GetNetworkAddress(char* string) const;

    /*!
     * \brief Writes the received signal strength, in dBm.
     *
     * On the RP2350, `WiFi.RSSI()` in decimal, refused while `WiFi.status()` does not answer
     * `WL_CONNECTED`. A string rather than a number because it is negative, and the answer format
     * carries unsigned values.
     *
     * \param[out] string A buffer of kLinkQualityStringLength.
     * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
     * \warning While the access point is open, `WiFi.status()` answers `WL_CONNECTED` as well, so
     *          this answers kEOk with what `WiFi.RSSI()` reports for a station that has joined
     *          nothing.
     */
    StdReturnType GetLinkQuality(char* string) const;

    /*!
     * \brief Writes the name of the network the clock is configured for.
     *
     * On the RP2350, the name stored with SetNetworkCredentials(), or, where none is stored, the
     * one compiled in as WORDCLOCK_WIFI_SSID - in that order, because the stored one is the later
     * word. A name stored empty reads as none stored, and so does a filesystem that did not mount.
     * A compiled-in name too long for `ssid` is refused as well. It reads the store on every call.
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
     * On the RP2350, whether GetNetworkSsid() names one: stored, or compiled in. It reads the store
     * on every call.
     *
     * \return True if a network is configured.
     */
    bool IsNetworkConfigured() const;

    /*!
     * \brief Stores the network the clock is to join, and joins it.
     *
     * On the RP2350 it refuses a null or too long name or pass phrase before storing anything,
     * then writes the name and the pass phrase to their files, and calls StartNetwork() once both
     * are written. An empty name takes the stored network away: it is written as an empty file,
     * which GetNetworkSsid() reads as no name stored, so StartNetwork() then joins the compiled-in
     * network, or opens the access point where there is none. That is the way out of a typo that
     * cannot be corrected over the network it broke.
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
     * On the RP2350, read from the store on every call rather than cached. A request is a network
     * round trip and this is a small file, so a copy that could fall behind is not worth having -
     * and a password changed by command takes effect on the next request rather than on the next
     * restart. A filesystem that did not mount reads as no password.
     *
     * The password itself is never handed back out to the core, the same way the WiFi pass phrase
     * is not: what the core may learn is yes or no.
     *
     * \return True if a password is stored.
     */
    bool IsConsoleProtected() const;

    /*!
     * \brief Writes the console's password into `password`, for this backend's WebInterface.
     *
     * Not in the platform contract: the core has no business with the credential itself, and this
     * hands the password to this backend's own WebInterface because the server library there
     * compares it - where the ESP32's own check encodes and compares a blob. It reads the store on
     * every call.
     *
     * \param[out] password A buffer of `capacity` characters.
     * \param[in] capacity The length of `password`, its terminator included.
     * \return kEOk, or kENotOk if `password` is null, the filesystem did not mount, or no password
     *         is stored; `password` is then an empty string, unless it is null. A stored password
     *         longer than `capacity` - 1 is cut short to that, and still answers kEOk.
     * \warning A `capacity` of 0 is not checked: the terminator is then written to `password[0]`,
     *          and the read into `password` is not bounded.
     */
    StdReturnType GetConsolePassword(char* password, size_t capacity) const;

    /*!
     * \brief Stores the password the web console asks for, or clears it for an empty one.
     *
     * On the RP2350 it refuses a null or too long password before touching the store. A password is
     * stored beside the network's name and pass phrase; an empty one removes its file, and answers
     * kEOk also when there was none to remove. Without one the console asks for nothing, which is
     * the state a clock ships in, so an update introduces no lock and the access point of a first
     * setup still answers. A password that is forgotten is undone from the serial line, which is
     * the one way in that does not go through the console.
     *
     * \param[in] password The password, terminated; an empty one clears it.
     * \return kEOk, or kENotOk if it was not stored. For a null or too long `password`, a
     *         filesystem that did not mount, or a file that did not open, the console then asks for
     *         what it asked for before; a write that stops part way leaves the part that was
     *         written as the password, and an empty part as none.
     */
    StdReturnType SetConsolePassword(const char* password);

    /*!
     * \brief Brings the network up as configured.
     *
     * On the RP2350 it joins the network GetNetworkSsid() names, with the stored pass phrase, or
     * the compiled-in WORDCLOCK_WIFI_PASSWORD where none is stored: in station mode, in the
     * default low-power mode or none as WORDCLOCK_WIFI_MODEM_SLEEP says. Where no network is
     * configured it opens the open access point SYSTEM_ACCESS_POINT_SSID instead, because the
     * console the clock is configured through is reachable over the network and nowhere else.
     * It returns at once: `WiFi.mode()` is called first, which keeps arduino-pico's `WiFi.begin()`
     * from waiting for the join, and joining goes on in the background. What `WiFi.softAP()` and
     * `WiFi.begin()` answer is not checked.
     *
     * The application calls it at start, and SetNetworkCredentials() and ReconnectNetwork() call it
     * again. Nothing is printed from here: a line printed in the middle of a command lands inside
     * that command's answer, which is how this was found. The application says what happened
     * instead.
     *
     * \note There is no automatic reconnect on this core. A clock whose network went away joins it
     *       again only through ReconnectNetwork(), where the ESP32 recovers on its own.
     * \warning A pass phrase stored empty, as for an open network, reads as none stored, so a clock
     *          built with a WORDCLOCK_WIFI_PASSWORD joins the stored network with that compiled-in
     *          pass phrase instead of with none.
     */
    void StartNetwork();

    /*!
     * \brief Asks for a restart of the controller.
     *
     * Only asks, and on the RP2350 always. PerformPendingRestart() carries it out at the end of the
     * application's tick, so that the answer to the command that asked for it still goes out: a
     * command asks within the tick, and the restart follows at its end; the firmware update asks
     * from the web server's callback, and the restart follows at the end of the tick that runs
     * next.
     *
     * \return kEOk if the restart was asked for, which the application carries out at the end of
     *         its tick; or kENotOk if the backend cannot restart, and then nothing happens.
     */
    StdReturnType Restart() { restart_pending_ = true; return kEOk; }

    /*!
     * \brief Restarts the controller, if Restart() asked for it.
     *
     * The application calls it at the end of every tick, after everything that had something to
     * send has sent it. Serial is flushed first: the port writes in the background, and a
     * controller that restarts with bytes still in that buffer cuts its own answer in half. The
     * restart is `rp2040.restart()`, from which the call does not return.
     */
    void PerformPendingRestart();

    /*!
     * \brief Asks the network for the time again.
     *
     * On the RP2350 it starts the time servers again with wordclock_time::StartTimeServers(), the
     * same call the application makes once the link is up; the zone rule is already in place from
     * start. kEOk says the query was started, not that an answer arrived. What it is for is the
     * clock that came up while the time server was unreachable and has been showing nothing since.
     * The clock counts as on a network while `WiFi.status()` answers `WL_CONNECTED`.
     *
     * \return kEOk if the time was asked for, or kENotOk if there is no network to ask over; the
     *         time is then left as it was.
     * \warning While the access point is open, `WiFi.status()` answers `WL_CONNECTED` as well, so
     *          this answers kEOk and starts the time servers with no link to reach them over.
     */
    StdReturnType ResynchroniseTime();

    /*!
     * \brief Joins the network again.
     *
     * On the RP2350 it calls StartNetwork() again, for the clock that lost its network: this core
     * has no automatic reconnect, and joining again is what stands in for it. Refused unless
     * IsNetworkConfigured(), so credentials entered over the console count as well as compiled-in
     * ones. StartNetwork() reads the stored credentials on its way, so this also
     * picks up a network that was entered while the old one was still being joined.
     *
     * \return kEOk if an attempt to join was started, or kENotOk if none was.
     */
    StdReturnType ReconnectNetwork();
};

#endif // _SYSTEM_H_
