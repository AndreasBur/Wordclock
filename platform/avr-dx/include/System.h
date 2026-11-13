// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The machine the clock runs on, as the status command asks about it.
 *
 * Half of this seam is about a network, and this board has none. Those calls answer kENotOk, which
 * is not a stub standing in for something missing - it is the defined answer, and the status
 * command sends an empty field rather than a made-up one. The simulator answers the same way.
 *
 * What is left is real here: how much memory is still free, and the reset. The uptime is not asked
 * of this class at all: the core counts it in Uptime.
 */

#ifndef WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_SYSTEM_H_
#define WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_SYSTEM_H_

#include "Arduino.h"
#include "StandardTypes.h"

// The buffer sizes the status command hands in. Kept identical to the other backends, so
// the command's own buffers do not have to change with the platform.
/*! \brief The length of a buffer for the network address, its terminator included. */
#define SYSTEM_ADDRESS_STRING_LENGTH 16u
/*! \brief The length of a buffer for the link quality, its terminator included. */
#define SYSTEM_LINK_QUALITY_STRING_LENGTH 5u
/*! \brief The length of a buffer for a network name, its terminator included. */
#define SYSTEM_SSID_STRING_LENGTH 33u
/*! \brief The length of a buffer for a pass phrase or a password, its terminator included. */
#define SYSTEM_PASSWORD_STRING_LENGTH 64u

/*!
 * \brief What the clock knows about the machine it runs on, and what it can ask of it: the free
 *        memory, the network, the console's password and a restart.
 *
 * The status command, the network command, the console command and the procedures call it. Every
 * function that answers a StdReturnType may answer kENotOk, and a backend that cannot do a thing
 * has to: a clock with no network has no address, and the status command sends an empty field
 * rather than a zero.
 *
 * On the AVR the free memory is real, and everything about a network and the console is refused:
 * this board has none. kENotOk is the answer the command is built to receive, not a gap. A
 * restart is asked for, and PerformPendingRestart() carries it out at the end of the tick that
 * asked.
 */
class System {
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
  bool restart_pending_{false};

  System() {}
  ~System() = default;

 public:
  /*! \brief Returns the one system. */
  static System& GetInstance() {
    static System singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Gives the memory that is still free, in kibibytes.
   *
   * On the AVR, what is left between the stack and everything below it: the gap the stack still
   * has to grow into, measured from a local variable because that is where the stack currently
   * is. Nothing here allocates, so the lower edge is normally the end of the static data;
   * `__brkval` is consulted anyway, so that the figure stays honest if something ever does. It is
   * refused if the stack lies below that edge.
   *
   * Reported in kibibytes because that is the seam's unit, which on a part with sixteen of them
   * costs the last three digits - enough to see the headroom, not enough to watch it move.
   *
   * \param[out] free_memory The free memory, in kibibytes, rounded down.
   * \return kEOk, or kENotOk if the backend cannot tell; `free_memory` is then left as it was.
   */
  StdReturnType GetFreeMemoryInKibibytes(uint16_t& free_memory) const;

  /*
   * Instance methods, though none of the stubs below reads a member: they are the
   * interface the firmware core calls through GetInstance(), and the backends that have a
   * network answer them from instance state. A backend without one keeps the shape.
   */
  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  /*!
   * \brief Writes the address the clock answers on in its network.
   *
   * Refused on the AVR, which has no network and so no address to report.
   *
   * \param[out] string A buffer of kAddressStringLength.
   * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
   */
  StdReturnType GetNetworkAddress(char* string) const {
    string[0u] = STD_NULL_CHARACTER;
    return kENotOk;
  }

  /*!
   * \brief Writes the name of the network the clock is configured for.
   *
   * Refused on the AVR, which has no network and so no name to report.
   *
   * \param[out] string A buffer of kSsidStringLength.
   * \return kEOk, or kENotOk if no network is configured; `string` is then an empty string.
   */
  StdReturnType GetNetworkSsid(char* string) const {
    string[0u] = STD_NULL_CHARACTER;
    return kENotOk;
  }

  /*!
   * \brief Writes the received signal strength, in dBm.
   *
   * Refused on the AVR, which has no network and so no quality to report.
   *
   * \param[out] string A buffer of kLinkQualityStringLength.
   * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
   */
  StdReturnType GetLinkQuality(char* string) const {
    string[0u] = STD_NULL_CHARACTER;
    return kENotOk;
  }

  /*! \brief Returns whether a network is configured for the clock to join; always false here. */
  bool IsNetworkConfigured() const { return false; }

  /*!
   * \brief Stores the network the clock is to join, and joins it.
   *
   * Refused on the AVR, which has no network to join.
   *
   * \param[in] ssid The name of the network, terminated.
   * \param[in] password The pass phrase, terminated.
   * \return kEOk, or kENotOk if the pair was not stored; the clock then stays on the network it was
   *         on.
   */
  StdReturnType SetNetworkCredentials(char const* ssid, char const* password) {
    UNUSED(ssid);
    UNUSED(password);
    return kENotOk;
  }

  /*!
   * \brief Returns whether the web console asks for a password; always false here.
   *
   * No console to protect: this build has no web interface, so the password has nowhere to be
   * asked for and the answer is the same "no" the network functions above give.
   *
   * \return False.
   */
  bool IsConsoleProtected() const { return false; }

  /*!
   * \brief Stores the password the web console asks for, or clears it for an empty one.
   *
   * Refused on the AVR, which has no web console.
   *
   * \param[in] password The password, terminated; an empty one clears it.
   * \return kEOk, or kENotOk if it was not stored; the console then asks for what it asked for
   *         before.
   */
  StdReturnType SetConsolePassword(char const* password) {
    UNUSED(password);
    return kENotOk;
  }

  /*!
   * \brief Brings the network up as configured.
   *
   * Does nothing on the AVR, which has no network.
   */
  void StartNetwork() {}

  /*!
   * \brief Joins the network again.
   *
   * Refused on the AVR, which has no network.
   *
   * \return kEOk if an attempt to join was started, or kENotOk if none was.
   */
  StdReturnType ReconnectNetwork() { return kENotOk; }

  /*!
   * \brief Asks the network for the time again.
   *
   * Refused on the AVR, which has no network. The clock chip is the time source here and needs
   * no synchronising - it is asked afresh about once a second anyway.
   *
   * \return kEOk if the time was asked for, or kENotOk if there is no network to ask over; the time
   *         is then left as it was.
   */
  StdReturnType ResynchroniseTime() { return kENotOk; }
  // NOLINTEND(readability-convert-member-functions-to-static)

  /*!
   * \brief Asks for a restart of the controller.
   *
   * Only asks, and on the AVR always. The application carries it out at the end of the tick that
   * asked, through PerformPendingRestart(), once the command's answer has reached the wire - a
   * reset from here would cut the answer off mid-sentence.
   *
   * \return kEOk if the restart was asked for, which the application carries out at the end of the
   *         tick that asked; or kENotOk if the backend cannot restart, and then nothing happens.
   */
  StdReturnType Restart() {
    restart_pending_ = true;
    return kEOk;
  }

  /*!
   * \brief Restarts the controller, if Restart() asked for it.
   *
   * The application calls it at the end of every tick. The console is drained first: the command
   * that asked for this has already printed its answer into the transmit queue, and resetting
   * with the queue still full would throw it away - the clock would come back up having never
   * confirmed what it was told to do. The restart is a software reset through `RSTCTRL.SWRR`,
   * from which the call does not return.
   */
  void PerformPendingRestart();
};

#endif  // WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_SYSTEM_H_
