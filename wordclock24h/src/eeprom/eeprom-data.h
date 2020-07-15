/*-------------------------------------------------------------------------------------------------------------------------------------------
 * eeprom-data.h - data structure of EEPROM
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef EEPROM_DATA_H
#define EEPROM_DATA_H

#include "night.h"                                              // need MAX_NIGHT_TIMES
#include "display.h"
#include "weather.h"
#include "remote-ir.h"

// Note: All EEPROM versions must have MSB=0x00 and LSB=0x00, only the 2nd and 3rd byte should differ from 0x00
#define EEPROM_VERSION_1_5          0x00010500                  // version 1.5
#define EEPROM_VERSION_1_6          0x00010600                  // version 1.6
#define EEPROM_VERSION_1_7          0x00010700                  // version 1.7
#define EEPROM_VERSION_1_8          0x00010800                  // version 1.8
#define EEPROM_VERSION_1_9          0x00010900                  // version 1.9
#define EEPROM_VERSION_2_0          0x00020000                  // version 2.0
#define EEPROM_VERSION_2_1          0x00020100                  // version 2.1
#define EEPROM_VERSION_2_2          0x00020200                  // version 2.2
#define EEPROM_VERSION_2_3          0x00020300                  // version 2.3
#define EEPROM_VERSION_2_4          0x00020400                  // version 2.4
#define EEPROM_VERSION_2_5          0x00020500                  // version 2.5
#define EEPROM_VERSION_2_6          0x00020600                  // version 2.6
#define EEPROM_VERSION              EEPROM_VERSION_2_6          // current version

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Some packed data to minimize used EEPROM space
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define PACKED_IRMP_DATA_SIZE       5                           // 1 byte for protocol, 2 bytes for address, 2 bytes for command

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Sizes:
 *
 *      EEPROM Version                     4 Bytes   ( 1 *  4)
 *      IRMP data                        160 Bytes   (32 *  5)
 *      Display RGB colors                 3 Bytes   ( 3 *  1)
 *      Display mode                       1 Byte    ( 1 *  1)
 *      Animation mode                     1 Byte    ( 1 *  1)
 *      Color animation mode               1 Byte    ( 1 *  1)
 *      Time server                       16 Bytes   ( 1 * 16)
 *      Time zone                          2 Bytes   ( 1 *  2)
 *      Display flags                      1 Byte    ( 1 *  1)
 *      Display brightness                 1 Byte    ( 1 *  1)
 *      Automatic brightness               1 Byte    ( 1 *  1)
 *      Night time                        32 Byte    ( 8 *  4)
 *      Ambilight RGB colors               3 Bytes   ( 3 *  1)
 *      Ambilight brightness               1 Byte    ( 1 *  1)
 *      Ambilight mode                     1 Byte    ( 1 *  1)
 *      Ambilight leds                     1 Byte    ( 1 *  1)
 *      Ambilight offset sec=0             1 Byte    ( 1 *  1)
 *      RTC temperature correction         1 Byte    ( 1 *  1)
 *      DS18xx temperature correction      1 Byte    ( 1 *  1)
 *      Not used (old: temp interval)      1 Byte    ( 1 *  1)
 *      LDR minimum value                  2 Bytes   ( 1 *  2)
 *      LDR maximum value                  2 Bytes   ( 1 *  2)
 *      Animation values                  64 Bytes   (64 *  1)
 *      Color animation values            16 Bytes   (16 *  1)
 *      Ambilight mode value              16 Bytes   (16 *  1)
 *      Display W color                    1 Byte    ( 1 *  1)
 *      Ambilight W color                  1 Byte    ( 1 *  1)
 *      Weather appid                     32 Bytes   (32 *  1)
 *      Weather city                      32 Bytes   (32 *  1)
 *      Weather longitude                  8 Bytes   ( 8 *  1)
 *      Weather latitude                   8 Bytes   ( 8 *  1)
 *      Overlay intervals                 64 Bytes   (64 *  1)
 *      Ambilight night times             32 Bytes   ( 8 *  4)
 *      Update host                       64 Bytes   (64 *  1)
 *      Update path                       64 Bytes   (64 *  1)
 *      Ticker deceleration                1 Byte    ( 1 *  1)
 *      ======================================================
 *      Sum                              608 Bytes
 *
 *  EEPROM size of AT24C32: 32KBit = 4096 Bytes
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

#define EEPROM_MAX_IR_CODES                         32                      // max IR codes, should be greater or equal than N_REMOTE_IR_CMDS
#if N_REMOTE_IR_CMDS > EEPROM_MAX_IR_CODES
#error value for EEPROM_MAX_IR_CODES is too low
#endif

#define EEPROM_MAX_IPADDR_LEN                       16                      // max length of IP address xxx.xxx.xxx.xxx + '\0'
#define EEPROM_MAX_HOSTNAME_LEN                     64                      // max length of hostname + '\0'
#define EEPROM_MAX_UPDATE_PATH_LEN                  64                      // max length of update path + '\0'
#define EEPROM_MAX_TIMEZONE_LEN                      2                      // max length of timezone: one byte for +/-, one byte for hour offset
#define EEPROM_MAX_NIGHT_TIME_LEN                   (MAX_NIGHT_TIMES * 3)   // length of night times: 1 byte flags, 2 bytes minute

#define EEPROM_MAX_OVERLAY_INTERVALS                64                      // max overlay intervals

#define EEPROM_DATA_SIZE_VERSION                    sizeof (uint32_t)
#define EEPROM_DATA_SIZE_IRMP_DATA                  (EEPROM_MAX_IR_CODES * PACKED_IRMP_DATA_SIZE)
#define EEPROM_DATA_SIZE_DSP_COLORS                 (3 * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_DISPLAY_MODE               sizeof (uint8_t)
#define EEPROM_DATA_SIZE_ANIMATION_MODE             sizeof (uint8_t)
#define EEPROM_DATA_SIZE_COLOR_ANIMATION_MODE       sizeof (uint8_t)
#define EEPROM_DATA_SIZE_TIMESERVER                 (EEPROM_MAX_IPADDR_LEN)
#define EEPROM_DATA_SIZE_TIMEZONE                   (EEPROM_MAX_TIMEZONE_LEN)
#define EEPROM_DATA_SIZE_DISPLAY_FLAGS              sizeof (uint8_t)
#define EEPROM_DATA_SIZE_BRIGHTNESS                 sizeof (uint8_t)
#define EEPROM_DATA_SIZE_AUTO_BRIGHTNESS            sizeof (uint8_t)
#define EEPROM_DATA_SIZE_NIGHT_TIME                 (EEPROM_MAX_NIGHT_TIME_LEN)
#define EEPROM_DATA_SIZE_AMBI_COLORS                (3 * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_AMBI_BRIGHTNESS            sizeof (uint8_t)
#define EEPROM_DATA_SIZE_AMBI_MODE                  sizeof (uint8_t)
#define EEPROM_DATA_SIZE_AMBI_LEDS                  sizeof (uint8_t)        // number of ambilight LEDs
#define EEPROM_DATA_SIZE_AMBI_OFFSET_SEC0           sizeof (uint8_t)        // ambilight offset of second 0
#define EEPROM_DATA_SIZE_RTC_TEMP_CORR              sizeof (uint8_t)        // RTC temperature correction
#define EEPROM_DATA_SIZE_DS18XX_TEMP_CORR           sizeof (uint8_t)        // DS18xx temperature correction
#define EEPROM_DATA_SIZE_NOT_USED_01                sizeof (uint8_t)        // not used anymore, obsolete: show temperature every n minutes
#define EEPROM_DATA_SIZE_LDR_MIN_VALUE              sizeof (uint16_t)       // minimum LDR value
#define EEPROM_DATA_SIZE_LDR_MAX_VALUE              sizeof (uint16_t)       // maximum LDR value
#define EEPROM_DATA_SIZE_ANIMATION_VALUES           (EEPROM_MAX_ANIMATION_MODES * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_COLOR_ANIMATION_VALUES     (EEPROM_MAX_COLOR_ANIMATION_MODES * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_AMBILIGHT_MODE_VALUES      (EEPROM_MAX_AMBILIGHT_MODES * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_DSP_W_COLOR                sizeof (uint8_t)
#define EEPROM_DATA_SIZE_AMBI_W_COLOR               sizeof (uint8_t)
#define EEPROM_DATA_SIZE_WEATHER_APPID              (MAX_WEATHER_APPID_LEN * sizeof (char))
#define EEPROM_DATA_SIZE_WEATHER_CITY               (MAX_WEATHER_CITY_LEN * sizeof (char))
#define EEPROM_DATA_SIZE_WEATHER_LON                (MAX_WEATHER_LON_LEN * sizeof (char))
#define EEPROM_DATA_SIZE_WEATHER_LAT                (MAX_WEATHER_LAT_LEN * sizeof (char))
#define EEPROM_DATA_SIZE_OVERLAY_INTERVALS          (EEPROM_MAX_OVERLAY_INTERVALS * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_AMBI_NIGHT_TIME            (EEPROM_MAX_NIGHT_TIME_LEN)
#define EEPROM_DATA_SIZE_DIMMED_COLORS              ((MAX_BRIGHTNESS + 1) * sizeof (uint8_t))
#define EEPROM_DATA_SIZE_UPDATE_HOSTNAME            (EEPROM_MAX_HOSTNAME_LEN)
#define EEPROM_DATA_SIZE_UPDATE_PATH                (EEPROM_MAX_UPDATE_PATH_LEN)
#define EEPROM_DATA_SIZE_TICKER_DECELERATION        sizeof (uint8_t)

#define EEPROM_DATA_OFFSET_VERSION                  0
#define EEPROM_DATA_OFFSET_IRMP_DATA                (EEPROM_DATA_OFFSET_VERSION                 + EEPROM_DATA_SIZE_VERSION)
#define EEPROM_DATA_OFFSET_DSP_COLORS               (EEPROM_DATA_OFFSET_IRMP_DATA               + EEPROM_DATA_SIZE_IRMP_DATA)
#define EEPROM_DATA_OFFSET_DISPLAY_MODE             (EEPROM_DATA_OFFSET_DSP_COLORS              + EEPROM_DATA_SIZE_DSP_COLORS)
#define EEPROM_DATA_OFFSET_ANIMATION_MODE           (EEPROM_DATA_OFFSET_DISPLAY_MODE            + EEPROM_DATA_SIZE_DISPLAY_MODE)
#define EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE     (EEPROM_DATA_OFFSET_ANIMATION_MODE          + EEPROM_DATA_SIZE_ANIMATION_MODE)
#define EEPROM_DATA_OFFSET_TIMESERVER               (EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE    + EEPROM_DATA_SIZE_COLOR_ANIMATION_MODE)
#define EEPROM_DATA_OFFSET_TIMEZONE                 (EEPROM_DATA_OFFSET_TIMESERVER              + EEPROM_DATA_SIZE_TIMESERVER)
#define EEPROM_DATA_OFFSET_DISPLAY_FLAGS            (EEPROM_DATA_OFFSET_TIMEZONE                + EEPROM_DATA_SIZE_TIMEZONE)
#define EEPROM_DATA_OFFSET_BRIGHTNESS               (EEPROM_DATA_OFFSET_DISPLAY_FLAGS           + EEPROM_DATA_SIZE_DISPLAY_FLAGS)
#define EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS          (EEPROM_DATA_OFFSET_BRIGHTNESS              + EEPROM_DATA_SIZE_BRIGHTNESS)
#define EEPROM_DATA_OFFSET_NIGHT_TIME               (EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS         + EEPROM_DATA_SIZE_AUTO_BRIGHTNESS)
#define EEPROM_DATA_OFFSET_AMBI_COLORS              (EEPROM_DATA_OFFSET_NIGHT_TIME              + EEPROM_DATA_SIZE_NIGHT_TIME)
#define EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS          (EEPROM_DATA_OFFSET_AMBI_COLORS             + EEPROM_DATA_SIZE_AMBI_COLORS)
#define EEPROM_DATA_OFFSET_AMBI_MODE                (EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS         + EEPROM_DATA_SIZE_AMBI_BRIGHTNESS)
#define EEPROM_DATA_OFFSET_AMBI_LEDS                (EEPROM_DATA_OFFSET_AMBI_MODE               + EEPROM_DATA_SIZE_AMBI_MODE)
#define EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0         (EEPROM_DATA_OFFSET_AMBI_LEDS               + EEPROM_DATA_SIZE_AMBI_LEDS)
#define EEPROM_DATA_OFFSET_RTC_TEMP_CORR            (EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0        + EEPROM_DATA_SIZE_AMBI_OFFSET_SEC0)
#define EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR         (EEPROM_DATA_OFFSET_RTC_TEMP_CORR           + EEPROM_DATA_SIZE_RTC_TEMP_CORR)
#define EEPROM_DATA_OFFSET_NOT_USED_01              (EEPROM_DATA_OFFSET_DS18XX_TEMP_CORR        + EEPROM_DATA_SIZE_DS18XX_TEMP_CORR)
#define EEPROM_DATA_OFFSET_LDR_MIN_VALUE            (EEPROM_DATA_OFFSET_NOT_USED_01             + EEPROM_DATA_SIZE_NOT_USED_01)
#define EEPROM_DATA_OFFSET_LDR_MAX_VALUE            (EEPROM_DATA_OFFSET_LDR_MIN_VALUE           + EEPROM_DATA_SIZE_LDR_MIN_VALUE)
#define EEPROM_DATA_OFFSET_ANIMATION_VALUES         (EEPROM_DATA_OFFSET_LDR_MAX_VALUE           + EEPROM_DATA_SIZE_LDR_MAX_VALUE)
#define EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES   (EEPROM_DATA_OFFSET_ANIMATION_VALUES        + EEPROM_DATA_SIZE_ANIMATION_VALUES)
#define EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES    (EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES  + EEPROM_DATA_SIZE_COLOR_ANIMATION_VALUES)
#define EEPROM_DATA_OFFSET_DSP_W_COLOR              (EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES   + EEPROM_DATA_SIZE_AMBILIGHT_MODE_VALUES)
#define EEPROM_DATA_OFFSET_AMBI_W_COLOR             (EEPROM_DATA_OFFSET_DSP_W_COLOR             + EEPROM_DATA_SIZE_DSP_W_COLOR)
#define EEPROM_DATA_OFFSET_WEATHER_APPID            (EEPROM_DATA_OFFSET_AMBI_W_COLOR            + EEPROM_DATA_SIZE_AMBI_W_COLOR)
#define EEPROM_DATA_OFFSET_WEATHER_CITY             (EEPROM_DATA_OFFSET_WEATHER_APPID           + EEPROM_DATA_SIZE_WEATHER_APPID)
#define EEPROM_DATA_OFFSET_WEATHER_LON              (EEPROM_DATA_OFFSET_WEATHER_CITY            + EEPROM_DATA_SIZE_WEATHER_CITY)
#define EEPROM_DATA_OFFSET_WEATHER_LAT              (EEPROM_DATA_OFFSET_WEATHER_LON             + EEPROM_DATA_SIZE_WEATHER_LON)
#define EEPROM_DATA_OFFSET_OVERLAY_INTERVALS        (EEPROM_DATA_OFFSET_WEATHER_LAT             + EEPROM_DATA_SIZE_WEATHER_LAT)
#define EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME          (EEPROM_DATA_OFFSET_OVERLAY_INTERVALS       + EEPROM_DATA_SIZE_OVERLAY_INTERVALS)
#define EEPROM_DATA_OFFSET_DIMMED_COLORS            (EEPROM_DATA_OFFSET_AMBI_NIGHT_TIME         + EEPROM_DATA_SIZE_AMBI_NIGHT_TIME)
#define EEPROM_DATA_OFFSET_UPDATE_HOSTNAME          (EEPROM_DATA_OFFSET_DIMMED_COLORS           + EEPROM_DATA_SIZE_DIMMED_COLORS)
#define EEPROM_DATA_OFFSET_UPDATE_PATH              (EEPROM_DATA_OFFSET_UPDATE_HOSTNAME         + EEPROM_DATA_SIZE_UPDATE_HOSTNAME)
#define EEPROM_DATA_OFFSET_TICKER_DECELERATION      (EEPROM_DATA_OFFSET_UPDATE_PATH             + EEPROM_DATA_SIZE_UPDATE_PATH)

#define EEPROM_DATA_END                             (EEPROM_DATA_OFFSET_TICKER_DECELERATION     + EEPROM_DATA_SIZE_TICKER_DECELERATION)

#endif
