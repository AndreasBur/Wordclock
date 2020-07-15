/*-------------------------------------------------------------------------------------------------------------------------------------------
 * vars.h - synchronisation of variables/parameters between STM32 and ESP8266
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * remote procedure calls:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    LDR_MIN_VALUE_RPC_VAR,                                                  // store LDR value as minimum value
    LDR_MAX_VALUE_RPC_VAR,                                                  // store LDR value as maximum value
    LEARN_IR_RPC_VAR,                                                       // learn ir remote control
    GET_NET_TIME_RPC_VAR,                                                   // get net time
    DISPLAY_TEMPERATURE_RPC_VAR,                                            // display current temperature
    TEST_DISPLAY_RPC_VAR,                                                   // test display
    GET_WEATHER_RPC_VAR,                                                    // get weather
    MAX_RPC_VARIABLES                                                       // must be the last member
} RPC_VARIABLE;

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
    MAX_NUM_VARIABLES                                                       // must be the last member
} NUM_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * numeric arrays:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    DISPLAY_DIMMED_COLORS,
    MAX_NUM_ARRAYS                                                          // must be the last member
} NUM_ARRAY;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * string variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
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
    MAX_STR_VARIABLES                                                       // must be the last member
} STR_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * TM variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    CURRENT_TM_VAR,
    MAX_TM_VARIABLES                                                        // must be the last member
} TM_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * 24 virtual display mode variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
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
    MAX_DISPLAY_MODE_VARIABLES                                              // must be the last member
} DISPLAY_MODE_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display color variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    DISPLAY_DSP_COLOR_VAR,
    AMBILIGHT_DSP_COLOR_VAR,
    MAX_DSP_COLOR_VARIABLES                                                 // must be the last member
} DSP_COLOR_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display animation variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
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
    MAX_DISPLAY_ANIMATION_VARIABLES                                         // must be the last member
} DISPLAY_ANIMATION_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * color animation variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    NO_COLOR_ANIMATION_VAR,
    RAINBOW_COLOR_ANIMATION_VAR,
    MAX_COLOR_ANIMATION_VARIABLES                                           // must be the last member
} COLOR_ANIMATION_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ambilight mode variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum
{
    NORMAL_AMBILIGHT_MODE_VAR,
    CLOCK_AMBILIGHT_MODE_VAR,
    CLOCK2_AMBILIGHT_MODE_VAR,
    RAINBOW_AMBILIGHT_MODE_VAR,
    MAX_AMBILIGHT_MODE_VARIABLES                                            // must be the last member
} AMBILIGHT_MODE_VARIABLE;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * night time variables:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
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

extern void     var_send_display_mode (void);
extern void     var_send_display_automatic_brightness_active (void);
extern void     var_send_ambilight_brightness (void);
extern void     var_send_animation_mode (void);
extern void     var_send_color_animation_mode (void);
extern void     var_send_display_brightness (void);

extern void     var_send_ldr_min_value (void);
extern void     var_send_ldr_max_value (void);
extern void     var_send_ldr_raw_value (void);
extern void     var_send_rtc_temp_index (void);
extern void     var_send_ds18xx_temp_index (void);
extern void     var_send_ambilight_mode (void);
extern void     var_send_display_power (void);

extern void     var_send_version (void);
extern void     var_send_eeprom_version (void);
extern void     var_send_timeserver (void);
extern void     var_send_weather_appid (void);
extern void     var_send_weather_city (void);
extern void     var_send_weather_lon (void);
extern void     var_send_weather_lat (void);
extern void     var_send_tm (void);
extern void     var_send_display_mode_names (void);
extern void     var_send_display_colors (void);
extern void     var_send_ambilight_colors (void);
extern void     var_send_display_animations (void);
extern void     var_send_color_animations (void);
extern void     var_send_ambilight_modes (void);
extern void     var_send_use_rgbw (void);
extern void     var_send_all_variables (void);
