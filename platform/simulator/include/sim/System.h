// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator stand-in for what the clock knows about its machine.
 *
 * None of it is real here: a simulator has no network to have an address on, and its heap is the
 * host's, where a number of free kibibytes says nothing about the firmware. The uptime is not asked
 * of this class at all: the core counts it in Uptime.
 *
 * Those answer kENotOk rather than a placeholder, which is also what makes the "no answer" path of
 * the status command reachable here - the same reasoning as the clock chip's absent reading.
 *
 * The three actions are refused for the same kind of reason. A restart would be the window closing
 * and coming back, which is what the person running the simulator does themselves; the other two
 * need a network.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SYSTEM_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SYSTEM_H_

#include "Arduino.h"
#include "StandardTypes.h"

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
 * Nothing of it is real in the simulator: every getter answers kENotOk or false, and every
 * procedure is refused.
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
  System() {}
  ~System() = default;

 public:
  /*! \brief Returns the one system. */
  static System& GetInstance() {
    static System singleton_instance;
    return singleton_instance;
  }

  /*
   * Not static, though nothing in them reads a member: these stand in for the backends the
   * clock really runs on, and there the same call reaches a peripheral through instance
   * state. A stand-in that answered the check would have a different API from the thing it
   * stands in for, which is the one property it exists to keep.
   */
  // NOLINTBEGIN(readability-convert-member-functions-to-static)
  /*!
   * \brief Gives the memory that is still free, in kibibytes.
   *
   * Refused in the simulator, whose heap is the host's.
   *
   * \param[out] free_memory The free memory, in kibibytes, rounded down.
   * \return kEOk, or kENotOk if the backend cannot tell; `free_memory` is then left as it was.
   */
  StdReturnType GetFreeMemoryInKibibytes(uint16_t& free_memory) const {
    UNUSED(free_memory);
    return kENotOk;
  }

  /*!
   * \brief Writes the address the clock answers on in its network.
   *
   * Refused in the simulator, which has no network.
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
   * Refused in the simulator, which has no network.
   *
   * \param[out] string A buffer of kSsidStringLength.
   * \return kEOk, or kENotOk if no network is configured; `string` is then an empty string.
   */
  StdReturnType GetNetworkSsid(char* string) const {
    string[0u] = STD_NULL_CHARACTER;
    return kENotOk;
  }

  /*! \brief Returns whether a network is configured for the clock to join; always false here. */
  bool IsNetworkConfigured() const { return false; }

  /*!
   * \brief Stores the network the clock is to join, and joins it.
   *
   * There is no network here to be on or to join, so the command that configures one answers
   * that it could not - which is the same answer a clock gives whose store refused the pair,
   * and the one the simulator has to be able to produce.
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
   * The simulator's server asks for none, and there is no password to store, so the answer is
   * the same "no" the network functions above give.
   *
   * \return False.
   */
  bool IsConsoleProtected() const { return false; }

  /*!
   * \brief Stores the password the web console asks for, or clears it for an empty one.
   *
   * Refused in the simulator, which keeps no password.
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
   * Does nothing in the simulator, which has no network.
   */
  void StartNetwork() {}

  /*!
   * \brief Writes the received signal strength, in dBm.
   *
   * Refused in the simulator, which has no network.
   *
   * \param[out] string A buffer of kLinkQualityStringLength.
   * \return kEOk, or kENotOk while the clock is on no network; `string` is then an empty string.
   */
  StdReturnType GetLinkQuality(char* string) const {
    string[0u] = STD_NULL_CHARACTER;
    return kENotOk;
  }

  /*!
   * \brief Asks for a restart of the controller.
   *
   * Refused in the simulator: a restart would be the window closing and coming back, which is
   * what the person running the simulator does themselves.
   *
   * \return kEOk if the restart was asked for, which the application carries out on its next tick;
   *         or kENotOk if the backend cannot restart, and then nothing happens.
   */
  StdReturnType Restart() { return kENotOk; }

  /*!
   * \brief Asks the network for the time again.
   *
   * Refused in the simulator, which has no network.
   *
   * \return kEOk if the time was asked for, or kENotOk if there is no network to ask over; the time
   *         is then left as it was.
   */
  StdReturnType ResynchroniseTime() { return kENotOk; }

  /*!
   * \brief Joins the network again.
   *
   * Refused in the simulator, which has no network.
   *
   * \return kEOk if an attempt to join was started, or kENotOk if none was.
   */
  StdReturnType ReconnectNetwork() { return kENotOk; }
  // NOLINTEND(readability-convert-member-functions-to-static)
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SYSTEM_H_
