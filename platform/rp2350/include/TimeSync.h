// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The zone rule and the NTP servers, which this core keeps apart.
 *
 * The ESP32's configTzTime() does both in one call, and both main and the resynchronise command use
 * it. Here the rule is POSIX and the servers are the core's NTP object, so the pair is written down
 * once rather than in the two places that would otherwise each do half of it.
 *
 * They also cannot be started at the same moment, which is the reason this is a header of its own
 * rather than two lines at each call site. The rule can be applied before there is a network and
 * has to be, because every conversion in RealTimeClock is local time from the first tick. The
 * servers are resolved by name, so asking for them before the link is up fails silently and leaves
 * the clock without a time source until something asks again.
 */

#ifndef _TIME_SYNC_H_
#define _TIME_SYNC_H_

// Ahead of WiFiNTP.h, which calls ::hostByName without including what declares it: the
// framework's header compiles only behind a WiFi.h that its includer brought along.
#include <WiFi.h>
#include <WiFiNTP.h>

#include "WordclockConfiguration.h"

#include <stdlib.h>
#include <time.h>

/*! \brief The two halves of the time synchronisation: the zone rule and the time servers. */
namespace wordclock_time {
/*!
 * \brief Installs the zone rule WORDCLOCK_TIMEZONE, so that the C library's local time follows it.
 *
 * It sets `TZ` and calls `tzset()`. Safe before the link is up, and needed there: until this runs,
 * localtime() is UTC and the display would show the wrong hour for as long as that lasted. setup()
 * calls it once.
 */
inline void ApplyTimeZone()
{
    setenv("TZ", WORDCLOCK_TIMEZONE, 1);
    tzset();
}

/*!
 * \brief Starts SNTP with the time servers WORDCLOCK_NTP_SERVER_PRIMARY and
 *        WORDCLOCK_NTP_SERVER_SECONDARY.
 *
 * It calls `NTP.begin()` with the two. The application calls it once, on the first tick that
 * finds the link up, and System::ResynchroniseTime() calls it again.
 *
 * \pre The link is up. The core resolves the server names here rather than at the first poll, so a
 *      call made too early returns having started nothing.
 */
inline void StartTimeServers()
{
    NTP.begin(WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
}

} // namespace wordclock_time

#endif // _TIME_SYNC_H_
