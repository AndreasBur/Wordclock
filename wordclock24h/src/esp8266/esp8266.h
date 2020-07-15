/*-------------------------------------------------------------------------------------------------------------------------------------------
 * esp8266.h - declarations of ESP8266 routines
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef ESP8266_H
#define ESP8266_H

#include <string.h>
#include <stdint.h>
#include <time.h>

#define ESP8266_MAX_ANSWER_LEN          256                     // max length of ESP answer length, could be very long if "HTTP GET ..."

/*--------------------------------------------------------------------------------------------------------------------------------------
 * possible return values of esp8266_get_answer():
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP8266_TIMEOUT                 0
#define ESP8266_OK                      1
#define ESP8266_ERROR                   2
#define ESP8266_DEBUGMSG                3
#define ESP8266_IPADDRESS               4
#define ESP8266_ACCESSPOINT             5
#define ESP8266_MODE                    6
#define ESP8266_TIME                    7
#define ESP8266_FIRMWARE                8
#define ESP8266_CMD                     9
#define ESP8266_WEATHER                 10
#define ESP8266_UNSPECIFIED             0xFF

/*--------------------------------------------------------------------------------------------------------------------------------------
 * possible modes
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP8266_UNKNOWN_MODE            0
#define ESP8266_CLIENT_MODE             1
#define ESP8266_AP_MODE                 2

/*--------------------------------------------------------------------------------------------------------------------------------------
 * sizes
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP8266_MAX_FIRMWARE_LEN        15
#define ESP8266_MAX_ACCESSPOINT_LEN     31
#define ESP8266_MAX_IPADDRESS_LEN       15
#define ESP8266_MAX_CMD_LEN             63
#define ESP8266_MAX_TIME_LEN            15
#define ESP8266_MAX_WEATHER_LEN         63

typedef struct
{
    uint_fast8_t                        is_up;
    uint_fast8_t                        is_online;
    uint_fast8_t                        mode;
    char                                firmware[ESP8266_MAX_FIRMWARE_LEN + 1];
    char                                accesspoint[ESP8266_MAX_ACCESSPOINT_LEN + 1];
    char                                ipaddress[ESP8266_MAX_IPADDRESS_LEN + 1];
    char                                cmd[ESP8266_MAX_CMD_LEN + 1];
    char                                time[ESP8266_MAX_TIME_LEN + 1];
    char                                weather[ESP8266_MAX_WEATHER_LEN + 1];
} ESP8266_GLOBALS;

extern ESP8266_GLOBALS                  esp8266;

extern volatile uint_fast8_t            esp8266_ten_ms_tick;

extern uint_fast8_t                     esp8266_get_message (void);
extern void                             esp8266_send_cmd (const char *);
extern void                             esp8266_send_data (unsigned char *, uint_fast8_t);
extern uint_fast8_t                     esp8266_get_up_status (void);
extern uint_fast8_t                     esp8266_get_online_status (void);
extern char *                           esp8266_get_access_point_connected (void);
extern char *                           esp8266_get_ip_address (void);
extern char *                           esp8266_get_firmware_version (void);
extern void                             esp8266_reset (void);
extern void                             esp8266_powerdown (void);
extern void                             esp8266_powerup (void);
extern void                             esp8266_connect_to_access_point (char *, char *);
extern void                             esp8266_accesspoint (const char *, const char *);
extern void                             esp8266_wps (void);
extern void                             esp8266_flash (void);
extern void                             esp8266_init (void);
#endif
