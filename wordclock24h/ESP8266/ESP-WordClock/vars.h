/*----------------------------------------------------------------------------------------------------------------------------------------
 * vars.h - synchronization of variables between STM32 and ESP8266
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef VARS_H
#define VARS_H

#include <stdint.h>
#include <string.h>
#include <time.h>

#define COLOR_ANIMATION_MODE_NONE                   0                   // no color animation
#define COLOR_ANIMATION_MODE_RAINBOW                1                   // rainbow

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * hardware configuration word: 0 xxx xxx LLL WWW HHH
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define HW_STM32_MASK               (0x07 << 0)                         // 3 bits for microprocessor mask
#define HW_STM32_F103               (0x00 << 0)                         // 0: F103
#define HW_STM32_F401               (0x01 << 0)                         // 1: F401
#define HW_STM32_F411               (0x02 << 0)                         // 2: F411

#define HW_WC_MASK                  (0x07 << 3)                         // 3 bits for WC mask
#define HW_WC_24H                   (0x00 << 3)                         // 0: WC24
#define HW_WC_12H                   (0x01 << 3)                         // 1: WC12

#define HW_LED_MASK                 (0x07 << 6)                         // 3 bits for LED type mask
#define HW_LED_WS2812_GRB_LED       (0x00 << 6)                         // 0: WS2812 GRB
#define HW_LED_WS2812_RGB_LED       (0x01 << 6)                         // 1: WS2812 RGB
#define HW_LED_APA102_RGB_LED       (0x02 << 6)                         // 2: APA102 RGB
#define HW_LED_SK6812_RGB_LED       (0x03 << 6)                         // 3: SK6812 RGB
#define HW_LED_SK6812_RGBW_LED      (0x04 << 6)                         // 4: SK6812 RGBW


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * remote procedure calls:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    LDR_MIN_VALUE_RPC_VAR,                                              // store LDR value as minimum value
    LDR_MAX_VALUE_RPC_VAR,                                              // store LDR value as maximum value
    LEARN_IR_RPC_VAR,                                                   // learn ir remote control
    GET_NET_TIME_RPC_VAR,                                               // get net time
    DISPLAY_TEMPERATURE_RPC_VAR,                                        // display current temperature
    TEST_DISPLAY_RPC_VAR,                                               // test display
    GET_WEATHER_RPC_VAR,                                                // get weather
    MAX_RPC_VARIABLES                                                   // must be the last member
} RPC_VARIABLE;

extern unsigned int     rpc (RPC_VARIABLE);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * numeric variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    DISPLAY_USE_RGBW_NUM_VAR,
    EEPROM_IS_UP_NUM_VAR,
    RTC_IS_UP_NUM_VAR,
    DISPLAY_POWER_NUM_VAR,
    DISPLAY_MODE_NUM_VAR,
    MAX_DISPLAY_MODES_NUM_VAR,
    DISPLAY_BRIGHTNESS_NUM_VAR,
    DISPLAY_FLAGS_NUM_VAR,
    DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR,
    DISPLAY_TEMPERATURE_INTERVAL_NUM_VAR,
    ANIMATION_MODE_NUM_VAR,
    AMBILIGHT_MODE_NUM_VAR,
    AMBILIGHT_LEDS_NUM_VAR,
    AMBILIGHT_OFFSET_NUM_VAR,
    AMBILIGHT_BRIGHTNESS_NUM_VAR,
    COLOR_ANIMATION_MODE_NUM_VAR,
    LDR_RAW_VALUE_NUM_VAR,
    LDR_MIN_VALUE_NUM_VAR,
    LDR_MAX_VALUE_NUM_VAR,
    TIMEZONE_NUM_VAR,
    DS18XX_IS_UP_NUM_VAR,
    RTC_TEMP_INDEX_NUM_VAR,
    RTC_TEMP_CORRECTION_NUM_VAR,
    DS18XX_TEMP_INDEX_NUM_VAR,
    DS18XX_TEMP_CORRECTION_NUM_VAR,
    DISPLAY_HEART_INTERVAL_NUM_VAR,
    DISPLAY_XMAS_TREE_INTERVAL_NUM_VAR,
    DISPLAY_DATE_INTERVAL_NUM_VAR,
    DISPLAY_FIREWORKS_INTERVAL_NUM_VAR,
    HARDWARE_CONFIGURATION_NUM_VAR,
    DISPLAY_AMBILIGHT_POWER_NUM_VAR,
    TICKER_DECELRATION_NUM_VAR,
    MAX_NUM_VARIABLES                                                   // must be the last member
} NUM_VARIABLE;

extern unsigned int     numvars[MAX_NUM_VARIABLES];
extern unsigned int     get_numvar (NUM_VARIABLE);
extern unsigned int     set_numvar (NUM_VARIABLE, unsigned int);
extern void             vars_init (void);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * numeric arrays:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

typedef enum
{
    DISPLAY_DIMMED_COLORS,
    MAX_NUM_ARRAYS                                                      // must be the last member
} NUM_ARRAY;

#define MAX_BRIGHTNESS      15
extern uint8_t              dimmed_colors[MAX_BRIGHTNESS + 1];
extern unsigned int         get_num_array (NUM_ARRAY, uint_fast8_t);
extern unsigned int         set_num_array (NUM_ARRAY, uint_fast8_t, uint_fast8_t);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * string variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_TICKER_TEXT_LEN             32
#define MAX_VERSION_TEXT_LEN            8
#define MAX_EEPROM_VERSION_TEXT_LEN     8
#define MAX_ESP8266_VERSION_TEXT_LEN    8
#define MAX_TIMESERVER_NAME_LEN         16
#define MAX_WEATHER_APPID_LEN           32
#define MAX_WEATHER_CITY_LEN            32
#define MAX_WEATHER_LON_LEN             8
#define MAX_WEATHER_LAT_LEN             8
#define MAX_UPDATE_HOST_LEN             (64 - 1)
#define MAX_UPDATE_PATH_LEN             (64 - 1)

typedef struct
{
    char *          str;
    uint_fast16_t   maxlen;
} STR_VAR;

typedef enum
{
    TICKER_TEXT_STR_VAR,
    VERSION_STR_VAR,
    EEPROM_VERSION_STR_VAR,
    ESP8266_VERSION_STR_VAR,
    TIMESERVER_STR_VAR,
    WEATHER_APPID_STR_VAR,
    WEATHER_CITY_STR_VAR,
    WEATHER_LON_STR_VAR,
    WEATHER_LAT_STR_VAR,
    UPDATE_HOST_VAR,
    UPDATE_PATH_VAR,
    MAX_STR_VARIABLES                                                   // must be the last member
} STR_VARIABLE;

extern STR_VAR          strvars[MAX_STR_VARIABLES];
extern STR_VAR *        get_strvar (STR_VARIABLE);
extern unsigned int     set_strvar (STR_VARIABLE, char *);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tm variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct tm TM;

typedef enum
{
    CURRENT_TM_VAR,
    MAX_TM_VARIABLES                                       // must be the last member
} TM_VARIABLE;

extern TM               tmvars[MAX_TM_VARIABLES];
extern TM *             get_tm_var (TM_VARIABLE);
extern unsigned int     set_tm_var (TM_VARIABLE, TM *);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display mode variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_DISPLAY_MODE_NAME_LEN   32

typedef struct
{
    char name[MAX_DISPLAY_MODE_NAME_LEN + 1];
} DISPLAY_MODE;

typedef enum
{
    DISPLAY_MODE_0_VAR,
    DISPLAY_MODE_1_VAR,
    DISPLAY_MODE_2_VAR,
    DISPLAY_MODE_3_VAR,
    DISPLAY_MODE_4_VAR,
    DISPLAY_MODE_5_VAR,
    DISPLAY_MODE_6_VAR,
    DISPLAY_MODE_7_VAR,
    DISPLAY_MODE_8_VAR,
    DISPLAY_MODE_9_VAR,
    DISPLAY_MODE_10_VAR,
    DISPLAY_MODE_11_VAR,
    DISPLAY_MODE_12_VAR,
    DISPLAY_MODE_13_VAR,
    DISPLAY_MODE_14_VAR,
    DISPLAY_MODE_15_VAR,
    DISPLAY_MODE_16_VAR,
    DISPLAY_MODE_17_VAR,
    DISPLAY_MODE_18_VAR,
    DISPLAY_MODE_19_VAR,
    DISPLAY_MODE_20_VAR,
    DISPLAY_MODE_21_VAR,
    DISPLAY_MODE_22_VAR,
    DISPLAY_MODE_23_VAR,
    MAX_DISPLAY_MODE_VARIABLES                                       // must be the last member
} DISPLAY_MODE_VARIABLE;

extern DISPLAY_MODE      displaymodevars[MAX_DISPLAY_MODE_VARIABLES];
extern DISPLAY_MODE *    get_display_mode_var (DISPLAY_MODE_VARIABLE);
extern unsigned int      set_display_mode_name (DISPLAY_MODE_VARIABLE, char *);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display color variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct                                                          // display colors
{
    uint8_t red;                                                        // red, 0...63 (MAX_COLOR_STEPS)
    uint8_t green;                                                      // green, 0...63 (MAX_COLOR_STEPS)
    uint8_t blue;                                                       // blue, 0...63 (MAX_COLOR_STEPS)
    uint8_t white;                                                      // white, 0...63 (MAX_COLOR_STEPS)
} DSP_COLORS;


typedef enum
{
    DISPLAY_DSP_COLOR_VAR,
    AMBILIGHT_DSP_COLOR_VAR,
    MAX_DSP_COLOR_VARIABLES                                                   // must be the last member
} DSP_COLOR_VARIABLE;

extern DSP_COLORS       dspcolorvars[MAX_DSP_COLOR_VARIABLES];
extern unsigned int     get_dsp_color_var (DSP_COLOR_VARIABLE, DSP_COLORS * rgb);
extern unsigned int     set_dsp_color_var (DSP_COLOR_VARIABLE, DSP_COLORS * rgb, int);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display animation variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_DISPLAY_ANIMATION_NAME_LEN          15

#define ANIMATION_MIN_DECELERATION               1
#define ANIMATION_MAX_DECELERATION              15

#define ANIMATION_FLAG_NONE                     0x00                    // no animation flag
#define ANIMATION_FLAG_CONFIGURABLE             0x01                    // animation selectable per random animation
#define ANIMATION_FLAG_FAVOURITE                0x02                    // animation is favourite

typedef struct
{
    char            name[MAX_DISPLAY_ANIMATION_NAME_LEN + 1];
    uint_fast8_t    deceleration;                                       // 1 = fastest, 15 = slowest
    uint_fast8_t    default_deceleration;                               // 1 = fastest, 15 = slowest
    uint_fast8_t    flags;                                              // yet not used
} DISPLAY_ANIMATION;

typedef enum
{
    NO_DISPLAY_ANIMATION_VAR,
    FADE_DISPLAY_ANIMATION_VAR,
    ROLL_DISPLAY_ANIMATION_VAR,
    EXPLODE_DISPLAY_ANIMATION_VAR,
    RANDOM_DISPLAY_ANIMATION_VAR,
    SNAKE_DISPLAY_ANIMATION_VAR,
    TELETYPE_DISPLAY_ANIMATION_VAR,
    CUBE_DISPLAY_ANIMATION_VAR,
    MATRIX_DISPLAY_ANIMATION_VAR,
    DROP_DISPLAY_ANIMATION_VAR,
    SQUEEZE_DISPLAY_ANIMATION_VAR,
    FLICKER_DISPLAY_ANIMATION_VAR,
    MAX_DISPLAY_ANIMATION_VARIABLES                                       // must be the last member
} DISPLAY_ANIMATION_VARIABLE;

extern DISPLAY_ANIMATION    displayanimationvars[MAX_DISPLAY_ANIMATION_VARIABLES];
extern DISPLAY_ANIMATION *  get_display_animation_var (DISPLAY_ANIMATION_VARIABLE);
extern unsigned int         set_display_animation_name (DISPLAY_ANIMATION_VARIABLE, char *);
extern unsigned int         set_display_animation_deceleration (DISPLAY_ANIMATION_VARIABLE, uint_fast8_t);
extern unsigned int         set_display_animation_default_deceleration (DISPLAY_ANIMATION_VARIABLE, uint_fast8_t);
extern unsigned int         set_display_animation_flags (DISPLAY_ANIMATION_VARIABLE, uint_fast8_t);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * color animation variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_COLOR_ANIMATION_NAME_LEN                15

#define COLOR_ANIMATION_MAX_DECELERATION            15                  // deceleration = 15 is lowest deceleration, min is 0

#define COLOR_ANIMATION_FLAG_NONE                   0x00                // no color animation flag
#define COLOR_ANIMATION_FLAG_CONFIGURABLE           0x01                // animation is configurable


typedef struct
{
    char            name[MAX_COLOR_ANIMATION_NAME_LEN + 1];
    uint_fast8_t    deceleration;                                       // 0 = fastest, 15 = slowest
    uint_fast8_t    default_deceleration;                               // 0 = fastest, 15 = slowest
    uint_fast8_t    flags;                                              // yet not used
} COLOR_ANIMATION;

typedef enum
{
    NO_COLOR_ANIMATION_VAR,
    RAINBOW_COLOR_ANIMATION_VAR,
    MAX_COLOR_ANIMATION_VARIABLES                                       // must be the last member
} COLOR_ANIMATION_VARIABLE;

extern COLOR_ANIMATION      coloranimationvars[MAX_COLOR_ANIMATION_VARIABLES];
extern COLOR_ANIMATION *    get_color_animation_var (COLOR_ANIMATION_VARIABLE);
extern unsigned int         set_color_animation_name (COLOR_ANIMATION_VARIABLE, char *);
extern unsigned int         set_color_animation_deceleration (COLOR_ANIMATION_VARIABLE, uint_fast8_t);
extern unsigned int         set_color_animation_default_deceleration (COLOR_ANIMATION_VARIABLE, uint_fast8_t);
extern unsigned int         set_color_animation_flags (COLOR_ANIMATION_VARIABLE, uint_fast8_t);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ambilight mode variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_AMBILIGHT_MODE_NAME_LEN             15

#define AMBILIGHT_MODE_MAX_DECELERATION         15                      // deceleration = 15 is lowest deceleration, min is 0

#define AMBILIGHT_FLAG_NONE                     0x00                    // no animation flag
#define AMBILIGHT_FLAG_CONFIGURABLE             0x01                    // ambilight mode is configurable
#define AMBILIGHT_FLAG_SECONDS_MARKER            0x02                    // only for CLOCK ambilight mode

typedef struct
{
    char            name[MAX_AMBILIGHT_MODE_NAME_LEN + 1];
    uint_fast8_t    deceleration;                                       // 0 = fastest, 15 = slowest
    uint_fast8_t    default_deceleration;                               // 0 = fastest, 15 = slowest
    uint_fast8_t    flags;
} AMBILIGHT_MODE;

typedef enum
{
    NORMAL_AMBILIGHT_MODE_VAR,
    CLOCK_AMBILIGHT_MODE_VAR,
    CLOCK2_AMBILIGHT_MODE_VAR,
    RAINBOW_AMBILIGHT_MODE_VAR,
    MAX_AMBILIGHT_MODE_VARIABLES                                       // must be the last member
} AMBILIGHT_MODE_VARIABLE;

extern AMBILIGHT_MODE       ambilightmodevars[MAX_AMBILIGHT_MODE_VARIABLES];
extern AMBILIGHT_MODE *     get_ambilight_mode_var (AMBILIGHT_MODE_VARIABLE);
extern unsigned int         set_ambilight_mode_name (AMBILIGHT_MODE_VARIABLE, char *);
extern unsigned int         set_ambilight_mode_deceleration (AMBILIGHT_MODE_VARIABLE, uint_fast8_t);
extern unsigned int         set_ambilight_mode_default_deceleration (AMBILIGHT_MODE_VARIABLE, uint_fast8_t);
extern unsigned int         set_ambilight_mode_flags (AMBILIGHT_MODE_VARIABLE, uint_fast8_t);

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * night time variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

#define NIGHT_TIME_FLAG_ACTIVE      0x80                                    // flag if entry is active (1) or not (0)
#define NIGHT_TIME_FLAG_SWITCH_ON   0x40                                    // flag if entry switches on (1) or off (0)
#define NIGHT_TIME_FROM_DAY_MASK    0x38                                    // 3 bits for from day
#define NIGHT_TIME_TO_DAY_MASK      0x07                                    // 3 bits for to day

typedef struct
{
    uint_fast8_t        flags;                                              // flags
    uint_fast16_t       minutes;                                            // time in minutes 0 - 1439
} NIGHT_TIME;

typedef enum
{
    NIGHT0_NIGHT_TIME_VAR,
    NIGHT1_NIGHT_TIME_VAR,
    NIGHT2_NIGHT_TIME_VAR,
    NIGHT3_NIGHT_TIME_VAR,
    NIGHT4_NIGHT_TIME_VAR,
    NIGHT5_NIGHT_TIME_VAR,
    NIGHT6_NIGHT_TIME_VAR,
    NIGHT7_NIGHT_TIME_VAR,
    MAX_NIGHT_TIME_VARIABLES                                                // must be the last member
} NIGHT_TIME_VARIABLE;

extern NIGHT_TIME           nighttimevars[MAX_NIGHT_TIME_VARIABLES];
extern NIGHT_TIME           ambilightnighttimevars[MAX_NIGHT_TIME_VARIABLES];

extern NIGHT_TIME *         get_night_time_var (uint_fast8_t, NIGHT_TIME_VARIABLE);
extern unsigned int         set_night_time_var (uint_fast8_t, NIGHT_TIME_VARIABLE, uint_fast16_t, uint_fast8_t);

extern void                 var_set_parameter (char *);
#endif
