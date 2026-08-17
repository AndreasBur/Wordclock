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
/**     \file       TimeSync.h
 *      \brief      The zone rule and the NTP servers, which this core keeps apart
 *
 *      \details    The ESP32's configTzTime() does both in one call, and both main and the
 *                  resynchronise command use it. Here the rule is POSIX and the servers are
 *                  the core's NTP object, so the pair is written down once rather than in
 *                  the two places that would otherwise each do half of it.
 *
 *                  They also cannot be started at the same moment, which is the reason this
 *                  is a header of its own rather than two lines at each call site. The rule
 *                  can be applied before there is a network and has to be, because every
 *                  conversion in RealTimeClock is local time from the first tick. The
 *                  servers are resolved by name, so asking for them before the link is up
 *                  fails silently and leaves the clock without a time source until
 *                  something asks again.
 *
******************************************************************************************************************************************************/
#ifndef _TIME_SYNC_H_
#define _TIME_SYNC_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <WiFiNTP.h>

#include "WordclockConfiguration.h"

#include <stdlib.h>
#include <time.h>

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N S
******************************************************************************************************************************************************/
namespace WordclockTime {

/* Safe before the link is up, and needed there: until this runs, localtime() is UTC and
   the display would show the wrong hour for as long as that lasted. */
inline void applyTimeZone()
{
    setenv("TZ", WORDCLOCK_TIMEZONE, 1);
    tzset();
}

/* Only once there is a link. The core resolves the server names here rather than at the
   first poll, so a call made too early returns having started nothing. */
inline void startTimeServers()
{
    NTP.begin(WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
}

} // namespace WordclockTime

#endif // _TIME_SYNC_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
