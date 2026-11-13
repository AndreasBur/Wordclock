// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Settings of the RP2350 application.
 *
 * Everything here is site-specific rather than firmware behaviour: the network to fall back on,
 * where the time comes from, which zone it is shown in.
 *
 * Each setting can be overridden from outside, either by a -D build flag or by a WordclockSecrets.h
 * next to this file, which is where compiled-in network credentials belong - that file is not
 * checked in. The credentials the network command stores in LittleFS win over the compiled-in
 * ones; these are what a clock with nothing stored joins.
 */

#ifndef WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKCONFIGURATION_H_
#define WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKCONFIGURATION_H_

#if defined(__has_include)
#if __has_include("WordclockSecrets.h")
#include "WordclockSecrets.h"
#endif
#endif

/*!
 * \brief The name of the network the clock joins while none is stored; empty for none.
 *
 * Empty rather than a placeholder network: a clock with nothing stored then opens its access point
 * and says so on the console, which still leaves a clock that answers commands and can be told its
 * network. A placeholder would instead spend every boot failing to join a network that never
 * existed.
 */
#ifndef WORDCLOCK_WIFI_SSID
#define WORDCLOCK_WIFI_SSID ""
#endif

/*!
 * \brief The pass phrase the clock joins with while none is stored; empty for an open network.
 *
 * A pass phrase stored empty counts as none stored, so this is also what the clock joins a stored
 * network with when its stored pass phrase is empty; see the warning at System::StartNetwork().
 */
#ifndef WORDCLOCK_WIFI_PASSWORD
#define WORDCLOCK_WIFI_PASSWORD ""
#endif

/*!
 * \brief The first time server SNTP asks.
 *
 * Two servers, because the network is the clock's one source of a synchronised time: if the first
 * one is unreachable the second one still gets the display right.
 */
#ifndef WORDCLOCK_NTP_SERVER_PRIMARY
#define WORDCLOCK_NTP_SERVER_PRIMARY "pool.ntp.org"
#endif

/*! \brief The second time server SNTP asks, for when the first one is unreachable. */
#ifndef WORDCLOCK_NTP_SERVER_SECONDARY
#define WORDCLOCK_NTP_SERVER_SECONDARY "time.cloudflare.com"
#endif

/*!
 * \brief The zone rule the time is shown in, as a POSIX TZ string.
 *
 * A POSIX zone rule rather than an offset, so the C library switches to summer time on
 * its own. Central Europe: UTC+1, one hour more from the last Sunday in March until the
 * last Sunday in October.
 */
#ifndef WORDCLOCK_TIMEZONE
#define WORDCLOCK_TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"
#endif

/*! \brief The baud rate setup() opens the serial console with. */
#ifndef WORDCLOCK_SERIAL_BAUDRATE
#define WORDCLOCK_SERIAL_BAUDRATE 115200
#endif

/*!
 * \brief True has WiFi join a network in its default low-power mode, false in none;
 *        System::StartNetwork() calls `WiFi.defaultLowPowerMode()` or `WiFi.noLowPowerMode()`.
 *
 * Modem sleep costs a few hundred milliseconds of network latency and saves around
 * 60 mA, which is more than the rest of the controller draws. A clock notices the one and
 * not the other.
 */
#ifndef WORDCLOCK_WIFI_MODEM_SLEEP
#define WORDCLOCK_WIFI_MODEM_SLEEP true
#endif

#endif  // WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKCONFIGURATION_H_
