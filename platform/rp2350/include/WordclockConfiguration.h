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
/**     \file       WordclockConfiguration.h
 *      \brief      Settings of the ESP32 application
 *
 *      \details    Everything here is site-specific rather than firmware behaviour: the
 *                  network to join, where the time comes from, which zone it is shown in.
 *
 *                  Each setting can be overridden from outside, either by a -D build flag
 *                  or by a WordclockSecrets.h next to this file, which is where the
 *                  network credentials belong - that file is not checked in. Credentials
 *                  will move into NVS once the firmware core can persist settings; until
 *                  then they are compiled in.
 *
******************************************************************************************************************************************************/
#ifndef _WORDCLOCK_CONFIGURATION_H_
#define _WORDCLOCK_CONFIGURATION_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#if defined(__has_include)
# if __has_include("WordclockSecrets.h")
#  include "WordclockSecrets.h"
# endif
#endif

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Empty rather than a placeholder network: the application then says so on the console
   and carries on without WiFi, which still leaves a clock that answers commands. A
   placeholder would instead spend every boot failing to join a network that never
   existed. */
#ifndef WORDCLOCK_WIFI_SSID
# define WORDCLOCK_WIFI_SSID                    ""
#endif

#ifndef WORDCLOCK_WIFI_PASSWORD
# define WORDCLOCK_WIFI_PASSWORD                ""
#endif

/* Two servers, because the clock has no other time source: if the first one is
   unreachable the second one still gets the display right. */
#ifndef WORDCLOCK_NTP_SERVER_PRIMARY
# define WORDCLOCK_NTP_SERVER_PRIMARY           "pool.ntp.org"
#endif

#ifndef WORDCLOCK_NTP_SERVER_SECONDARY
# define WORDCLOCK_NTP_SERVER_SECONDARY         "time.cloudflare.com"
#endif

/* A POSIX zone rule rather than an offset, so the C library switches to summer time on
   its own. Central Europe: UTC+1, one hour more from the last Sunday in March until the
   last Sunday in October. */
#ifndef WORDCLOCK_TIMEZONE
# define WORDCLOCK_TIMEZONE                     "CET-1CEST,M3.5.0,M10.5.0/3"
#endif

#ifndef WORDCLOCK_SERIAL_BAUDRATE
# define WORDCLOCK_SERIAL_BAUDRATE              115200
#endif

/* Modem sleep costs a few hundred milliseconds of network latency and saves around
   60 mA, which is more than the rest of the controller draws. A clock notices the one and
   not the other. */
#ifndef WORDCLOCK_WIFI_MODEM_SLEEP
# define WORDCLOCK_WIFI_MODEM_SLEEP             true
#endif

#endif // _WORDCLOCK_CONFIGURATION_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
