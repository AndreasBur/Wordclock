/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display.c - routines for LED display 16x18 and 10x11
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "wclock24h-config.h"

#if WCLOCK24H == 1
#include "tables.h"
#else
#include "tables12h.h"
#endif

#include "display.h"
#include "power.h"
#include "irmp.h"
#include "vars.h"

#include "eeprom.h"
#include "eeprom-data.h"
#include "delay.h"
#include "ldr.h"
#include "log.h"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
HERE IS A SHORT PROGRAM TO CALCULATE THE PWM TABLE:

//-----------------------------------------------------------------------------------------------------------------------------
// pwmtable[i] = floor(maxvalue * ((i / (steps - 1)) ^ (1/gamma))w + 0.5)
//
// See also:
// http://www.mikrocontroller.net/articles/Diskussion:LED-Fading#Diskussion_wissenschaftl.-technischer_Hintergrund
// http://www.maxim-ic.com/app-notes/index.mvp/id/3667
//
// Compile it with: cc gamma.c -o gamma -lm
//-----------------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>

#define STEPS       (64)            // CHANGE HERE
#define PWMBITS     (8)             // CHANGE HERE

// GAMMA:
// 0.5   für punktförmige oder aufblitzende Helligkeiten
// 0.33  für Lichtquellen bei 5° Blickwinkel
// 1/2.2 für diffus strahlende LEDs - entspricht der Gammakorrektur von VGA-Bildschirmen
#define GAMMA       (1/2.2)

int main ()
{
    int     maxvalue;
    double  value;
    int     i;

    maxvalue = (1 << PWMBITS) - 1;

    for (i = 0; i < STEPS; i++)
    {
        value = floor ((double) maxvalue * pow ((double) i / (double) (STEPS - 1), 1/(GAMMA)) + 0.5);
        if (i > 0 && value < 1)
        {
            value = 1;
        }

        printf ("%5.0f", value);

        if (i < STEPS - 1)
        {
            putchar (',');
            putchar (' ');
        }

        if (!((i + 1) % 8))
        {
            putchar ('\n');
        }
    }
}

 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * 8-Bit PWM with 64 different settings:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
const uint16_t pwmtable8[MAX_COLOR_STEPS]  =
{
    0,     1,     1,     2,     2,     3,     3,     4,
    4,     5,     5,     6,     7,     8,     9,    11,
   13,    14,    16,    18,    20,    23,    25,    28,
   31,    33,    36,    40,    43,    46,    50,    54,
   57,    61,    66,    70,    74,    79,    84,    89,
   94,    99,   105,   110,   116,   122,   128,   134,
  140,   147,   153,   160,   167,   174,   182,   189,
  197,   205,   213,   221,   229,   238,   246,   255
};

static DSP_COLORS       dimmed_display_colors   = DSP_DARK_RED_COLOR;
static DSP_COLORS       dimmed_ambilight_colors = DSP_DARK_RED_COLOR;

#define MAX_TICKER_DECELERATION             16
#define DEFAULT_TICKER_DECELERATION         3

#if WCLOCK24H == 1                                              // faster
#  define ANIMATION_NONE_DEFAULT_DEC        2
#  define ANIMATION_FADE_DEFAULT_DEC        4
#  define ANIMATION_ROLL_DEFAULT_DEC        2
#  define ANIMATION_EXPLODE_DEFAULT_DEC     2
#  define ANIMATION_RANDOM_DEFAULT_DEC      2
#  define ANIMATION_SNAKE_DEFAULT_DEC       2
#  define ANIMATION_TELETYPE_DEFAULT_DEC    4
#  define ANIMATION_CUBE_DEFAULT_DEC        2
#  define ANIMATION_MATRIX_DEFAULT_DEC      3
#  define ANIMATION_DROP_DEFAULT_DEC        2
#  define ANIMATION_SQUEEZE_DEFAULT_DEC     4
#  define ANIMATION_FLICKER_DEFAULT_DEC     4
#else                                                           // slower
#  define ANIMATION_NONE_DEFAULT_DEC        4
#  define ANIMATION_FADE_DEFAULT_DEC        4
#  define ANIMATION_ROLL_DEFAULT_DEC        4
#  define ANIMATION_EXPLODE_DEFAULT_DEC     6
#  define ANIMATION_RANDOM_DEFAULT_DEC      4
#  define ANIMATION_SNAKE_DEFAULT_DEC       4
#  define ANIMATION_TELETYPE_DEFAULT_DEC    4
#  define ANIMATION_CUBE_DEFAULT_DEC        4
#  define ANIMATION_MATRIX_DEFAULT_DEC      4
#  define ANIMATION_DROP_DEFAULT_DEC        4
#  define ANIMATION_SQUEEZE_DEFAULT_DEC     4
#  define ANIMATION_FLICKER_DEFAULT_DEC     4
#endif

static void     display_animation_none (void);
static void     display_animation_fade (void);
static void     display_animation_roll (void);
static void     display_animation_explode (void);
static void     display_animation_random (void);
static void     display_animation_snake (void);
static void     display_animation_teletype (void);
static void     display_animation_cube (void);
static void     display_animation_matrix (void);
static void     display_animation_drop (void);
static void     display_animation_squeeze (void);
static void     display_animation_flicker (void);


DISPLAY_GLOBALS display =
{
    0,                                                                  // display_mode
    0,                                                                  // ambilight_mode
    COLOR_ANIMATION_MODE_NONE,                                          // color_animation_mode
    MAX_BRIGHTNESS,                                                     // display_brightness
    MAX_BRIGHTNESS,                                                     // ambilight_brightness
    0,                                                                  // automatic_brightness
    (DISPLAY_FLAGS_PERMANENT_IT_IS | DISPLAY_FLAGS_SYNC_AMBILIGHT),     // display_flags
    ANIMATION_MODE_FADE,                                                // animation_mode
    0,                                                                  // animation_start_flag
    0,                                                                  // animation_stop_flag
    1,                                                                  // power_is_on
    1,                                                                  // ambilight_power_is_on
    0xFF,                                                               // display_icon

    {                                                                   // overlays
        { 0 },                                                          // overlay_intervals
        {                                                               // icons:
            ICON_HEART_ICON_IDX,                                        // OVERLAY_HEART_IDX
            ICON_XMAS_TREE_ICON,                                        // OVERLAY_XMAS_TREE_IDX
            0xFF,                                                       // OVERLAY_DATE_IDX
            ICON_FIREWORKS_ICON,                                        // OVERLAY_FIREWORKS_IDX
            0xFF,                                                       // OVERLAY_TEMPERATURE_IDX
        }
    },

    22,                                                                 // ambilight_led_offset
    60,                                                                 // ambilight_leds

    {                                                                   // display_colors
        MAX_COLOR_STEPS / 2,
        0,
        0,
#if DSP_USE_SK6812_RGBW == 1
        0
#endif
    },

    {                                                                   // ambilight colors
        MAX_COLOR_STEPS / 2,
        0,
        0,
#if DSP_USE_SK6812_RGBW == 1
        0
#endif
    },

    {                                                                   // animations
        {
            "None",
            display_animation_none,
            ANIMATION_NONE_DEFAULT_DEC,
            ANIMATION_NONE_DEFAULT_DEC,
            ANIMATION_FLAG_NONE
        },
        {
            "Fade",
            display_animation_fade,
            ANIMATION_FADE_DEFAULT_DEC,
            ANIMATION_FADE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Roll",
            display_animation_roll,
            ANIMATION_ROLL_DEFAULT_DEC,
            ANIMATION_ROLL_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Explode",
            display_animation_explode,
            ANIMATION_EXPLODE_DEFAULT_DEC,
            ANIMATION_EXPLODE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Random",
            display_animation_random,
            ANIMATION_RANDOM_DEFAULT_DEC,
            ANIMATION_RANDOM_DEFAULT_DEC,
            ANIMATION_FLAG_NONE
        },
        {
            "Snake",
            display_animation_snake,
            ANIMATION_SNAKE_DEFAULT_DEC,
            ANIMATION_SNAKE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Teletype",
            display_animation_teletype,
            ANIMATION_TELETYPE_DEFAULT_DEC,
            ANIMATION_TELETYPE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Cube",
            display_animation_cube,
            ANIMATION_CUBE_DEFAULT_DEC,
            ANIMATION_CUBE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Matrix",
            display_animation_matrix,
            ANIMATION_MATRIX_DEFAULT_DEC,
            ANIMATION_MATRIX_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Drop",
            display_animation_drop,
            ANIMATION_DROP_DEFAULT_DEC,
            ANIMATION_DROP_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Squeeze",
            display_animation_squeeze,
            ANIMATION_SQUEEZE_DEFAULT_DEC,
            ANIMATION_SQUEEZE_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        },
        {
            "Flicker",
            display_animation_flicker,
            ANIMATION_FLICKER_DEFAULT_DEC,
            ANIMATION_FLICKER_DEFAULT_DEC,
            ANIMATION_FLAG_CONFIGURABLE | ANIMATION_FLAG_FAVOURITE
        }
    },

    {                                                                   // color_animations[COLOR_ANIMATION_MODES];
        { "None",       1, 1, COLOR_ANIMATION_FLAG_NONE         },
        { "Rainbow",    4, 4, COLOR_ANIMATION_FLAG_CONFIGURABLE }
    },

    {                                                                   // ambilight_modes[AMBILIGHT_MODES]
        { "Normal",   0,  0,  AMBILIGHT_FLAG_NONE           },
        { "Clock",    0,  0,  AMBILIGHT_FLAG_NONE           },
        { "Clock2",   0,  0,  AMBILIGHT_FLAG_NONE,          },
        { "Rainbow",  4,  4,  AMBILIGHT_FLAG_CONFIGURABLE   }
    },

    {                                                                   // dimmed_colors: 15 configurable dimmed values + 1 fixed value
        8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, MAX_BRIGHTNESS
    },
    DEFAULT_TICKER_DECELERATION,                                        // ticker deceleration
};

static DSP_COLORS                       dimmed_display_colors_up;
static DSP_COLORS                       dimmed_display_colors_down;

#define CURRENT_STATE                   0x01
#define TARGET_STATE                    0x02
#define NEW_STATE                       0x04
#define CALC_STATE                      0x08

static union
{
    uint8_t                             state[DSP_DISPLAY_LEDS];
    uint8_t                             matrix[WC_ROWS][WC_COLUMNS];
} led;

static uint_fast8_t                     red_step;
static uint_fast8_t                     green_step;
static uint_fast8_t                     blue_step;
#if DSP_USE_SK6812_RGBW == 1
static uint_fast8_t                     white_step;
#endif

#define MAX_TICKER_LEN  64

static unsigned char                    ticker_str[MAX_TICKER_LEN + 1];
static uint_fast8_t                     ticker_col = 0;
static unsigned char *                  ticker_ptr = ticker_str;

#define TICKER_DEGREE                   1
#define TICKER_UP_A_UMLAUT              2
#define TICKER_UP_O_UMLAUT              3
#define TICKER_UP_U_UMLAUT              4
#define TICKER_LO_A_UMLAUT              5
#define TICKER_LO_O_UMLAUT              6
#define TICKER_LO_U_UMLAUT              7
#define TICKER_SHARP_S                  8

#if WCLOCK24H == 1

#define TICKER_LINES                    8
#define TICKER_COLS                     8

static const char ticker_font[128][TICKER_LINES] =          //  8x8 font: only 7 Bit ASCII + some special characters
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //   0
    { 0x3C, 0x42, 0x42, 0x3C, 0x00, 0x00, 0x00, 0x00 },     //   1 degree
    { 0x24, 0x18, 0x24, 0x42, 0x7e, 0x42, 0x42, 0x00 },     //   2 A umlaut
    { 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x00 },     //   3 O umlaut
    { 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00 },     //   4 U umlaut
    { 0x24, 0x18, 0x24, 0x42, 0x7e, 0x42, 0x42, 0x00 },     //   5 a umlaut
    { 0x24, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x00 },     //   6 o umlaut
    { 0x24, 0x00, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00 },     //   7 u umlaut
    { 0x7C, 0x42, 0x7C, 0x42, 0x7C, 0x40, 0x40, 0x00 },     //   8 sharp s
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //   9
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  10
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  11
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  12
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  13
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  14
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  15
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  16
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  17
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  18
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  19
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  20
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  21
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  22
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  23
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  24
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  25
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  26
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  27
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  28
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  29
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  30
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  31
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  32
    { 0x38, 0x38, 0x38, 0x10, 0x00, 0x38, 0x38, 0x00 },     //  33 !
    { 0xee, 0xee, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  34 "
    { 0x28, 0x28, 0xfe, 0x28, 0xfe, 0x28, 0x28, 0x00 },     //  35 #
    { 0x7c, 0x92, 0x90, 0x7c, 0x12, 0x92, 0x7c, 0x00 },     //  36 $
    { 0xe2, 0xa4, 0xe8, 0x10, 0x2e, 0x4a, 0x8e, 0x00 },     //  37 %
    { 0x30, 0x48, 0x30, 0x70, 0x8a, 0x84, 0x7a, 0x00 },     //  38 &
    { 0x38, 0x38, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00 },     //  39 '
    { 0x18, 0x20, 0x40, 0x40, 0x40, 0x20, 0x18, 0x00 },     //  40 (
    { 0x30, 0x08, 0x04, 0x04, 0x04, 0x08, 0x30, 0x00 },     //  41 )
    { 0x00, 0x44, 0x28, 0xee, 0x28, 0x44, 0x00, 0x00 },     //  42 *
    { 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00 },     //  43 +
    { 0x00, 0x00, 0x00, 0x38, 0x38, 0x10, 0x20, 0x00 },     //  44 ,
    { 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00 },     //  45 -
    { 0x00, 0x00, 0x00, 0x00, 0x38, 0x38, 0x38, 0x00 },     //  46 .
    { 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00 },     //  47 /
    { 0x38, 0x44, 0xa2, 0x92, 0x8a, 0x44, 0x38, 0x00 },     //  48 0
    { 0x10, 0x30, 0x50, 0x10, 0x10, 0x10, 0x7c, 0x00 },     //  49 1
    { 0x7c, 0x82, 0x02, 0x7c, 0x80, 0x80, 0xfe, 0x00 },     //  50 2
    { 0x7c, 0x82, 0x02, 0x7c, 0x02, 0x82, 0x7c, 0x00 },     //  51 3
    { 0x80, 0x84, 0x84, 0xfe, 0x04, 0x04, 0x04, 0x00 },     //  52 4
    { 0xfe, 0x80, 0x80, 0x7c, 0x02, 0x82, 0x7c, 0x00 },     //  53 5
    { 0x7c, 0x82, 0x80, 0xfc, 0x82, 0x82, 0x7c, 0x00 },     //  54 6
    { 0xfe, 0x84, 0x08, 0x10, 0x20, 0x20, 0x20, 0x00 },     //  55 7
    { 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c, 0x00 },     //  56 8
    { 0x7c, 0x82, 0x82, 0x7e, 0x02, 0x82, 0x7c, 0x00 },     //  57 9
    { 0x10, 0x28, 0x10, 0x00, 0x10, 0x28, 0x10, 0x00 },     //  58 :
    { 0x38, 0x38, 0x00, 0x38, 0x38, 0x10, 0x20, 0x00 },     //  59 ;
    { 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x00 },     //  60 <
    { 0x00, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0x00, 0x00 },     //  61 =
    { 0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00 },     //  62 >
    { 0x7c, 0x82, 0x02, 0x0c, 0x10, 0x00, 0x10, 0x00 },     //  63 ?
    { 0x7c, 0x82, 0xba, 0xaa, 0xbc, 0x82, 0x7c, 0x00 },     //  64 @
    { 0x10, 0x28, 0x44, 0x82, 0xfe, 0x82, 0x82, 0x00 },     //  65 A
    { 0xfc, 0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc, 0x00 },     //  66 B
    { 0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00 },     //  67 C
    { 0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x00 },     //  68 D
    { 0xfe, 0x80, 0x80, 0xf8, 0x80, 0x80, 0xfe, 0x00 },     //  69 E
    { 0xfe, 0x80, 0x80, 0xf8, 0x80, 0x80, 0x80, 0x00 },     //  70 F
    { 0x7c, 0x82, 0x80, 0x9e, 0x82, 0x82, 0x7c, 0x00 },     //  71 G
    { 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x00 },     //  72 H
    { 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00 },     //  73 I
    { 0x02, 0x02, 0x02, 0x02, 0x82, 0x82, 0x7c, 0x00 },     //  74 J
    { 0x84, 0x88, 0x90, 0xe0, 0x90, 0x88, 0x84, 0x00 },     //  75 K
    { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00 },     //  76 L
    { 0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x00 },     //  77 M
    { 0x82, 0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x82, 0x00 },     //  78 N
    { 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00 },     //  79 O fm: 0xfe -> 0x7c
    { 0xfc, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00 },     //  80 P
    { 0x7c, 0x82, 0x82, 0x82, 0x8a, 0x84, 0x7a, 0x00 },     //  81 Q
    { 0xfc, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00 },     //  82 R
    { 0x7c, 0x82, 0x80, 0x7c, 0x02, 0x82, 0x7c, 0x00 },     //  83 S
    { 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 },     //  84 T
    { 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00 },     //  85 U
    { 0x82, 0x82, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00 },     //  86 V
    { 0x82, 0x92, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00 },     //  87 W
    { 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00 },     //  88 X
    { 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x10, 0x00 },     //  89 Y
    { 0xfe, 0x04, 0x08, 0x10, 0x20, 0x40, 0xfe, 0x00 },     //  90 Z
    { 0x7c, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7c, 0x00 },     //  91 [
    { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 },     //  92 backslash
    { 0x7c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x7c, 0x00 },     //  93 ]
    { 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00 },     //  94 ^
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00 },     //  95 _
    { 0x38, 0x38, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00 },     //  96 `
    { 0x00, 0x18, 0x24, 0x42, 0x7e, 0x42, 0x42, 0x00 },     //  97 a
    { 0x00, 0x7c, 0x42, 0x7c, 0x42, 0x42, 0x7c, 0x00 },     //  98 b
    { 0x00, 0x3c, 0x42, 0x40, 0x40, 0x42, 0x3c, 0x00 },     //  99 c
    { 0x00, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x7c, 0x00 },     // 100 d
    { 0x00, 0x7e, 0x40, 0x7c, 0x40, 0x40, 0x7e, 0x00 },     // 101 e
    { 0x00, 0x7e, 0x40, 0x7c, 0x40, 0x40, 0x40, 0x00 },     // 102 f
    { 0x00, 0x3c, 0x42, 0x40, 0x4e, 0x42, 0x3c, 0x00 },     // 103 g
    { 0x00, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x00 },     // 104 h
    { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 },     // 105 i
    { 0x00, 0x02, 0x02, 0x02, 0x02, 0x42, 0x3c, 0x00 },     // 106 j
    { 0x00, 0x42, 0x44, 0x78, 0x48, 0x44, 0x42, 0x00 },     // 107 k
    { 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7e, 0x00 },     // 108 l
    { 0x00, 0x42, 0x66, 0x5a, 0x42, 0x42, 0x42, 0x00 },     // 109 m
    { 0x00, 0x42, 0x62, 0x52, 0x4a, 0x46, 0x42, 0x00 },     // 110 n
    { 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00 },     // 111 o
    { 0x00, 0x7c, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x00 },     // 112 p
    { 0x00, 0x3c, 0x42, 0x42, 0x4a, 0x44, 0x3a, 0x00 },     // 113 q
    { 0x00, 0x7c, 0x42, 0x42, 0x7c, 0x44, 0x42, 0x00 },     // 114 r
    { 0x00, 0x3c, 0x40, 0x3c, 0x02, 0x42, 0x3c, 0x00 },     // 115 s
    { 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 },     // 116 t
    { 0x00, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00 },     // 117 u
    { 0x00, 0x42, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00 },     // 118 v
    { 0x00, 0x42, 0x42, 0x42, 0x5a, 0x66, 0x42, 0x00 },     // 119 w
    { 0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00 },     // 120 x
    { 0x00, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00 },     // 121 y
    { 0x00, 0x7e, 0x04, 0x08, 0x10, 0x20, 0x7e, 0x00 },     // 122 z
    { 0x38, 0x40, 0x40, 0xc0, 0x40, 0x40, 0x38, 0x00 },     // 123 {
    { 0x10, 0x10, 0x10, 0x00, 0x10, 0x10, 0x10, 0x00 },     // 124 |
    { 0x38, 0x04, 0x04, 0x06, 0x04, 0x04, 0x38, 0x00 },     // 125 }
    { 0x60, 0x92, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 126 ~
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }      // 127 DEL
};

#else // 12h:

#define TICKER_LINES                8
#define TICKER_COLS                 5

static const char ticker_font[128][TICKER_LINES] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x00
    { 0x0C, 0x12, 0x12, 0x0C, 0x00, 0x00, 0x00, 0x00 },     // 0x01 degree
    { 0x0A, 0x00, 0x04, 0x0A, 0x11, 0x1F, 0x11, 0x00 },         // 0x02 A umlaut
    { 0x12, 0x0C, 0x12, 0x12, 0x12, 0x12, 0x0C, 0x00 },     // 0x03 O umlaut
    { 0x0A, 0x00, 0x12, 0x12, 0x12, 0x12, 0x0C, 0x00 },     // 0x04 U umlaut
    { 0x0A, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00 },     // 0x05 a umlaut
    { 0x0A, 0x00, 0x0C, 0x12, 0x12, 0x12, 0x0C, 0x00 },     // 0x06 o umlaut
    { 0x12, 0x00, 0x12, 0x12, 0x12, 0x16, 0x0A, 0x00 },     // 0x07 u umlaut
    { 0x00, 0x1C, 0x12, 0x1C, 0x12, 0x12, 0x1C, 0x10 },     // 0x08 sharp s
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x09
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x0F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x10
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x11
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x12
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x13
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x14
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x15
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x16
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x17
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x18
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x19
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x1F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x20
    { 0x04, 0x0E, 0x0E, 0x04, 0x04, 0x00, 0x04, 0x00 },     // 0x21
    { 0x1B, 0x1B, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x22
    { 0x00, 0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00 },     // 0x23
    { 0x08, 0x0E, 0x10, 0x0C, 0x02, 0x1C, 0x04, 0x00 },     // 0x24
    { 0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13, 0x00 },     // 0x25
    { 0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D, 0x00 },     // 0x26
    { 0x0C, 0x0C, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x27
    { 0x04, 0x08, 0x08, 0x08, 0x08, 0x08, 0x04, 0x00 },     // 0x28
    { 0x08, 0x04, 0x04, 0x04, 0x04, 0x04, 0x08, 0x00 },     // 0x29
    { 0x00, 0x0A, 0x0E, 0x1F, 0x0E, 0x0A, 0x00, 0x00 },     // 0x2A
    { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00 },     // 0x2B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x08 },     // 0x2C
    { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00 },     // 0x2D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00 },     // 0x2E
    { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00 },     // 0x2F
    { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E, 0x00 },     // 0x30
    { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00 },     // 0x31
    { 0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F, 0x00 },     // 0x32
    { 0x0E, 0x11, 0x01, 0x0E, 0x01, 0x11, 0x0E, 0x00 },     // 0x33
    { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02, 0x00 },     // 0x34
    { 0x1F, 0x10, 0x10, 0x1E, 0x01, 0x11, 0x0E, 0x00 },     // 0x35
    { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E, 0x00 },     // 0x36
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08, 0x00 },     // 0x37
    { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00 },     // 0x38
    { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C, 0x00 },     // 0x39
    { 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00 },     // 0x3A
    { 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x08 },     // 0x3B
    { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02, 0x00 },     // 0x3C
    { 0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 0x00 },     // 0x3D
    { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00 },     // 0x3E
    { 0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04, 0x00 },     // 0x3F
    { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E, 0x00 },     // 0x40
    { 0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00 },     // 0x41
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E, 0x00 },     // 0x42
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E, 0x00 },     // 0x43
    { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E, 0x00 },     // 0x44
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F, 0x00 },     // 0x45
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10, 0x00 },     // 0x46
    { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F, 0x00 },     // 0x47
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00 },     // 0x48
    { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00 },     // 0x49
    { 0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E, 0x00 },     // 0x4A
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11, 0x00 },     // 0x4B
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F, 0x00 },     // 0x4C
    { 0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11, 0x00 },     // 0x4D
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11, 0x00 },     // 0x4E
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00 },     // 0x4F
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10, 0x00 },     // 0x50
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D, 0x00 },     // 0x51
    { 0x1E, 0x11, 0x11, 0x1E, 0x12, 0x11, 0x11, 0x00 },     // 0x52
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E, 0x00 },     // 0x53
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 },     // 0x54
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00 },     // 0x55
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00 },     // 0x56
    { 0x11, 0x11, 0x15, 0x15, 0x15, 0x15, 0x0A, 0x00 },     // 0x57
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00 },     // 0x58
    { 0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00 },     // 0x59
    { 0x1E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x1E, 0x00 },     // 0x5A
    { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E, 0x00 },     // 0x5B
    { 0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00 },     // 0x5C
    { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E, 0x00 },     // 0x5D
    { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x5E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F },     // 0x5F
    { 0x0C, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x60
    { 0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00 },     // 0x61
    { 0x10, 0x10, 0x1E, 0x11, 0x11, 0x11, 0x1E, 0x00 },     // 0x62
    { 0x00, 0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E, 0x00 },     // 0x63
    { 0x01, 0x01, 0x0F, 0x11, 0x11, 0x11, 0x0F, 0x00 },     // 0x64
    { 0x00, 0x00, 0x0E, 0x11, 0x1E, 0x10, 0x0E, 0x00 },     // 0x65
    { 0x06, 0x08, 0x08, 0x1E, 0x08, 0x08, 0x08, 0x00 },     // 0x66
    { 0x00, 0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x0E },     // 0x67
    { 0x10, 0x10, 0x1C, 0x12, 0x12, 0x12, 0x12, 0x00 },     // 0x68
    { 0x04, 0x00, 0x04, 0x04, 0x04, 0x04, 0x06, 0x00 },     // 0x69
    { 0x02, 0x00, 0x06, 0x02, 0x02, 0x02, 0x12, 0x0C },     // 0x6A
    { 0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12, 0x00 },     // 0x6B
    { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x06, 0x00 },     // 0x6C
    { 0x00, 0x00, 0x1A, 0x15, 0x15, 0x11, 0x11, 0x00 },     // 0x6D
    { 0x00, 0x00, 0x1C, 0x12, 0x12, 0x12, 0x12, 0x00 },     // 0x6E
    { 0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00 },     // 0x6F
    { 0x00, 0x00, 0x1E, 0x11, 0x11, 0x11, 0x1E, 0x10 },     // 0x70
    { 0x00, 0x00, 0x0F, 0x11, 0x11, 0x11, 0x0F, 0x01 },     // 0x71
    { 0x00, 0x00, 0x16, 0x09, 0x08, 0x08, 0x1C, 0x00 },     // 0x72
    { 0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x0E, 0x00 },     // 0x73
    { 0x00, 0x08, 0x1E, 0x08, 0x08, 0x0A, 0x04, 0x00 },     // 0x74
    { 0x00, 0x00, 0x12, 0x12, 0x12, 0x16, 0x0A, 0x00 },     // 0x75
    { 0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00 },     // 0x76
    { 0x00, 0x00, 0x11, 0x11, 0x15, 0x1F, 0x0A, 0x00 },     // 0x77
    { 0x00, 0x00, 0x12, 0x12, 0x0C, 0x12, 0x12, 0x00 },     // 0x78
    { 0x00, 0x00, 0x12, 0x12, 0x12, 0x0E, 0x04, 0x18 },     // 0x79
    { 0x00, 0x00, 0x1E, 0x02, 0x0C, 0x10, 0x1E, 0x00 },     // 0x7A
    { 0x06, 0x08, 0x08, 0x18, 0x08, 0x08, 0x06, 0x00 },     // 0x7B
    { 0x04, 0x04, 0x04, 0x00, 0x04, 0x04, 0x04, 0x00 },     // 0x7C
    { 0x0C, 0x02, 0x02, 0x03, 0x02, 0x02, 0x0C, 0x00 },     // 0x7D
    { 0x0A, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },     // 0x7E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }      // 0x7F DEL
};

#endif // WCLOCK24H == 1

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * refresh status LED
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_refresh_status_led (void)
{
    led_refresh (DSP_STATUS_LEDS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * refresh minute LEDs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if DSP_MINUTE_LEDS != 0
static void
display_refresh_minute_leds (void)
{
    led_refresh (DSP_STATUS_LEDS + DSP_MINUTE_LEDS);
}
#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * refresh display LEDs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_refresh_display_leds (void)
{
    led_refresh (DSP_STATUS_LEDS + DSP_MINUTE_LEDS + DSP_DISPLAY_LEDS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * refresh ambilight LEDs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_refresh_ambilight_leds (void)
{
    if (display.ambilight_power_is_on)
    {
        led_refresh (DSP_STATUS_LEDS + DSP_MINUTE_LEDS + DSP_DISPLAY_LEDS + DSP_AMBILIGHT_LEDS);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set status LED
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_status_led (uint_fast8_t r_flag, uint_fast8_t g_flag, uint_fast8_t b_flag)
{
    if (DSP_STATUS_LEDS > 0)
    {
        LED_RGB rgb;

        rgb.red     = r_flag ? pwmtable8[MAX_COLOR_STEPS - 1] : 0;
        rgb.green   = g_flag ? pwmtable8[MAX_COLOR_STEPS - 1] : 0;
        rgb.blue    = b_flag ? pwmtable8[MAX_COLOR_STEPS - 1] : 0;
#if DSP_USE_SK6812_RGBW == 1
        rgb.white   = 0;                                                        // white not used for status LED
#endif

        led_set_led (DSP_STATUS_LED_OFFSET, &rgb);
        display_refresh_status_led ();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display minute LEDs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if DSP_MINUTE_LEDS != 0
static uint_fast8_t last_minute;
static uint_fast8_t last_power_is_on;

static void
display_minute_leds (uint_fast8_t tmp_power_is_on, uint_fast8_t minute)
{
    LED_RGB         rgb;
    LED_RGB         rgb0;
    uint_fast8_t    n_leds;
    uint_fast8_t    i;

    last_minute = minute;
    last_power_is_on = tmp_power_is_on;

    if (DSP_MINUTE_LEDS > 0)
    {
        RESET_LED_RGB(rgb0);

        if (tmp_power_is_on)
        {
            CALC_LED_RGB(rgb, dimmed_display_colors);
            n_leds = minute % 5;

            for (i = 0; i < DSP_MINUTE_LEDS; i++)
            {
                if (i < n_leds)
                {
                    led_set_led (DSP_MINUTE_LED_OFFSET + i, &rgb);
                }
                else
                {
                    led_set_led (DSP_MINUTE_LED_OFFSET + i, &rgb0);
                }
            }
        }
        else
        {
            for (i = 0; i < DSP_MINUTE_LEDS; i++)
            {
                led_set_led (DSP_MINUTE_LED_OFFSET + i, &rgb0);
            }
        }

        display_refresh_minute_leds ();
    }
}
#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display LED to RGB
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_display_led (uint_fast16_t n, LED_RGB * rgb, uint_fast8_t refresh)
{
    if (n < DSP_DISPLAY_LEDS)
    {
        uint_fast8_t y;
        uint_fast8_t x;

        y = n / WC_COLUMNS;

        if (y & 0x01)                                       // snake: odd row: count from right to left
        {
            x = n % WC_COLUMNS;
            n = y * WC_COLUMNS + (WC_COLUMNS - 1 - x);
        }

        led_set_led (DSP_DISPLAY_LED_OFFSET + n, rgb);

        if (refresh)
        {
            display_refresh_display_leds ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight LED to RGB
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_set_ambilight_led (uint_fast16_t n, LED_RGB * rgb, uint_fast8_t refresh)
{
    if (n < DSP_AMBILIGHT_LEDS)
    {
        led_set_led (DSP_AMBILIGHT_LED_OFFSET + n, rgb);
    }

    if (refresh)
    {
        display_refresh_ambilight_leds();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight LEDs to RGB
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_set_ambilight_leds (LED_RGB * rgb, uint_fast8_t refresh)
{
    uint_fast16_t   n;

    for (n = 0; n < DSP_AMBILIGHT_LEDS; n++)
    {
        led_set_led (DSP_AMBILIGHT_LED_OFFSET + n, rgb);
    }

    if (refresh)
    {
        display_refresh_ambilight_leds();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * switch all LEDs off
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_reset_led_states (void)
{
    uint_fast16_t     idx;

    for (idx = 0; idx < DSP_DISPLAY_LEDS; idx++)
    {
        if (led.state[idx] & TARGET_STATE)
        {
            led.state[idx] = CURRENT_STATE;
        }
        else
        {
            led.state[idx] = 0;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display one LED
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_led_on (uint_fast8_t y, uint_fast8_t x)
{
    led.matrix[y][x] |= TARGET_STATE;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display one word
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_word_on (uint_fast8_t idx)
{
    uint_fast8_t y = illumination[idx].row;
    uint_fast8_t x = illumination[idx].col;
    uint_fast8_t l = illumination[idx].len;

    while (l--)
    {
        display_led_on (y, x);
        x++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * calc dimmed colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
calc_dimmed_colors (DSP_COLORS * dimmed_colors, const DSP_COLORS * colors, uint_fast8_t brightness, uint_fast8_t use_dimmed_colors)
{
    if (brightness >= MAX_BRIGHTNESS)
    {
        dimmed_colors->red   = colors->red;
        dimmed_colors->green = colors->green;
        dimmed_colors->blue  = colors->blue;
#if DSP_USE_SK6812_RGBW == 1
        dimmed_colors->white = colors->white;
#endif
    }
    else
    {
        uint_fast8_t    factor;

        if (use_dimmed_colors)
        {
            factor = display.dimmed_colors[brightness];
        }
        else
        {
            factor = brightness;
        }

        dimmed_colors->red = (colors->red * factor) / MAX_BRIGHTNESS;

        if (colors->red > 0 && dimmed_colors->red == 0)
        {
            dimmed_colors->red = 1;
        }

        dimmed_colors->green = (colors->green * factor) / MAX_BRIGHTNESS;

        if (colors->green > 0 && dimmed_colors->green == 0)
        {
            dimmed_colors->green = 1;
        }

        dimmed_colors->blue = (colors->blue * factor) / MAX_BRIGHTNESS;

        if (colors->blue > 0 && dimmed_colors->blue == 0)
        {
            dimmed_colors->blue = 1;
        }

#if DSP_USE_SK6812_RGBW == 1
        dimmed_colors->white = (colors->white * factor) / MAX_BRIGHTNESS;

        if (colors->white > 0 && dimmed_colors->white == 0)
        {
            dimmed_colors->white = 1;
        }
#endif
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display_show_new_display - show new display
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_show_new_display (uint_fast8_t mask)
{
    LED_RGB          rgb;
    LED_RGB          rgb0;
    uint_fast16_t    idx;

    CALC_LED_RGB(rgb, dimmed_display_colors);
    RESET_LED_RGB(rgb0);

    for (idx = 0; idx < DSP_DISPLAY_LEDS; idx++)
    {
        if (led.state[idx] & mask)                                              // on
        {
            display_set_display_led (idx, &rgb, 0);
        }
        else
        {
            display_set_display_led (idx, &rgb0, 0);
        }
    }

    display_refresh_display_leds ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display_set_new_states - copy target states to new states
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_set_new_states (void)
{
    uint_fast16_t idx;

    for (idx = 0; idx < DSP_DISPLAY_LEDS; idx++)                                // final state: activate new LEDs
    {
        if (led.state[idx] & TARGET_STATE)
        {
            led.state[idx] |= NEW_STATE;
        }
        else
        {
            led.state[idx] &= ~NEW_STATE;
        }
    }
}

static void
display_show_ambilight_normal_mode (uint_fast8_t refresh)
{
    LED_RGB rgb;

    CALC_LED_RGB(rgb, dimmed_ambilight_colors);
    display_set_ambilight_leds (&rgb, refresh);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * flush animation
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_flush (uint_fast8_t flush_ambi)
{
    static uint_fast8_t already_called;
    LED_RGB          rgb;
    LED_RGB          rgb0;
    uint_fast16_t    idx;

    if (! already_called)
    {
        flush_ambi = 1;
        already_called = 1;
    }

    CALC_LED_RGB(rgb, dimmed_display_colors);
    RESET_LED_RGB(rgb0);

    for (idx = 0; idx < DSP_DISPLAY_LEDS; idx++)
    {
        if (led.state[idx] & TARGET_STATE)
        {
            display_set_display_led (idx, &rgb, 0);
            led.state[idx] |= CURRENT_STATE;                        // we are in sync
        }
        else
        {
            display_set_display_led (idx, &rgb0, 0);
            led.state[idx] &= ~CURRENT_STATE;                       // we are in sync
        }
    }

#if DSP_MINUTE_LEDS != 0
    display_minute_leds (last_power_is_on, last_minute);
#endif

    if (flush_ambi && display.ambilight_mode == AMBILIGHT_MODE_NORMAL)
    {
        display_show_ambilight_normal_mode (1);
    }
    else
    {
        display_refresh_display_leds ();
    }

    display.animation_stop_flag = 1;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: no animation
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_none (void)
{
    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display_animation_flush (FALSE);
    }

    display.animation_stop_flag = 1;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: fade
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_fade (void)
{
    uint_fast16_t           idx;
    uint_fast8_t            changed = 0;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;

        red_step = dimmed_display_colors.red / 20;

        if (red_step == 0 && dimmed_display_colors.red > 0)
        {
            red_step = 1;
        }

        green_step = dimmed_display_colors.green / 20;

        if (green_step == 0 && dimmed_display_colors.green > 0)
        {
            green_step = 1;
        }

        blue_step = dimmed_display_colors.blue / 20;

        if (blue_step == 0 && dimmed_display_colors.blue > 0)
        {
            blue_step = 1;
        }

#if DSP_USE_SK6812_RGBW == 1
        white_step = dimmed_display_colors.white / 20;

        if (white_step == 0 && dimmed_display_colors.white > 0)
        {
            white_step = 1;
        }
#endif

        dimmed_display_colors_up.red       = 0;
        dimmed_display_colors_up.green     = 0;
        dimmed_display_colors_up.blue      = 0;
#if DSP_USE_SK6812_RGBW == 1
        dimmed_display_colors_up.white     = 0;
#endif

        dimmed_display_colors_down.red     = dimmed_display_colors.red;
        dimmed_display_colors_down.green   = dimmed_display_colors.green;
        dimmed_display_colors_down.blue    = dimmed_display_colors.blue;
#if DSP_USE_SK6812_RGBW == 1
        dimmed_display_colors_down.white   = dimmed_display_colors.white;
#endif
    }

    if (! display.animation_stop_flag)
    {
        if (red_step > 0)
        {
            if (dimmed_display_colors_down.red >= red_step)
            {
                dimmed_display_colors_down.red -= red_step;
                changed = 1;
            }
            else if (dimmed_display_colors_down.red != 0)
            {
                dimmed_display_colors_down.red = 0;
                changed = 1;
            }

            if (dimmed_display_colors_up.red + red_step <= dimmed_display_colors.red)
            {
                dimmed_display_colors_up.red += red_step;
                changed = 1;
            }
            else if (dimmed_display_colors_up.red != dimmed_display_colors.red)
            {
                dimmed_display_colors_up.red = dimmed_display_colors.red;
                changed = 1;
            }
        }

        if (green_step > 0)
        {
            if (dimmed_display_colors_down.green >= green_step)
            {
                dimmed_display_colors_down.green -= green_step;
                changed = 1;
            }
            else if (dimmed_display_colors_down.green != 0)
            {
                dimmed_display_colors_down.green = 0;
                changed = 1;
            }

            if (dimmed_display_colors_up.green + green_step <= dimmed_display_colors.green)
            {
                dimmed_display_colors_up.green += green_step;
                changed = 1;
            }
            else if (dimmed_display_colors_up.green != dimmed_display_colors.green)
            {
                dimmed_display_colors_up.green = dimmed_display_colors.green;
                changed = 1;
            }
        }

        if (blue_step > 0)
        {
            if (dimmed_display_colors_down.blue >= blue_step)
            {
                dimmed_display_colors_down.blue -= blue_step;
                changed = 1;
            }
            else if (dimmed_display_colors_down.blue != 0)
            {
                dimmed_display_colors_down.blue = 0;
                changed = 1;
            }

            if (dimmed_display_colors_up.blue + blue_step <= dimmed_display_colors.blue)
            {
                dimmed_display_colors_up.blue += blue_step;
                changed = 1;
            }
            else if (dimmed_display_colors_up.blue != dimmed_display_colors.blue)
            {
                dimmed_display_colors_up.blue = dimmed_display_colors.blue;
                changed = 1;
            }
        }

#if DSP_USE_SK6812_RGBW == 1
        if (white_step > 0)
        {
            if (dimmed_display_colors_down.white >= white_step)
            {
                dimmed_display_colors_down.white -= white_step;
                changed = 1;
            }
            else if (dimmed_display_colors_down.white != 0)
            {
                dimmed_display_colors_down.white = 0;
                changed = 1;
            }

            if (dimmed_display_colors_up.white + white_step <= dimmed_display_colors.white)
            {
                dimmed_display_colors_up.white += white_step;
                changed = 1;
            }
            else if (dimmed_display_colors_up.white != dimmed_display_colors.white)
            {
                dimmed_display_colors_up.white = dimmed_display_colors.white;
                changed = 1;
            }
        }
#endif

        if (changed)
        {
            LED_RGB  rgb;
            LED_RGB  rgb_up;
            LED_RGB  rgb_down;

            CALC_LED_RGB(rgb, dimmed_display_colors);
            CALC_LED_RGB(rgb_up, dimmed_display_colors_up);
            CALC_LED_RGB(rgb_down, dimmed_display_colors_down);

            for (idx = 0; idx < DSP_DISPLAY_LEDS; idx++)
            {
                if (led.state[idx] == TARGET_STATE)                           // up
                {
                    display_set_display_led (idx, &rgb_up, 0);
                }
                else if (led.state[idx] == CURRENT_STATE)                     // down
                {
                    display_set_display_led (idx, &rgb_down, 0);
                }
                else if (led.state[idx] == (CURRENT_STATE | TARGET_STATE))    // on, but no change
                {
                    display_set_display_led (idx, &rgb, 0);
                }
            }

            display_refresh_display_leds ();
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: roll right
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_roll_right (void)
{
    static uint_fast16_t    cnt;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_COLUMNS)
        {
            cnt++;                                                  // 1...WC_COLUMNS

            for (y = 0; y < WC_ROWS * WC_COLUMNS; y += WC_COLUMNS)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if (x >= cnt)
                    {
                        if (led.state[y + x - cnt] & CURRENT_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                    else
                    {
                        if (led.state[y + x + WC_COLUMNS - cnt] & TARGET_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                }
            }

            display_show_new_display (NEW_STATE);
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: roll left
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_roll_left (void)
{
    static uint_fast16_t    cnt;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_COLUMNS)
        {
            cnt++;                                                  // 1...WC_COLUMNS

            for (y = 0; y < WC_ROWS * WC_COLUMNS; y += WC_COLUMNS)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if (x + cnt < WC_COLUMNS)
                    {
                        if (led.state[y + x + cnt] & CURRENT_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                    else
                    {
                        if (led.state[y + x + cnt - WC_COLUMNS] & TARGET_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                }
            }

            display_show_new_display (NEW_STATE);
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: roll down
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_roll_down (void)
{
    static uint_fast16_t    cnt;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_ROWS * WC_COLUMNS)
        {
            cnt += WC_COLUMNS;                                  // (1...WC_ROWS) * WC_COLUMNS

            for (y = 0; y < WC_ROWS * WC_COLUMNS; y += WC_COLUMNS)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if (y >= cnt)
                    {
                        if (led.state[y - cnt + x] & CURRENT_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                    else
                    {
                        if (led.state[y + (WC_ROWS * WC_COLUMNS - cnt) + x] & TARGET_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                }
            }
            display_show_new_display (NEW_STATE);
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: roll up
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_roll_up (void)
{
    static uint_fast16_t    cnt;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_ROWS * WC_COLUMNS)
        {
            cnt += WC_COLUMNS;                                  // (1...WC_ROWS) * WC_COLUMNS

            for (y = 0; y < WC_ROWS * WC_COLUMNS; y += WC_COLUMNS)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if (y + cnt < WC_ROWS * WC_COLUMNS)
                    {
                        if (led.state[y + cnt + x] & CURRENT_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                    else
                    {
                        if (led.state[y + (cnt - WC_ROWS * WC_COLUMNS) + x] & TARGET_STATE)
                        {
                            led.state[y + x] |= NEW_STATE;
                        }
                        else
                        {
                            led.state[y + x] &= ~NEW_STATE;
                        }
                    }
                }
            }
            display_show_new_display (NEW_STATE);
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: roll
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_roll (void)
{
    static uint32_t        x;

    if (display.animation_start_flag)
    {
        x = rand () % 4;
    }

    switch (x)
    {
        case 0:     display_animation_roll_right ();       break;
        case 1:     display_animation_roll_left  ();       break;
        case 2:     display_animation_roll_down  ();       break;
        case 3:     display_animation_roll_up    ();       break;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: explode - helper functions
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_calc_implode (int n)
{
    uint_fast8_t    y;
    uint_fast16_t   yi;
    uint_fast8_t    x;
    uint_fast16_t   xi;
    uint_fast8_t    ny;
    uint_fast8_t    nx;

    for (yi = 0; yi < WC_ROWS * WC_COLUMNS; yi += WC_COLUMNS)
    {
        for (xi = 0; xi < WC_COLUMNS; xi++)
        {
            led.state[yi + xi] &= ~CALC_STATE;
        }
    }

    for (y = 0; y < WC_ROWS; y++)
    {
        for (x = 0; x < WC_COLUMNS; x++)
        {
            if (led.matrix[y][x] & TARGET_STATE)
            {
                if (y < WC_ROWS / 2)
                {
                    if (x < WC_COLUMNS / 2)
                    {
                        ny = y + n;
                        nx = x + n;

                        if (ny > WC_ROWS / 2 - 1)
                        {
                            ny = WC_ROWS / 2 - 1;
                        }

                        if (nx > WC_COLUMNS / 2 - 1)
                        {
                            nx = WC_COLUMNS / 2 - 1;
                        }
                    }
                    else
                    {
                        ny = y + n;
                        nx = x - n;

                        if (ny > WC_ROWS / 2 - 1)
                        {
                            ny = WC_ROWS / 2 - 1;
                        }

                        if (nx < WC_COLUMNS / 2)
                        {
                            nx = WC_COLUMNS / 2;
                        }
                    }
                }
                else
                {
                    if (x < WC_COLUMNS / 2)
                    {
                        ny = y - n;
                        nx = x + n;

                        if (ny < WC_ROWS / 2)
                        {
                            ny = WC_ROWS / 2;
                        }

                        if (nx > WC_COLUMNS / 2 - 1)
                        {
                            nx = WC_COLUMNS / 2 - 1;
                        }
                    }
                    else
                    {
                        ny = y - n;
                        nx = x - n;

                        if (ny < WC_ROWS / 2)
                        {
                            ny = WC_ROWS / 2;
                        }

                        if (nx < WC_COLUMNS / 2)
                        {
                            nx = WC_COLUMNS / 2;
                        }
                    }
                }

                led.matrix[ny][nx] |= CALC_STATE;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: explode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_explode (void)
{
    static uint_fast8_t cnt;
    uint_fast8_t        y;
    uint_fast16_t       yi;
    uint_fast8_t        x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_COLUMNS / 2)
        {
            for (y = 0; y < WC_ROWS; y++)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    led.matrix[y][x] &= ~NEW_STATE;
                }
            }

            cnt++;

            display_animation_calc_implode (WC_COLUMNS / 2 - cnt);

            for (y = 0, yi = 0; y < WC_ROWS; y++, yi += WC_COLUMNS)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if (led.matrix[y][x] & CURRENT_STATE)
                    {
                        if (y < WC_ROWS / 2)
                        {
                            if (x < WC_COLUMNS / 2)
                            {
                                if (y >= cnt && x >= cnt)
                                {
                                    led.matrix[y - cnt][x - cnt] |= NEW_STATE;
                                }
                            }
                            else
                            {
                                if (y >= cnt && x + cnt < WC_COLUMNS)
                                {
                                    led.matrix[y - cnt][x + cnt] |= NEW_STATE;
                                }
                            }
                        }
                        else
                        {
                            if (x < WC_COLUMNS / 2)
                            {
                                if (y + cnt < WC_ROWS && x >= cnt)
                                {
                                    led.matrix[y + cnt][x - cnt] |= NEW_STATE;
                                }
                            }
                            else
                            {
                                if (y + cnt < WC_ROWS && x + cnt < WC_COLUMNS)
                                {
                                    led.matrix[y + cnt][x + cnt] |= NEW_STATE;
                                }
                            }
                        }
                    }

                    if (led.state[yi + x] & CALC_STATE)
                    {
                        led.state[yi + x] |= NEW_STATE;        // matrix leds |= calculated leds
                    }
                }
            }

            display_show_new_display (NEW_STATE);
        }
        else
        {
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: snake - helper functions
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define SNAKE_LEN                   (WC_COLUMNS - 5)

static uint_fast8_t
animation_snake_next_word_in_line (uint_fast8_t start_y, uint_fast8_t start_x, uint_fast8_t * next_y, uint_fast8_t * next_x)
{
    uint_fast8_t x;

    if (start_y & 0x01)
    {
        for (x = start_x + 1; x > 0; x--)
        {
            if (led.matrix[start_y][x - 1] & CURRENT_STATE)
            {
                *next_y = start_y;
                *next_x = x - 1;
                return 1;
            }
        }
    }
    else
    {
        for (x = start_x; x < WC_COLUMNS; x++)
        {
            if (led.matrix[start_y][x] & CURRENT_STATE)
            {
                *next_y = start_y;
                *next_x = x;
                return 1;
            }
        }
    }

    return 0;
}

static uint_fast8_t
animation_snake_search_next_word (uint_fast8_t start_y, uint_fast8_t start_x, uint_fast8_t * next_y, uint_fast8_t * next_x)
{
    uint_fast8_t y;
    uint_fast8_t x;

    if (start_y & 0x01)
    {
        if (start_x > 0)
        {
            start_x--;
        }
        else
        {
            start_y++;
        }
    }
    else
    {
        if (start_x < WC_COLUMNS - 1)
        {
            start_x++;
        }
        else
        {
            start_y++;
        }
    }

    for (y = start_y; y < WC_ROWS; y++)
    {
        if (y == start_y)
        {
            x = start_x;
        }
        else
        {
            if (y & 0x01)
            {
                x = WC_COLUMNS - 1;
            }
            else
            {
                x = 0;
            }
        }

        if (animation_snake_next_word_in_line (y, x, next_y, next_x))
        {
            return 1;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: snake
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_snake (void)
{
    static LED_RGB          snake_led_rgb;
    static LED_RGB          snake_led_rgb0;
    static uint8_t          snakepos_y[SNAKE_LEN];
    static uint8_t          snakepos_x[SNAKE_LEN];
    static uint_fast16_t    snake_y;
    static uint_fast16_t    snake_x;
    static uint_fast16_t    next_y;
    static uint_fast16_t    next_x;
    static uint_fast16_t    len = 0;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        len     = 0;
        snake_y = 0;
        snake_x = 0;
        next_y = 0;
        next_x = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (len == 0)
        {
            DSP_COLORS   dsp_rgb;
            DSP_COLORS   dsp_rgb_dimmed;

            if (display.display_colors.red < MAX_COLOR_STEPS / 2)
            {
                SET_DSP_RGB(dsp_rgb, MAX_COLOR_STEPS - 1, 0, 0, 0);                                     // use red as snake color
            }
            else if (display.display_colors.green < MAX_COLOR_STEPS / 2)
            {
                SET_DSP_RGB(dsp_rgb, 0, MAX_COLOR_STEPS - 1, 0, 0);                                     // use green as snake color
            }
            else if (display.display_colors.blue < MAX_COLOR_STEPS / 2)
            {
                SET_DSP_RGB(dsp_rgb, 0, 0, MAX_COLOR_STEPS - 1, 0);                                     // use blue as snake color
            }
            else
            {
                SET_DSP_RGB(dsp_rgb, 0, MAX_COLOR_STEPS - 1, MAX_COLOR_STEPS - 1, 0);                   // use cyan as snake color
            }

            calc_dimmed_colors (&dsp_rgb_dimmed, &dsp_rgb, display.display_brightness, TRUE);

            CALC_LED_RGB(snake_led_rgb, dsp_rgb_dimmed);
            RESET_LED_RGB(snake_led_rgb0);

            // led.matrix[snake_y][snake_x] |= NEW_STATE;
            display_set_display_led (snake_y * WC_COLUMNS + snake_x, &snake_led_rgb, 0);                // LED on

            snakepos_y[len] = snake_y;
            snakepos_x[len] = snake_x;

            len++;

            display_refresh_display_leds ();
        }
        else
        {
            if (snake_y >= WC_ROWS + SNAKE_LEN)
            {
                display_set_new_states ();
                display_show_new_display (NEW_STATE);
                display.animation_stop_flag = 1;
            }
            else
            {
                if (snake_y == next_y && snake_x == next_x)
                {
                    if (! animation_snake_search_next_word (snake_y, snake_x, &next_y, &next_x))
                    {
                        next_y = WC_ROWS + SNAKE_LEN;
                    }
                }

                if ((snake_y & 0x01) && snake_x > next_x)
                {
                    snake_x--;
                }
                else if (!(snake_y & 0x01) && snake_x < next_x)
                {
                    snake_x++;
                }
                else if (snake_y < next_y)
                {
                    snake_y++;
                }
                else
                {                                                   // error, never should be
                    display.animation_stop_flag = 1;
                }

                if (snake_y < WC_ROWS)
                {
                    // led.matrix[snake_y][snake_x] |= NEW_STATE;
                    display_set_display_led (snake_y * WC_COLUMNS + snake_x, &snake_led_rgb, 0);                        // LED on
                }

                if (len == SNAKE_LEN)
                {
                    int i;

                    if (snakepos_y[0] < WC_ROWS)
                    {
                        //led.matrix[snakepos_y[0]][snakepos_x[0]] &= ~NEW_STATE;
                        display_set_display_led (snakepos_y[0] * WC_COLUMNS + snakepos_x[0] , &snake_led_rgb0, 0);      // LED off
                    }

                    for (i = 0; i < SNAKE_LEN - 1; i++)
                    {
                        snakepos_y[i] = snakepos_y[i + 1];
                        snakepos_x[i] = snakepos_x[i + 1];
                    }

                    len--;
                }

                snakepos_y[len] = snake_y;
                snakepos_x[len] = snake_x;

                if (len < SNAKE_LEN)
                {
                    len++;
                }

                display_refresh_display_leds ();
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: teletype
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_teletype (void)
{
    static uint_fast16_t            prescaler;
    static uint_fast16_t            cnt;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
        prescaler = 0;
    }

    if (! display.animation_stop_flag)
    {
        uint16_t    n;

        if (cnt == 0)                                               // switch off complete display
        {
            for (n = 0; n < WC_ROWS * WC_COLUMNS; n++)
            {
                led.state[n] &= ~NEW_STATE;
            }

            display_show_new_display (NEW_STATE);
            cnt++;                                                  // next cell
        }
        else
        {
            prescaler++;

            if (prescaler % 2)
            {
                n = cnt - 1;

                while (n < WC_ROWS * WC_COLUMNS)
                {
                    if (led.state[n] & TARGET_STATE)
                    {
                        led.state[n] |= NEW_STATE;                      // found a letter to illuminate: switch on and break;
                        display_show_new_display (NEW_STATE);
                        break;
                    }

                    n++;
                }

                if (n >= WC_ROWS * WC_COLUMNS)
                {
                    display.animation_stop_flag = 1;
                }
                else
                {
                    cnt = n + 1;                                        // store back cnt
                }

                cnt++;                                                  // next cell
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: cube
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_cube (void)
{
    static uint_fast16_t            cnt;
    uint_fast16_t                   y;
    uint_fast16_t                   x;
    uint_fast16_t                   n;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < WC_ROWS / 2)
        {
            n = cnt;

            for (y = n, x = n; x < WC_COLUMNS - n; x++)                     // draw cube
            {
                led.matrix[y][x] |= NEW_STATE;
            }

            for (y = n + 1; y < WC_ROWS - n - 1; y++)
            {
                x = n;
                led.matrix[y][x] |= NEW_STATE;

                x = WC_COLUMNS - n - 1;
                led.matrix[y][x] |= NEW_STATE;
            }

            for (y = WC_ROWS - n - 1, x = n; x < WC_COLUMNS - n; x++)
            {
                led.matrix[y][x] |= NEW_STATE;
            }
        }

        // erase outer cube, but leave target LEDs

        if (cnt < WC_ROWS / 2 + 1)
        {
            if (cnt > 0)                                                // outer cube existant?
            {
                n = cnt - 1;                                            // yes...

                for (y = n, x = n; x < WC_COLUMNS - n; x++)             // erase outer cube
                {
                    led.matrix[y][x] &= ~NEW_STATE;
                }

                for (y = n + 1; y < WC_ROWS - n - 1; y++)
                {
                    x = n;
                    led.matrix[y][x] &= ~NEW_STATE;

                    x = WC_COLUMNS - n - 1;
                    led.matrix[y][x] &= ~NEW_STATE;
                }

                for (y = WC_ROWS - n - 1, x = n; x < WC_COLUMNS - n; x++)
                {
                    led.matrix[y][x] &= ~NEW_STATE;
                }
            }

            display_show_new_display (NEW_STATE);
            cnt++;
        }
        else
        {
            display_set_new_states ();
            display_show_new_display (NEW_STATE);
            display.animation_stop_flag = 1;
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: matrix
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_matrix (void)
{
    static uint_fast16_t    cnt;
    static uint_fast8_t     y[WC_COLUMNS];
    static LED_RGB          led_rgb_yellow;
    static LED_RGB          led_rgb_green;
    uint_fast8_t            x;

    if (display.animation_start_flag)
    {
        DSP_COLORS          dsp_rgb_yellow;
        DSP_COLORS          dsp_rgb_green;
        DSP_COLORS          dsp_rgb_yellow_dimmed;
        DSP_COLORS          dsp_rgb_green_dimmed;

        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;

        SET_DSP_RGB(dsp_rgb_yellow, MAX_COLOR_STEPS - 1, MAX_COLOR_STEPS - 1, 0, 0);                // yellow
        SET_DSP_RGB(dsp_rgb_green, 0, MAX_COLOR_STEPS - 1, MAX_COLOR_STEPS / 4, 0);                 // green with a piece of blue

        calc_dimmed_colors (&dsp_rgb_yellow_dimmed, &dsp_rgb_yellow, display.display_brightness, TRUE);
        calc_dimmed_colors (&dsp_rgb_green_dimmed, &dsp_rgb_green, display.display_brightness, TRUE);

        CALC_LED_RGB(led_rgb_yellow, dsp_rgb_yellow_dimmed);
        CALC_LED_RGB(led_rgb_green, dsp_rgb_green_dimmed);

        for (x = 0; x < WC_COLUMNS; x++)
        {
            if (x & 0x01)
            {
                y[x] = rand () % (WC_ROWS);
            }
            else
            {
                y[x] = (rand () % WC_ROWS) + WC_ROWS / 2;
            }
        }
    }

    if (! display.animation_stop_flag)
    {
        if (cnt < 3 * WC_ROWS)
        {
            LED_RGB      led_rgb;
            uint_fast8_t yy;

            COPY_LED_RGB(led_rgb, led_rgb_green);

            for (x = 0; x < WC_COLUMNS; x++)
            {
                yy = y[x];

                if (yy > WC_ROWS + WC_ROWS / 2)
                {
                    uint_fast8_t i;

                    yy -= WC_ROWS + WC_ROWS / 2;

                    if (yy > 1)
                    {
                        if (yy - 2 < WC_ROWS)
                        {
                            display_set_display_led ((yy - 2) * WC_COLUMNS + x, &led_rgb_green, 0);
                        }
                    }

                    if (yy < WC_ROWS)
                    {
                        display_set_display_led (yy * WC_COLUMNS + x, &led_rgb_yellow, 0);
                    }

                    for (i = 1; yy > 0; i++)
                    {
                        yy--;

                        if (yy < WC_ROWS)
                        {
                            led_rgb.green = led_rgb_green.green / i;
                            display_set_display_led (yy * WC_COLUMNS + x, &led_rgb, 0);
                        }

                        if (yy > 0)
                        {
                            yy--;

                            if (yy > 0 && yy < WC_ROWS)
                            {
                                led_rgb.green = led_rgb_green.green / i;
                                display_set_display_led (yy * WC_COLUMNS + x, &led_rgb, 0);
                            }
                        }
                    }
                }

                y[x]++;
            }

            display_refresh_display_leds ();

            cnt++;
        }
        else
        {
            display_set_new_states ();
            display_show_new_display (NEW_STATE);
            display.animation_stop_flag = 1;
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: drop
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_drop (void)
{
    static uint_fast8_t     drop_old;
    static uint_fast8_t     drop_new;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        drop_old = TRUE;
        drop_new = TRUE;
    }

    if (! display.animation_stop_flag)
    {
        uint_fast8_t    letter_to_drop_found = FALSE;

        if (drop_old)
        {
            y = WC_ROWS;

            while (y > 0)
            {
                y--;

                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if ((led.matrix[y][x] & CURRENT_STATE) && ! (led.matrix[y][x] & TARGET_STATE))      // old letter
                    {
                        letter_to_drop_found = TRUE;
                        led.matrix[y][x] &= ~CURRENT_STATE;

                        if (y < WC_ROWS - 1)
                        {
                            led.matrix[y + 1][x] |= CURRENT_STATE;
                        }
                        break;
                    }
                }

                if (letter_to_drop_found)
                {
                    break;
                }
            }

            if (! letter_to_drop_found)
            {
                drop_old = FALSE;
            }

            display_show_new_display (NEW_STATE | CURRENT_STATE);
        }
        else if (drop_new)
        {
            static uint_fast8_t     dropping = FALSE;
            static uint_fast16_t    drop_from_y;
            static uint_fast16_t    drop_to_y;
            static uint_fast16_t    drop_x;

            if (dropping)
            {
                letter_to_drop_found = TRUE;

                led.matrix[drop_from_y][drop_x] &= ~NEW_STATE;              // target in first row: simply switch on

                drop_from_y++;

                led.matrix[drop_from_y][drop_x] |= NEW_STATE;               // target in first row: simply switch on

                if (drop_from_y == drop_to_y)
                {
                    led.matrix[drop_from_y][drop_x] |= CURRENT_STATE;       // mark it as ready
                    dropping = FALSE;
                }
            }
            else
            {
                y = WC_ROWS;

                while (y > 0)
                {
                    y--;

                    for (x = 0; x < WC_COLUMNS && ! letter_to_drop_found; x++)
                    {
                        if ((led.matrix[y][x] & TARGET_STATE) && ! (led.matrix[y][x] & CURRENT_STATE))      // new letter
                        {
                            letter_to_drop_found = TRUE;
                            drop_to_y = y;
                            drop_x = x;

                            if (y > 0)
                            {
                                uint_fast16_t yy = y;

                                while (yy > 0)
                                {
                                    yy--;

                                    if (led.matrix[y][x] & CURRENT_STATE)       // drop from existing letter
                                    {
                                        led.matrix[y + 1][x] |= NEW_STATE;
                                        drop_from_y = y + 1;
                                        dropping = TRUE;
                                        break;
                                    }
                                }

                                if (! dropping)                                 // drop from first row
                                {
                                    led.matrix[0][x] |= NEW_STATE;
                                    drop_from_y = 0;
                                    dropping = TRUE;
                                }
                            }
                            else
                            {
                                led.matrix[0][x] |= CURRENT_STATE | NEW_STATE;  // target in first row: simply switch on
                            }
                        }
                    }
                }
            }

            if (! letter_to_drop_found)
            {
                drop_new = FALSE;
            }

            display_show_new_display (NEW_STATE | CURRENT_STATE);
        }
        else
        {
            display_set_new_states ();                      // practically not necessary, because all cells should be now on right place
            display_show_new_display (NEW_STATE);           // but I've seen at least one LED being on at the end, which should be off
            display.animation_stop_flag = 1;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: squeeze
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_squeeze (void)
{
    static uint_fast16_t    state;
    uint_fast16_t           y;
    uint_fast16_t           x;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        state = 0;
    }

    if (! display.animation_stop_flag)
    {
        uint_fast8_t led_found = 0;

        if (state == 0)
        {
            for (y = 0; y < WC_ROWS; y++)
            {
                for (x = WC_COLUMNS; x > 0; )
                {
                    x--;

                    if (led.matrix[y][x] & CURRENT_STATE)
                    {
                        if ((x == WC_COLUMNS - 1) || !(led.matrix[y][x + 1] & CURRENT_STATE))
                        {
                            led.matrix[y][x] &= ~CURRENT_STATE;

                            while (x > 0)
                            {
                                x--;

                                if (!(led.matrix[y][x + 1] & CURRENT_STATE))
                                {
                                    break;
                                }
                            }
                        }

                        led_found = 1;
                    }
                }
            }

            display_show_new_display (CURRENT_STATE);

            if (! led_found)
            {
                state = 1;
            }
        }

        if (state == 1)                                                         // NOT else
        {
            for (y = 0; y < WC_ROWS; y++)
            {
                for (x = 0; x < WC_COLUMNS; x++)
                {
                    if ((led.matrix[y][x] & TARGET_STATE) && !(led.matrix[y][x] & NEW_STATE))
                    {
                        if ((x == 0) || !(led.matrix[y][x] & NEW_STATE))
                        {
                            led.matrix[y][x] |= NEW_STATE;

                            while (x < WC_COLUMNS - 1)
                            {
                                if (!(led.matrix[y][x + 1] & TARGET_STATE))
                                {
                                    break;
                                }
                                x++;
                            }

                            led_found = 1;
                        }
                    }
                }
            }

            display_show_new_display (NEW_STATE);

            if (! led_found)
            {
                display.animation_stop_flag = 1;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animation: flicker
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_flicker (void)
{
    static uint_fast16_t    cnt;
    uint_fast16_t           n;
    uint_fast8_t            flicker_on;

    if (display.animation_start_flag)
    {
        display.animation_start_flag = 0;
        display.animation_stop_flag = 0;
        cnt = 0;
    }

    if (! display.animation_stop_flag)
    {
        cnt++;

        if ((rand () & 0x07) > 1)
        {
            flicker_on = 1;
        }
        else
        {
            flicker_on = 0;
        }

        for (n = 0; n < DSP_DISPLAY_LEDS; n++)
        {
            if (flicker_on)
            {
                if ((led.state[n] & CURRENT_STATE) || (led.state[n] & TARGET_STATE))
                {
                    led.state[n] |= NEW_STATE;
                }
                else
                {
                    led.state[n] &= ~NEW_STATE;
                }
            }
            else
            {
                led.state[n] &= ~NEW_STATE;
            }
        }

        if (cnt == 32)
        {
            display.animation_stop_flag = 1;
            display_show_new_display (TARGET_STATE);
        }
        else
        {
            display_show_new_display (NEW_STATE);
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * random animation
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_animation_random ()
{
    static uint_fast16_t    deceleration_cnt;
    static uint32_t         x;

    if (display.animation_start_flag)
    {
        uint_fast8_t    map_idx[ANIMATION_MODES];
        uint_fast8_t    map_size = 0;
        uint_fast8_t    idx;

        for (idx = 0; idx < ANIMATION_MODES; idx++)
        {
            if (display.animations[idx].flags & ANIMATION_FLAG_FAVOURITE)
            {
                map_idx[map_size++] = idx;
            }
        }

        if (map_size == 0)
        {
            x = ANIMATION_MODE_NONE;
        }
        else
        {
            x = rand () %  map_size;
            x = map_idx[x];
        }

        display.animations[x].func ();
        deceleration_cnt = 0;
    }
    else
    {
        deceleration_cnt++;

        if (deceleration_cnt >= display.animations[x].deceleration)
        {
            deceleration_cnt = 0;
            display.animations[x].func ();
        }
    }
}

static void
display_show_ambilight_off (uint_fast8_t refresh)
{
    LED_RGB         rgb0;

    RESET_LED_RGB(rgb0);
    display_set_ambilight_leds (&rgb0, refresh);
}

void
display_set_ambilight_power (uint_fast8_t on)
{
    if (on)
    {
        display.ambilight_power_is_on = 1;
        display_refresh_ambilight_leds ();
    }
    else
    {
        display_show_ambilight_off (1);
        display.ambilight_power_is_on = 0;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display clock time
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_clock (uint_fast8_t hour, uint_fast8_t minute, uint_fast8_t display_clock_flag)
{
    static uint8_t                  words[WP_COUNT];
    uint_fast8_t                    hour_mode;
    uint_fast8_t                    minute_mode;
    uint_fast8_t                    it_is_mode;
#if WCLOCK24H == 1
    const struct MinuteDisplay *    tbl_minute;
#else
    const struct MinuteDisplay12 *  tbl_minute;
#endif
    const uint8_t *                 word_idx_p;
    uint_fast16_t                   idx;

    if (display_clock_flag)
    {
        display_animation_flush (FALSE);

        if (display_clock_flag & DISPLAY_CLOCK_FLAG_POWER_ON)
        {
            power_on ();
            delay_msec (200);                                                   // wait 0.2 sec for LEDs coming up

            if (display.ambilight_mode == AMBILIGHT_MODE_NORMAL)
            {
                display_show_ambilight_normal_mode (1);
            }
        }
        else if (display_clock_flag & DISPLAY_CLOCK_FLAG_POWER_OFF)
        {
            display_show_ambilight_off (1);
        }

#if WCLOCK24H == 1
        minute_mode = tbl_modes[display.display_mode].minute_txt;
        tbl_minute  = &tbl_minutes[minute_mode][minute];
        hour_mode   = tbl_modes[display.display_mode].hour_txt;
#else
        display_minute_leds (display.power_is_on, minute);
        minute_mode = display.display_mode;
        tbl_minute  = &tbl_minutes[minute_mode][minute / 5];
        hour_mode   = tbl_minute->hour_mode;
#endif

        if (display_clock_flag & DISPLAY_CLOCK_FLAG_UPDATE_ALL)
        {
            memset (words, 0, WP_COUNT);
            display_reset_led_states ();

            if (display.power_is_on)
            {
#if WCLOCK24H == 1
                if ((display.display_flags & DISPLAY_FLAGS_PERMANENT_IT_IS) || minute == 0 || minute == 30)
#else
                if ((display.display_flags & DISPLAY_FLAGS_PERMANENT_IT_IS) || minute <= 4 || (minute >= 30 && minute <= 34))
#endif
                {
                    it_is_mode = tbl_minute->flags & MDF_IT_IS_MASK;
                    word_idx_p = tbl_it_is[it_is_mode];
                    words[word_idx_p[0]] = 1;
                    words[word_idx_p[1]] = 1;
                }

                for (idx = 0; idx < MAX_MINUTE_WORDS && tbl_minute->wordIdx[idx] != 0; idx++)
                {
                    words[tbl_minute->wordIdx[idx]] = 1;
                }

#if WCLOCK24H == 0                                                          // WC12h: we have only 12 hours
                if (hour >= HOUR_COUNT)
                {
                    hour -= HOUR_COUNT;
                }
#endif

                if (tbl_minute->flags & MDF_HOUR_OFFSET_MASK)
                {
                    hour += 1;                                              // correct hour offset
                }

                if (hour >= HOUR_COUNT)
                {
                    hour -= HOUR_COUNT;
                }

                word_idx_p = tbl_hours[hour_mode][hour];                    // get the hour words from hour table

                for (idx = 0; idx < MAX_HOUR_WORDS && word_idx_p[idx] != 0; idx++)
                {
#if WCLOCK24H == 1                                                          // WC24h:
                    if (minute == 0 && word_idx_p[idx] == WP_EINS_2)        // fm 2016-08-23 - dirty hack, because it cannot be handled by table:
                    {                                                       // patch "ES IST EINS UHR" to "ES IST EIN UHR" at 01:00 & 13:00
                        words[WP_EIN_4] = 1;
                    }
                    else
                    {
                        words[word_idx_p[idx]] = 1;
                    }
#else
                    words[word_idx_p[idx]] = 1;
#endif
                }

                for (idx = 0; idx < WP_COUNT; idx++)
                {
                    if (words[idx])
                    {
                        display_word_on (idx);
                    }
                }

                display.animation_start_flag = 1;                                   // start animation if power is on
            }
            else if (display_clock_flag & DISPLAY_CLOCK_FLAG_POWER_OFF)
            {
                display.animation_start_flag = 1;                                   // start animation if power is switching off
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * sync display colors with ambilight colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
sync_colors (void)
{
    display.ambilight_brightness = display.display_brightness;

    COPY_DSP_RGB(display.ambilight_colors, display.display_colors);
    COPY_DSP_RGB(dimmed_ambilight_colors, dimmed_display_colors);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * calculate dimmed ambilight colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_calc_dimmed_ambilight_colors (void)
{
    calc_dimmed_colors (&dimmed_ambilight_colors, &display.ambilight_colors, display.ambilight_brightness, TRUE);

    if (display.ambilight_power_is_on)
    {
        LED_RGB          rgb;

        CALC_LED_RGB(rgb, dimmed_ambilight_colors);
        display_set_ambilight_leds (&rgb, 1);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize ambilight mode rainbow
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_init_ambilight_mode_rainbow (void)
{
    SET_DSP_RGB(display.ambilight_colors, MAX_COLOR_STEPS - 1, 0, 0, 0);            // red
    display_calc_dimmed_ambilight_colors ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize ambilight mode clock
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t     ambilight_clock_increasing;

static void
display_init_ambilight_mode_clock (void)
{
    display_show_ambilight_off (0);                                         // switch all ambilight LEDs off, but don't refresh here
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize ambilight mode clock2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_init_ambilight_mode_clock2 (void)
{
    ambilight_clock_increasing = 1;

    display_show_ambilight_off (0);                                         // switch all ambilight LEDs off, but don't refresh here
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize ambilight mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_init_ambilight_mode (void)
{
    switch (display.ambilight_mode)
    {
        case AMBILIGHT_MODE_NORMAL:
            display_show_ambilight_normal_mode (display.ambilight_power_is_on);
            break;
        case AMBILIGHT_MODE_CLOCK:
            display_init_ambilight_mode_clock ();
            break;
        case AMBILIGHT_MODE_CLOCK2:
            display_init_ambilight_mode_clock2 ();
            break;
        case AMBILIGHT_MODE_RAINBOW:
            display_init_ambilight_mode_rainbow ();
            break;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * calculate dimmed colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_calc_dimmed_display_colors (void)
{
    calc_dimmed_colors (&dimmed_display_colors, &display.display_colors, display.display_brightness, TRUE);

    if (display.ambilight_mode != AMBILIGHT_MODE_RAINBOW && (display.display_flags & DISPLAY_FLAGS_SYNC_AMBILIGHT))
    {
        sync_colors ();
    }

    if (display.power_is_on)
    {
        display_animation_flush (TRUE);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize display rainbow color animation
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_init_color_animation_rainbow (void)
{
    SET_DSP_RGB(display.display_colors, MAX_COLOR_STEPS - 1, 0, 0, 0);
    display_calc_dimmed_display_colors ();
}

static void
display_init_color_animation (void)
{
    switch (display.color_animation_mode)
    {
        case COLOR_ANIMATION_MODE_RAINBOW:  display_init_color_animation_rainbow (); break;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read configuration from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_read_config_from_eeprom (uint32_t eeprom_version)
{
    uint_fast8_t                rtc = 0;

    if (eeprom_is_up)
    {
        uint8_t                 display_rgb_color_buf8[EEPROM_DATA_SIZE_DSP_COLORS];
        uint8_t                 display_w_color8 = 0;
        uint8_t                 ambilight_rgb_color_buf8[EEPROM_DATA_SIZE_AMBI_COLORS] = { MAX_COLOR_STEPS - 1, 0, 0 };
        uint8_t                 ambilight_w_color8 = 0;
        uint8_t                 animation_values8[ANIMATION_MODES];
        uint8_t                 color_animation_values8[COLOR_ANIMATION_MODES];
        uint8_t                 ambilight_mode_values8[AMBILIGHT_MODES];
        uint8_t                 display_mode8;
        uint8_t                 ambilight_mode8;
        uint8_t                 ambilight_leds8                 = 60;
        uint8_t                 ambilight_led_offset8           = 22;
        uint8_t                 animation_mode8                 = AMBILIGHT_MODE_NORMAL;
        uint8_t                 color_animation_mode8           = 0;
        uint8_t                 display_flags8                  = DISPLAY_FLAGS_PERMANENT_IT_IS | DISPLAY_FLAGS_SYNC_AMBILIGHT;
        uint8_t                 display_brightness8;
        uint8_t                 ambilight_brightness8           = MAX_BRIGHTNESS;
        uint8_t                 automatic_brightness8;
        uint8_t                 ticker_deceleration8            = 3;
        uint_fast8_t            disp_intervals                  = 0;
        uint_fast8_t            save_power_is_on;
        uint_fast8_t            idx;

        rtc = 1;

        eeprom_read (EEPROM_DATA_OFFSET_DSP_COLORS,         display_rgb_color_buf8,     EEPROM_DATA_SIZE_DSP_COLORS);
        eeprom_read (EEPROM_DATA_OFFSET_DISPLAY_MODE,       &display_mode8,             EEPROM_DATA_SIZE_DISPLAY_MODE);
        eeprom_read (EEPROM_DATA_OFFSET_ANIMATION_MODE,     &animation_mode8,           EEPROM_DATA_SIZE_ANIMATION_MODE);
        eeprom_read (EEPROM_DATA_OFFSET_BRIGHTNESS,         &display_brightness8,       EEPROM_DATA_SIZE_BRIGHTNESS);
        eeprom_read (EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS,    &automatic_brightness8,     EEPROM_DATA_SIZE_AUTO_BRIGHTNESS);

        if (eeprom_version >= EEPROM_VERSION_1_8)
        {
            eeprom_read (EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE, &color_animation_mode8, EEPROM_DATA_SIZE_COLOR_ANIMATION_MODE);
        }

        if (eeprom_version >= EEPROM_VERSION_1_9)
        {
            eeprom_read (EEPROM_DATA_OFFSET_DISPLAY_FLAGS,          &display_flags8,            EEPROM_DATA_SIZE_DISPLAY_FLAGS);
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_COLORS,            ambilight_rgb_color_buf8,   EEPROM_DATA_SIZE_AMBI_COLORS);
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS,        &ambilight_brightness8,     EEPROM_DATA_SIZE_AMBI_BRIGHTNESS);
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_MODE,              &ambilight_mode8,           EEPROM_DATA_SIZE_AMBI_MODE);
        }

        if (eeprom_version >= EEPROM_VERSION_2_0)
        {
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_LEDS,              &ambilight_leds8,           EEPROM_DATA_SIZE_AMBI_LEDS);
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0,       &ambilight_led_offset8,     EEPROM_DATA_SIZE_AMBI_OFFSET_SEC0);
        }

        if (eeprom_version >= EEPROM_VERSION_2_1)
        {
            eeprom_read (EEPROM_DATA_OFFSET_ANIMATION_VALUES,       animation_values8,          ANIMATION_MODES);
            eeprom_read (EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES, color_animation_values8,    COLOR_ANIMATION_MODES);
            eeprom_read (EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES,  ambilight_mode_values8,     AMBILIGHT_MODES);

            for (idx = 0; idx < ANIMATION_MODES; idx++)
            {
                if (animation_values8[idx] != 0xFF)                                                     // yet unused in EEPROM
                {
                    display.animations[idx].deceleration = animation_values8[idx] & EEPROM_ANIMATION_DECELERATION_MASK;
                                                                                                        // lower 4 bits = deceleration

                    if (animation_values8[idx] & EEPROM_ANIMATION_FAVOURITE_MASK)                       // bit 4 = favourite flag
                    {
                        display.animations[idx].flags |= ANIMATION_FLAG_FAVOURITE;
                    }
                    else
                    {
                        display.animations[idx].flags &= ~ANIMATION_FLAG_FAVOURITE;
                    }
                }
            }

            for (idx = 0; idx < COLOR_ANIMATION_MODES; idx++)
            {
                if (color_animation_values8[idx] != 0xFF)                                               // yet unused in EEPROM
                {
                    display.color_animations[idx].deceleration = color_animation_values8[idx] & EEPROM_COLOR_ANIMATION_DECELERATION_MASK;
                }
            }

            for (idx = 0; idx < AMBILIGHT_MODES; idx++)
            {
                if (ambilight_mode_values8[idx] != 0xFF)
                {                                                                       // deceleration in lower nibble
                    display.ambilight_modes[idx].deceleration = ambilight_mode_values8[idx] & EEPROM_AMBILIGHT_DECELERATION_MASK;

                    if (eeprom_version >= EEPROM_VERSION_2_6)
                    {                                                                   // ambilight mode flags in upper nibble
                        if (idx == AMBILIGHT_MODE_CLOCK && ambilight_mode_values8[idx] & (AMBILIGHT_FLAG_SECONDS_MARKER << 4))
                        {
                            display.ambilight_modes[idx].flags |= AMBILIGHT_FLAG_SECONDS_MARKER;
                        }
                    }
                }
            }
        }

        if (eeprom_version >= EEPROM_VERSION_2_2)
        {
            eeprom_read (EEPROM_DATA_OFFSET_DSP_W_COLOR,  &display_w_color8,   EEPROM_DATA_SIZE_DSP_W_COLOR);
            eeprom_read (EEPROM_DATA_OFFSET_AMBI_W_COLOR, &ambilight_w_color8, EEPROM_DATA_SIZE_AMBI_W_COLOR);
        }

        if (eeprom_version == EEPROM_VERSION_2_3)
        {
            disp_intervals = 2;
        }
        else if (eeprom_version == EEPROM_VERSION_2_4)
        {
            disp_intervals = 4;
        }
        else if (eeprom_version > EEPROM_VERSION_2_4)
        {
            uint8_t dimmed_colors8[MAX_BRIGHTNESS + 1];

            eeprom_read (EEPROM_DATA_OFFSET_DIMMED_COLORS, dimmed_colors8, EEPROM_DATA_SIZE_DIMMED_COLORS);

            for (idx = 0; idx < MAX_BRIGHTNESS; idx++)                              // not (MAX_BRIGHTNESS + 1)!
            {
                if (dimmed_colors8[idx] > MAX_BRIGHTNESS)
                {
                    display.dimmed_colors[idx] = MAX_BRIGHTNESS;
                }
                else
                {
                    display.dimmed_colors[idx] = dimmed_colors8[idx];
                }
            }

            display.dimmed_colors[MAX_BRIGHTNESS] = MAX_BRIGHTNESS;             // last member should be MAX_BRIGHTNESS

            disp_intervals = MAX_OVERLAYS;
        }

        if (eeprom_version >= EEPROM_VERSION_2_6)
        {
            eeprom_read (EEPROM_DATA_OFFSET_TICKER_DECELERATION,  &ticker_deceleration8,   EEPROM_DATA_SIZE_TICKER_DECELERATION);
        }

        if (disp_intervals > 0)
        {
            eeprom_read (EEPROM_DATA_OFFSET_OVERLAY_INTERVALS, display.overlays.intervals, disp_intervals);  // only fill MAX_OVERLAYS items!

            for (idx = 0; idx < MAX_OVERLAYS; idx++)
            {
                if (display.overlays.intervals[idx] > 60)                    // reset nonsense values (e.g. uninitialized EEPROM bytes)
                {
                    display.overlays.intervals[idx] = 0;
                }
            }
        }

        if (display_mode8 >= MODES_COUNT)
        {
            display_mode8 = 0;
        }

        if (ambilight_mode8 >= AMBILIGHT_MODES)
        {
            ambilight_mode8 = 0;
        }

        if (animation_mode8 >= ANIMATION_MODES)
        {
            animation_mode8 = 0;
        }

        if (color_animation_mode8 >= COLOR_ANIMATION_MODES)
        {
            color_animation_mode8 = 0;
        }

        if (display_rgb_color_buf8[0] >= MAX_COLOR_STEPS)
        {
            display_rgb_color_buf8[0] = MAX_COLOR_STEPS - 1;
        }
        if (display_rgb_color_buf8[1] >= MAX_COLOR_STEPS)
        {
            display_rgb_color_buf8[1] = MAX_COLOR_STEPS - 1;
        }
        if (display_rgb_color_buf8[2] >= MAX_COLOR_STEPS)
        {
            display_rgb_color_buf8[2] = MAX_COLOR_STEPS - 1;
        }
#if DSP_USE_SK6812_RGBW == 1
        if (display_w_color8 >= MAX_COLOR_STEPS)
        {
            display_w_color8 = MAX_COLOR_STEPS - 1;
        }
#endif

        if (ambilight_rgb_color_buf8[0] >= MAX_COLOR_STEPS)
        {
            ambilight_rgb_color_buf8[0] = MAX_COLOR_STEPS - 1;
        }
        if (ambilight_rgb_color_buf8[1] >= MAX_COLOR_STEPS)
        {
            ambilight_rgb_color_buf8[1] = MAX_COLOR_STEPS - 1;
        }
        if (ambilight_rgb_color_buf8[2] >= MAX_COLOR_STEPS)
        {
            ambilight_rgb_color_buf8[2] = MAX_COLOR_STEPS - 1;
        }
#if DSP_USE_SK6812_RGBW == 1
        if (ambilight_w_color8 >= MAX_COLOR_STEPS)
        {
            ambilight_w_color8 = MAX_COLOR_STEPS - 1;
        }
#endif

        if (display_brightness8 > MAX_BRIGHTNESS)
        {
            display_brightness8 = MAX_BRIGHTNESS;
        }

        if (ambilight_brightness8 > MAX_BRIGHTNESS)
        {
            ambilight_brightness8 = MAX_BRIGHTNESS;
        }

        if (automatic_brightness8 != 0x01)        // only 0x00 or 0x01 allowed, 0xFF means empty EEPROM
        {
            automatic_brightness8 = 0;
        }

        if (ambilight_leds8 > DSP_AMBILIGHT_LEDS)
        {
            ambilight_leds8 = DSP_AMBILIGHT_LEDS;
        }

        if (ambilight_led_offset8 > DSP_AMBILIGHT_LEDS - 1)
        {
            ambilight_led_offset8 = 22;
        }

        if (ticker_deceleration8 > MAX_TICKER_DECELERATION)
        {
            ticker_deceleration8                = DEFAULT_TICKER_DECELERATION;        // one user reported a value of 255.
        }

        display.display_mode                    = display_mode8;
        display.ambilight_mode                  = ambilight_mode8;
        display.display_flags                   = display_flags8;
        display.animation_mode                  = animation_mode8;
        display.color_animation_mode            = color_animation_mode8;

        display.display_colors.red              = display_rgb_color_buf8[0];
        display.display_colors.green            = display_rgb_color_buf8[1];
        display.display_colors.blue             = display_rgb_color_buf8[2];
#if DSP_USE_SK6812_RGBW == 1
        display.display_colors.white            = display_w_color8;
#endif

        display.ambilight_colors.red            = ambilight_rgb_color_buf8[0];
        display.ambilight_colors.green          = ambilight_rgb_color_buf8[1];
        display.ambilight_colors.blue           = ambilight_rgb_color_buf8[2];
#if DSP_USE_SK6812_RGBW == 1
        display.ambilight_colors.white          = ambilight_w_color8;
#endif

        display.display_brightness              = display_brightness8;
        display.ambilight_brightness            = ambilight_brightness8;
        display.automatic_brightness            = automatic_brightness8;

        display.ambilight_leds                  = ambilight_leds8;
        display.ambilight_led_offset            = ambilight_led_offset8;

        save_power_is_on                        = display.power_is_on;
        display.power_is_on                     = FALSE;
        display_calc_dimmed_ambilight_colors ();                    // first calculate dimmed ambilight colors
        display_calc_dimmed_display_colors ();                      // then calculate display colors, may overwrite ambilight colors by sync
        display.power_is_on                     = save_power_is_on;
        display.ticker_deceleration             = ticker_deceleration8;

        display_init_color_animation ();                            // init color animation
        display_init_ambilight_mode ();                             // init ambilight mode
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save display colors in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_display_colors (void)
{
    uint8_t display_rgb_color_buf8[3];

    display_rgb_color_buf8[0] = display.display_colors.red;
    display_rgb_color_buf8[1] = display.display_colors.green;
    display_rgb_color_buf8[2] = display.display_colors.blue;

    eeprom_write (EEPROM_DATA_OFFSET_DSP_COLORS, display_rgb_color_buf8, EEPROM_DATA_SIZE_DSP_COLORS);

#if DSP_USE_SK6812_RGBW == 1
    uint8_t display_w_color8;

    display_w_color8 = display.display_colors.white;
    eeprom_write (EEPROM_DATA_OFFSET_DSP_W_COLOR, &display_w_color8, EEPROM_DATA_SIZE_DSP_W_COLOR);
#endif
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save display mode in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_display_mode (void)
{
    uint8_t     display_mode8;

    display_mode8 = display.display_mode;
    eeprom_write (EEPROM_DATA_OFFSET_DISPLAY_MODE, &display_mode8, EEPROM_DATA_SIZE_DISPLAY_MODE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save display flags in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_display_flags (void)
{
    uint8_t     display_flags8;

    display_flags8 = display.display_flags;
    eeprom_write (EEPROM_DATA_OFFSET_DISPLAY_FLAGS, &display_flags8, EEPROM_DATA_SIZE_DISPLAY_FLAGS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save animation mode in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_animation_mode (void)
{
    uint8_t     animation_mode8;

    animation_mode8 = display.animation_mode;
    eeprom_write (EEPROM_DATA_OFFSET_ANIMATION_MODE, &animation_mode8, EEPROM_DATA_SIZE_ANIMATION_MODE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save color animation mode in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_color_animation_mode (void)
{
    uint8_t     color_animation_mode8;

    color_animation_mode8 = display.color_animation_mode;
    eeprom_write (EEPROM_DATA_OFFSET_COLOR_ANIMATION_MODE, &color_animation_mode8, EEPROM_DATA_SIZE_COLOR_ANIMATION_MODE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save brightness in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_brightness (void)
{
    uint8_t     display_brightness8;

    display_brightness8 = display.display_brightness;
    eeprom_write (EEPROM_DATA_OFFSET_BRIGHTNESS, &display_brightness8, EEPROM_DATA_SIZE_BRIGHTNESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save auto brightness in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_automatic_brightness (void)
{
    uint8_t automatic_brightness8;

    automatic_brightness8 = display.automatic_brightness;
    eeprom_write (EEPROM_DATA_OFFSET_AUTO_BRIGHTNESS, &automatic_brightness8, EEPROM_DATA_SIZE_AUTO_BRIGHTNESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight colors in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_colors (void)
{
    uint8_t     ambilight_rgb_color_buf8[3];

    ambilight_rgb_color_buf8[0] = display.ambilight_colors.red;
    ambilight_rgb_color_buf8[1] = display.ambilight_colors.green;
    ambilight_rgb_color_buf8[2] = display.ambilight_colors.blue;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_COLORS, ambilight_rgb_color_buf8, EEPROM_DATA_SIZE_AMBI_COLORS);

#if DSP_USE_SK6812_RGBW == 1
    uint8_t     ambilight_w_color8;

    ambilight_w_color8 = display.ambilight_colors.white;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_W_COLOR, &ambilight_w_color8, EEPROM_DATA_SIZE_AMBI_W_COLOR);
#endif
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight brightness in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_brightness (void)
{
    uint8_t     ambilight_brightness8;

    ambilight_brightness8 = display.ambilight_brightness;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_BRIGHTNESS, &ambilight_brightness8, EEPROM_DATA_SIZE_AMBI_BRIGHTNESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight mode in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_mode (void)
{
    uint8_t     ambilight_mode8;

    ambilight_mode8 = display.ambilight_mode;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_MODE, &ambilight_mode8, EEPROM_DATA_SIZE_AMBI_MODE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save number of ambilight LEDs in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_number_of_ambilight_leds (void)
{
    uint8_t ambilight_leds8;

    ambilight_leds8 = display.ambilight_leds;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_LEDS, &ambilight_leds8, EEPROM_DATA_SIZE_AMBI_LEDS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight LED offset in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_led_offset (void)
{
    uint8_t     ambilight_led_offset8;

    ambilight_led_offset8 = display.ambilight_led_offset;
    eeprom_write (EEPROM_DATA_OFFSET_AMBI_OFFSET_SEC0, &ambilight_led_offset8, EEPROM_DATA_SIZE_AMBI_OFFSET_SEC0);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save overlay intervals in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_overlay_intervals (void)
{
    eeprom_write (EEPROM_DATA_OFFSET_OVERLAY_INTERVALS, display.overlays.intervals, EEPROM_DATA_SIZE_OVERLAY_INTERVALS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save animation in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_animation (uint_fast8_t idx)
{
    uint8_t animation_value8;

    if (idx < ANIMATION_MODES)
    {
        animation_value8 = display.animations[idx].deceleration & EEPROM_ANIMATION_DECELERATION_MASK;   // lower 4 bits = deceleration

        if (display.animations[idx].flags & ANIMATION_FLAG_FAVOURITE)
        {
            animation_value8 |= EEPROM_ANIMATION_FAVOURITE_MASK;                                        // bit 4 = favourite flag
        }
    }
    else
    {
        animation_value8 = 0xFF;                                                                        // set as unused
    }

    eeprom_write (EEPROM_DATA_OFFSET_ANIMATION_VALUES + idx, &animation_value8, sizeof (uint8_t));
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save animation decelerations in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_animations (void)
{
    uint_fast8_t    idx;

    for (idx = 0; idx < EEPROM_MAX_ANIMATION_MODES; idx++)
    {
        display_save_animation (idx);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save color animation in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_color_animation (uint_fast8_t idx)
{
    uint8_t color_animation_value8;

    if (idx < COLOR_ANIMATION_MODES)
    {                                                                                               // lower 4 bits = deceleration
        color_animation_value8 = display.color_animations[idx].deceleration & EEPROM_COLOR_ANIMATION_DECELERATION_MASK;
    }
    else
    {
        color_animation_value8 = 0xFF;                                                          // set as unused
    }

    eeprom_write (EEPROM_DATA_OFFSET_COLOR_ANIMATION_VALUES + idx, &color_animation_value8, sizeof (uint8_t));
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save color animations in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_color_animations (void)
{
    uint_fast8_t    idx;

    for (idx = 0; idx < EEPROM_MAX_COLOR_ANIMATION_MODES; idx++)
    {
        display_save_color_animation (idx);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight mode deceleration in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_mode_deceleration (uint_fast8_t idx)
{
    uint8_t     ambilight_mode_value8;

    if (idx < AMBILIGHT_MODES)
    {                                                                                       // lower 4 bits = deceleration
        ambilight_mode_value8 = display.ambilight_modes[idx].deceleration & EEPROM_AMBILIGHT_DECELERATION_MASK;

        if (display.ambilight_modes[idx].flags & AMBILIGHT_FLAG_SECONDS_MARKER)
        {                                                                                   // upper 4 bits = some flags
            ambilight_mode_value8 |= AMBILIGHT_FLAG_SECONDS_MARKER << 4;
        }
    }
    else
    {
        ambilight_mode_value8 = 0xFF;                                                      // set as unused
    }

    eeprom_write (EEPROM_DATA_OFFSET_AMBILIGHT_MODE_VALUES + idx, &ambilight_mode_value8, sizeof (uint8_t));
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ambilight mode decelerations in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ambilight_mode_decelerations (void)
{
    uint_fast8_t    idx;

    for (idx = 0; idx < EEPROM_MAX_AMBILIGHT_MODES; idx++)
    {
        display_save_ambilight_mode_deceleration (idx);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save dimmed colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_dimmed_colors (void)
{
    uint8_t         dimmed_colors8[MAX_BRIGHTNESS + 1];
    uint_fast8_t    idx;

    for (idx = 0; idx <= MAX_BRIGHTNESS; idx++)
    {
        dimmed_colors8[idx] = display.dimmed_colors[idx];
    }

    eeprom_write (EEPROM_DATA_OFFSET_DIMMED_COLORS, dimmed_colors8, EEPROM_DATA_SIZE_DIMMED_COLORS);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set dimmed color
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_dimmed_color (uint_fast8_t idx, uint_fast8_t val)
{
    if (val > MAX_BRIGHTNESS)
    {
        val = MAX_BRIGHTNESS;
    }

    display.dimmed_colors[idx] = val;
    display_save_dimmed_colors ();
    return val;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save ticker deceleration in EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_save_ticker_deceleration (void)
{
    uint8_t     ticker_deceleration8 = display.ticker_deceleration;

    eeprom_write (EEPROM_DATA_OFFSET_TICKER_DECELERATION, &ticker_deceleration8, EEPROM_DATA_SIZE_TICKER_DECELERATION);
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write configuration to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_write_config_to_eeprom (void)
{
    uint_fast8_t            rtc = 0;

    if (eeprom_is_up)
    {
        display_save_display_colors ();
        display_save_display_mode ();
        display_save_display_flags ();
        display_save_animation_mode ();
        display_save_color_animation_mode ();
        display_save_brightness ();
        display_save_automatic_brightness ();
        display_save_ambilight_colors ();
        display_save_ambilight_brightness ();
        display_save_ambilight_mode ();
        display_save_number_of_ambilight_leds ();
        display_save_ambilight_led_offset ();
        display_save_overlay_intervals ();
        display_save_animations ();
        display_save_color_animations ();
        display_save_ambilight_mode_decelerations ();
        display_save_dimmed_colors ();
        display_save_ticker_deceleration ();

        rtc = 1;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ticker deceleration
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_ticker_deceleration (uint_fast8_t new_ticker_deceleration)
{
    if (new_ticker_deceleration <= MAX_TICKER_DECELERATION)
    {
        display.ticker_deceleration = new_ticker_deceleration;
    }
    else
    {
        display.ticker_deceleration = MAX_TICKER_DECELERATION;
    }
    display_save_ticker_deceleration ();
    return display.ticker_deceleration;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display flags
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_display_flags (uint_fast8_t new_flags)
{
    if (display.ambilight_mode != AMBILIGHT_MODE_RAINBOW &&
        (new_flags & DISPLAY_FLAGS_SYNC_AMBILIGHT) && (! (display.display_flags & DISPLAY_FLAGS_SYNC_AMBILIGHT)))
    {                                                                           // synchronize ambilight colors
        sync_colors ();
    }

    display_init_ambilight_mode ();
    display.display_flags = new_flags;
    display.animation_start_flag = 1;                                           // fm: needed?
    display_save_display_flags ();
    return display.display_flags;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_ambilight_mode (uint_fast8_t new_mode, uint_fast8_t do_sync)
{
    if (new_mode < AMBILIGHT_MODES && display.ambilight_mode != new_mode)
    {
        display.ambilight_mode = new_mode;
        display_init_ambilight_mode ();

        if (do_sync)
        {
            var_send_ambilight_mode ();
        }

        display_save_ambilight_mode ();
    }
    return display.ambilight_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set number ambilight of leds
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_number_of_ambilight_leds (uint_fast8_t new_leds)
{
    if (new_leds < DSP_AMBILIGHT_LEDS)
    {
        display.ambilight_leds = new_leds;
    }
    else
    {
        display.ambilight_leds = DSP_AMBILIGHT_LEDS;
    }

    display_save_number_of_ambilight_leds ();
    return display.ambilight_leds;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight led offset
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_ambilight_led_offset (uint_fast8_t new_led_offset)
{
    if (new_led_offset < DSP_AMBILIGHT_LEDS - 1)
    {
        display.ambilight_led_offset = new_led_offset;
    }
    else
    {
        display.ambilight_led_offset = 0;
    }

    display_save_ambilight_led_offset ();
    return display.ambilight_led_offset;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_display_mode (uint_fast8_t new_mode, uint_fast8_t do_sync)
{
    if (new_mode < MODES_COUNT)
    {
        display.display_mode = new_mode;
        display.animation_start_flag = 1;

        if (do_sync)
        {
            var_send_display_mode ();
        }

        display_save_display_mode ();
    }
    return display.display_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment display mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_increment_display_mode (uint_fast8_t do_sync)
{
    if (display.display_mode < MODES_COUNT - 1)
    {
        display.display_mode++;
    }
    else
    {
        display.display_mode = 0;
    }

    display.animation_start_flag = 1;

    if (do_sync)
    {
        var_send_display_mode ();
    }

    display_save_display_mode ();
    return display.display_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement display mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_decrement_display_mode (uint_fast8_t do_sync)
{
    if (display.display_mode == 0)
    {
        display.display_mode = MODES_COUNT - 1;
    }
    else
    {
        display.display_mode--;
    }

    display.animation_start_flag = 1;

    if (do_sync)
    {
        var_send_display_mode ();
    }

    display_save_display_mode ();
    return display.display_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set automatic brightness control flag
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_automatic_brightness (uint_fast8_t new_automatic_brightness, uint_fast8_t do_sync)
{
    display.automatic_brightness = new_automatic_brightness;

    if (do_sync)
    {
        var_send_display_automatic_brightness_active ();
    }

    if (! display.automatic_brightness)
    {
        display_set_display_brightness (MAX_BRIGHTNESS, do_sync, TRUE);
        display_set_ambilight_brightness (MAX_BRIGHTNESS, do_sync, TRUE);
    }

    display_save_automatic_brightness ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_animation_mode (uint_fast8_t new_mode, uint_fast8_t do_sync)
{
    if (new_mode < ANIMATION_MODES)
    {
        display.animation_mode = new_mode;

        if (do_sync)
        {
            var_send_animation_mode ();
        }

        display_save_animation_mode ();
    }

    return display.animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_increment_animation_mode (uint_fast8_t do_sync)
{
    if (display.animation_mode < ANIMATION_MODES - 1)
    {
        display.animation_mode++;
    }
    else
    {
        display.animation_mode = 0;
    }

    if (do_sync)
    {
        var_send_animation_mode ();
    }

    display_save_animation_mode ();
    return display.animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_decrement_animation_mode (uint_fast8_t do_sync)
{
    if (display.animation_mode == 0)
    {
        display.animation_mode = ANIMATION_MODES - 1;
    }
    else
    {
        display.animation_mode--;
    }

    if (do_sync)
    {
        var_send_animation_mode ();
    }

    display_save_animation_mode ();
    return display.animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set color animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_set_color_animation_mode (uint_fast8_t new_mode, uint_fast8_t do_sync)
{
    if (new_mode < COLOR_ANIMATION_MODES && display.color_animation_mode != new_mode)
    {
        display.color_animation_mode = new_mode;

        display_init_color_animation ();

        if (do_sync)
        {
            var_send_color_animation_mode ();
        }

        display_save_color_animation_mode ();
    }

    return display.color_animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment color animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_increment_color_animation_mode (uint_fast8_t do_sync)
{
    if (display.color_animation_mode < COLOR_ANIMATION_MODES - 1)
    {
        display.color_animation_mode++;
    }
    else
    {
        display.color_animation_mode = 0;
    }

    display_init_color_animation ();

    if (do_sync)
    {
        var_send_color_animation_mode ();
    }

    display_save_color_animation_mode ();
    return display.color_animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement color animation mode
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_decrement_color_animation_mode (uint_fast8_t do_sync)
{
    if (display.color_animation_mode == 0)
    {
        display.color_animation_mode = COLOR_ANIMATION_MODES - 1;
    }
    else
    {
        display.color_animation_mode--;
    }

    display_init_color_animation ();

    if (do_sync)
    {
        var_send_color_animation_mode ();
    }

    display_save_color_animation_mode ();
    return display.color_animation_mode;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display seconds
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_seconds (uint_fast8_t seconds)
{
    uint_fast8_t    led_idx;
    uint_fast8_t    marker_idx;
    uint_fast8_t    idx;
    uint_fast8_t    n_clock_leds;

    if (display.ambilight_mode == AMBILIGHT_MODE_CLOCK)
    {
        LED_RGB rgb;
        LED_RGB marker_rgb;

        display_show_ambilight_off (0);                                     // switch all ambilight LEDs off, but don't refresh here

        if (display.ambilight_leds >= 60)
        {
            led_idx = seconds;
            led_idx += display.ambilight_led_offset;
            n_clock_leds = 60;
        }
        else if (display.ambilight_leds >= 30)
        {
            led_idx = seconds / 2;
            led_idx += display.ambilight_led_offset;
            n_clock_leds = 30;
        }
        else if (display.ambilight_leds >= 20)
        {
            led_idx = seconds / 3;
            led_idx += display.ambilight_led_offset;
            n_clock_leds = 20;
        }
        else
        {
            n_clock_leds = 0;
        }

        if (n_clock_leds)
        {
            if (led_idx >= n_clock_leds)                                    // wrap around
            {
                led_idx -= n_clock_leds;
            }

            if ((display.ambilight_modes[AMBILIGHT_MODE_CLOCK].flags & AMBILIGHT_FLAG_SECONDS_MARKER) && n_clock_leds == 60)
            {                                                               // show 5-second markers, but only if we have at least 60 LEDs
                DSP_COLORS  dsp_marker_rgb = DSP_CYAN_COLOR;
                DSP_COLORS  dsp_marker_dimmed;

                calc_dimmed_colors (&dsp_marker_dimmed, &dsp_marker_rgb, display.ambilight_brightness, TRUE);
                CALC_LED_RGB(marker_rgb, dsp_marker_dimmed);

                for (idx = 0; idx < 60; idx += 5)
                {
                    marker_idx = idx + display.ambilight_led_offset;

                    if (marker_idx >= n_clock_leds)                         // wrap around
                    {
                        marker_idx -= n_clock_leds;
                    }
                    display_set_ambilight_led (marker_idx, &marker_rgb, 0);
                }
            }

            CALC_LED_RGB(rgb, dimmed_ambilight_colors);
            display_set_ambilight_led (led_idx, &rgb, 0);

            if (display.ambilight_power_is_on)
            {
                display_refresh_ambilight_leds();                           // refresh here
            }
        }
    }
    else if (display.ambilight_mode == AMBILIGHT_MODE_CLOCK2)
    {
        LED_RGB rgb;

        if (seconds == 0)
        {
            if (ambilight_clock_increasing)
            {
                ambilight_clock_increasing = 0;
            }
            else
            {
                ambilight_clock_increasing = 1;
            }
        }

        display_show_ambilight_off (0);                                     // switch all ambilight LEDs off, but don't refresh here

        if (display.ambilight_leds >= 60)
        {
            led_idx = seconds;
            n_clock_leds = 60;
        }
        else if (display.ambilight_leds >= 30)
        {
            led_idx = seconds / 2;
            n_clock_leds = 30;
        }
        else if (display.ambilight_leds >= 20)
        {
            led_idx = seconds / 3;
            n_clock_leds = 20;
        }
        else
        {
            n_clock_leds = 0;
        }

        if (n_clock_leds)
        {
            CALC_LED_RGB(rgb, dimmed_ambilight_colors);

            if (ambilight_clock_increasing)
            {
                for (idx = 0; idx <= led_idx; idx++)
                {
                    uint_fast8_t i = idx + display.ambilight_led_offset;

                    if (i >= n_clock_leds)                                  // wrap around
                    {
                        i -= n_clock_leds;
                    }

                    display_set_ambilight_led (i, &rgb, 0);                 // switch LED on
                }
            }
            else
            {
                for (idx = led_idx; idx < n_clock_leds; idx++)
                {
                    uint_fast8_t i = idx + display.ambilight_led_offset;

                    if (i >= n_clock_leds)                                  // wrap around
                    {
                        i -= n_clock_leds;
                    }

                    display_set_ambilight_led (i, &rgb, 0);                 // switch LED off
                }
            }

            if (display.ambilight_power_is_on)
            {
                display_refresh_ambilight_leds();                           // refresh here
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * show ticker character
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_show_ticker_char (uint_fast8_t start_line, uint_fast8_t start_col, unsigned char ch, uint_fast8_t col_offset, uint_fast8_t use_target)
{
    uint_fast8_t    line;
    uint_fast8_t    col;
    uint_fast8_t    offset;

    if (ch >= 128)
    {
        switch (ch)
        {
            case 0xc4:  ch = TICKER_UP_A_UMLAUT;    break;
            case 0xe4:  ch = TICKER_LO_A_UMLAUT;    break;
            case 0xd6:  ch = TICKER_UP_O_UMLAUT;    break;
            case 0xf6:  ch = TICKER_LO_O_UMLAUT;    break;
            case 0xdc:  ch = TICKER_UP_U_UMLAUT;    break;
            case 0xfc:  ch = TICKER_LO_U_UMLAUT;    break;
            case 0xdf:  ch = TICKER_SHARP_S;        break;
            case 0xb0:  ch = TICKER_DEGREE;         break;
            default:    ch = ' ';                   break;
        }
    }

    for (line = 0; line < TICKER_LINES; line++)
    {
        offset = col_offset + 1;

        for (col = 0; offset < TICKER_COLS + 1; col++, offset++)
        {
            if (start_line + line < WC_ROWS && start_col + col < WC_COLUMNS)
            {
                if (use_target)
                {
                    if (ticker_font[ch][line] & (1<<(TICKER_COLS - offset)))
                    {
                        led.matrix[start_line + line][start_col + col] |= TARGET_STATE;
                    }
                }
                else
                {
                    if (ticker_font[ch][line] & (1<<(TICKER_COLS - offset)))
                    {
                        led.matrix[start_line + line][start_col + col] = NEW_STATE;
                    }
                    else
                    {
                        led.matrix[start_line + line][start_col + col] = CURRENT_STATE;
                    }

                }
            }
        }

        if (! use_target)
        {
            while (start_col + col < WC_COLUMNS)
            {
                led.matrix[start_line + line][start_col + col] = CURRENT_STATE;
                col++;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * show ticker with offset
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_ticker_with_offset (unsigned char * str, uint_fast8_t n)
{
    uint_fast8_t     start_line;
    uint_fast8_t     start_col;

    start_line = (WC_ROWS - TICKER_LINES) / 2;
    start_col  = 0;

    display_show_ticker_char (start_line, start_col, *str, n, FALSE);

    str++;
    start_col = TICKER_COLS + 1 - n;

    while (*str && start_col < WC_COLUMNS)
    {
        display_show_ticker_char (start_line, start_col, *str++, 0, FALSE);
        start_col += TICKER_COLS + 1;                                       // +1 = gap between letters
    }

    while (start_col < WC_COLUMNS)
    {
        display_show_ticker_char (start_line, start_col, ' ', 0, FALSE);
        start_col += TICKER_COLS + 1;                                       // +1 = gap between letters
    }

    display_show_new_display (NEW_STATE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display ticker
 *
 * Will be called every 3/64 sec = 47msec, as long as *ticker_ptr != '\0'
 * See also: display_animation()
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_ticker (void)
{
    display_ticker_with_offset (ticker_ptr, ticker_col);
    ticker_col++;

    if (ticker_col == TICKER_COLS + 1)
    {
        ticker_col = 0;
        ticker_ptr++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ticker text
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_ticker (const unsigned char * ticker, uint_fast8_t do_wait)
{
    if (display.power_is_on)
    {
        ticker_str[0] = ' ';
        ticker_str[1] = ' ';
        strncpy ((char *) ticker_str + 2, (const char *) ticker, MAX_TICKER_LEN - 2);
        ticker_col = 0;
        ticker_ptr = ticker_str;
        display_reset_led_states ();

        if (do_wait)
        {
            while (*ticker_ptr)
            {
                display_ticker ();
                delay_msec ((display.ticker_deceleration * 1000) / 64);
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * check if ticker is active
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
display_ticker_active (void)
{
    uint_fast8_t    rtc = 0;

    if (* ticker_ptr)
    {
        rtc = 1;
    }

    return rtc;
}

#define ICON_COLOR_BLACK     0              // 0000
#define ICON_COLOR_RED       1              // 0001
#define ICON_COLOR_GREEN     2              // 0010
#define ICON_COLOR_YELLOW    3              // 0011
#define ICON_COLOR_BLUE      4              // 0100
#define ICON_COLOR_MAGENTA   5              // 0101
#define ICON_COLOR_CYAN      6              // 0110
#define ICON_COLOR_WHITE     7              // 0111
#define ICON_COLOR_BROWN     8              // dark yellow
#define ICON_COLOR_GREY      9              // grey
#define ICON_COLORS         10

static const DSP_COLORS icon_colors[ICON_COLORS] =
{
    DSP_BLACK_COLOR,
    DSP_RED_COLOR,
    DSP_GREEN_COLOR,
    DSP_YELLOW_COLOR,
    DSP_BLUE_COLOR,
    DSP_MAGENTA_COLOR,
    DSP_CYAN_COLOR,
    DSP_WHITE_COLOR,
    DSP_BROWN_COLOR,
    DSP_GREY_COLOR
};

typedef struct
{
    const uint_fast8_t  rows;
    const uint_fast8_t  cols;
    const char *        colors;
    const char *        animation;
} ICON;

#if WCLOCK24H == 1                                                          // WC24h:

static const ICON       icons[MAX_ICONS] =
{
    {                                                                       // 0 heart
        10, 12,
        "001110011100"                                                      // colors
        "011111111110"
        "111111111111"
        "111111111111"
        "111111111111"
        "011111111110"
        "001111111100"
        "000111111000"
        "000011110000"
        "000001100000",

        "00AAA00AAA00"                                                      // animation
        "0ALLLLLLLLA0"
        "AKKKKKKKKKKA"
        "AJJJJJJJJJJA"
        "AIIIIIIIIIIA"
        "0AHHHHHHHHA0"
        "00AGGGGGGA00"
        "000ABFFFA000"
        "0000ADDA0000"
        "00000AA00000"
    },
    {                                                                       // 1 Xmas tree
        16, 16,
        "0000000220000000"                                                  // colors
        "0000002222000000"
        "0000021221200000"
        "0000322222230000"
        "0000022222200000"
        "0000212222120000"
        "0003222222223000"
        "0000222222220000"
        "0002122222212000"
        "0032222222222300"
        "0002222222222000"
        "0021222222221200"
        "0322222222222230"
        "0000008888000000"
        "0077008888000777"
        "7777778888777777",

        "0000000AA0000000"                                                  // animation
        "000000AAAA000000"
        "00000AJAAJA00000"
        "0000TAAAAAAT0000"
        "00000AAAAAA00000"
        "0000AJAAAAJA0000"
        "000TAAAAAAAAT000"
        "0000AAAAAAAA0000"
        "000AJAAAAAAJA000"
        "00TAAAAAAAAAAT00"
        "000AAAAAAAAAA000"
        "00AJAAAAAAAAJA00"
        "0TAAAAAAAAAAAAT0"
        "000000AAAA000000"
        "00TT00AAAA000TTT"
        "TTTTTTAAAATTTTTT"
    },
    {                                                                       // 2 fireworks
        16, 16,
        "0001110000111000"                                                  // colors
        "2200011001100022"
        "0222001001002220"
        "0002220110222000"
        "0000022002200000"
        "0004440770444000"
        "0440004774000440"
        "4000333003330004"
        "0033300000033300"
        "0330011771100330"
        "3300110770110033"
        "0001100770011000"
        "0001000770001000"
        "0011000770001100"
        "0010000770000100"
        "0000000770000000",

        "000PPP0000PPP000"                                                  // animation
        "PP000ON00NO000PP"
        "0OOO00N00N00OOO0"
        "000NNN0MM0NNN000"
        "00000MM00MM00000"
        "000NNN0KK0NNN000"
        "0OO000MJJM000OO0"
        "P000MMM00MMM000P"
        "00NNN000000NNN00"
        "0OO00MMIIMM00OO0"
        "PP00NN0HH0NN00PP"
        "000OO00GG00OO000"
        "000P000FF000P000"
        "00RR000EE000RR00"
        "00S0000CC0000S00"
        "0000000AA0000000"
    },
#if 0
    {                                                                       // blue star / ice crystal, comes later
        16, 16,
        "0000000440000000"
        "0400000440000040"
        "0044004444004400"
        "0044444004444400"
        "0004004004004000"
        "0004000440004000"
        "0044404444044400"
        "4440044004400444"
        "4440044004400444"
        "0044404444044400"
        "0004000440004000"
        "0004004004004000"
        "0044444004444400"
        "0044004444004400"
        "0400000440000040"
        "0000000440000000"

        "",                                                                 // no animation
    },
#endif
};

#else                                                                       // WC12h:

static const ICON       icons[MAX_ICONS] =
{
    {                                                                       // 0 heart
        10, 11,
        "00000000000"                                                       // colors
        "00111011100"
        "01111111110"
        "11111111111"
        "11111111111"
        "01111111110"
        "00111111100"
        "00011111000"
        "00001110000"
        "00000100000",

        "00000000000"                                                       // animation
        "00EEE0EEE00"
        "0FEDCCCDE10"
        "GFDCBBBCDFG"
        "GFDCBABCDFG"
        "0FDCBBBCDF0"
        "00EDCCCDE00"
        "000EDDD1000"
        "0000EEE0000"
        "00000F00000"
    },
    {                                                                       // 1 Xmas tree
        10, 11,
        "00000200000"                                                       // colors
        "00032223000"
        "00021212000"
        "03222222230"
        "02212221220"
        "00222222200"
        "32122222123"
        "22222222222"
        "00008880000"
        "00008880000",

        "00000A00000"                                                       // animation
        "000TAAAT000"
        "000AJAJA000"
        "0TAAAAAAAT0"
        "0AAJAAAJAA0"
        "00AAAAAAA00"
        "TAJAAAAAJAT"
        "AAAAAAAAAAA"
        "0000AAA0000"
        "0000AAA0000"
    },
    {                                                                       // 2 fireworks
        10, 11,
        "00006001000"                                                       // colors
        "01103010101"
        "10010100100"
        "05001010000"
        "00130700510"
        "00037300600"
        "00007000000"
        "00070000000"
        "00070000000"
        "00070000000",

        "0000P00K000"                                                       // animation
        "0LK0P0I0J0K"
        "M00H0H00H00"
        "0P00G0G0000"
        "00IH0F00PJ0"
        "000IEI00H00"
        "0000D000000"
        "000C0000000"
        "000B0000000"
        "000A0000000",
    },

#if 0
    {                                                                       // 2 Sonne
        10, 11,
        "03000300030"                                                       // colors
        "00300000300"
        "00003330000"
        "00033333000"
        "33033333033"
        "00033333000"
        "00003330000"
        "00300000300"
        "03000300030"
        "00000300000",

        ""                                                                  // no animation
    },
    {                                                                       // 3 Nebel
        10, 11,
        "03000300030"                                                       // colors
        "00300009990"
        "00003339990"
        "09993333000"
        "33033333033"
        "00999993000"
        "00009990000"
        "09999999990"
        "09999999990"
        "00000000000",

        ""                                                                  // no animation

    },
    {                                                                       // 4 Leicht bewoelkt
        10, 11,
        "00000030000"                                                       // colors
        "00300030003"
        "00030000030"
        "04440333000"
        "44444344400"
        "44444444443"
        "44444444444"
        "44444444444"
        "04444444440"
        "00000000000",

        ""                                                                  // no animation

    },
    {                                                                       // 5 Leichter Regen
        10, 11,
        "00030000030"                                                       // colors
        "04440333000"
        "44444344400"
        "44444444443"
        "44444444444"
        "44444444444"
        "04444444440"
        "00400400400"
        "00004000400"
        "00404004000",

        ""                                                                  // no animation

    },
    {                                                                       // 6 Regenschauer
        10, 11,
        "00000000000"                                                       // colors
        "04440000000"
        "44444044400"
        "44444444440"
        "44444444444"
        "44444444444"
        "04444444440"
        "00400400400"
        "00004000400"
        "00404004000",

        ""                                                                  // no animation

    },
    {                                                                       // 7 Schnee
        10, 11,
        "07770000000"                                                       // colors
        "77777077700"
        "77777777770"
        "77777777777"
        "77777777777"
        "07777777770"
        "07000000700"
        "00007000000"
        "00000000000"
        "07000707000",

        ""                                                                  // no animation

    },
    {                                                                       // 8 Gewitter
        10, 11,
        "04440000000"                                                       // colors
        "44444044400"
        "44444444440"
        "44443334444"
        "44433344444"
        "04433444440"
        "00003300000"
        "00033000000"
        "00330000000"
        "00300000000",

        ""                                                                  // no animation

    },
#endif
};

#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display an icon
 * called 64 times per second
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
display_icon (void)
{
    static uint_fast8_t     icon_idx = 0xFF;
    static uint_fast8_t     animation_len;
    static uint_fast16_t    cnt;
    static uint_fast8_t     row_offset;
    static uint_fast8_t     col_offset;
    uint_fast16_t           n;
    uint_fast8_t            row;
    uint_fast8_t            col;
    uint_fast8_t            color_idx;
    DSP_COLORS              dsp_rgb;
    DSP_COLORS              dsp_rgb_dimmed;
    LED_RGB                 led_rgb;

    if (display.power_is_on)
    {
        LED_RGB             led_rgb0;

        RESET_LED_RGB(led_rgb0);

        if (icon_idx == 0xFF)
        {
            icon_idx = display.display_icon;
            if (icons[icon_idx].animation)
            {
                animation_len = strlen (icons[icon_idx].animation);
            }
            else
            {
                animation_len = 0;
            }

            cnt = 0;

            row_offset = (WC_ROWS - icons[icon_idx].rows) / 2;
            col_offset = (WC_COLUMNS - icons[icon_idx].cols) / 2;

            for (n = 0; n < DSP_DISPLAY_LEDS; n++)
            {
                led.state[n] = 0;
                display_set_display_led (n, &led_rgb0, 0);
            }

            display_refresh_display_leds ();
        }
        else
        {
            uint_fast16_t brightness;

            cnt++;

            if (cnt & 0x01)                                     // work only with odd values of cnt
            {
                uint_fast8_t do_display = 0;

                if (cnt < 128)                                  // first half of animation?
                {                                               // yes, step up
                    brightness = cnt >> 1;
                }
                else                                            // second half of animation?
                {                                               // yes, step down
                    brightness = (255 - cnt) >> 1;              // last odd value of cnt is 255
                }

                if (brightness <= display.dimmed_colors[display.display_brightness])
                {
                    do_display = 1;
                }
                else if (animation_len > 0)
                {
                    do_display = 1;
                    brightness = display.dimmed_colors[display.display_brightness];
                }

                if (do_display)
                {
                    uint_fast8_t animation_step;

                    n = 0;

                    for (row = 0; row < icons[icon_idx].rows; row++)
                    {
                        for (col = 0; col < icons[icon_idx].cols; col++)
                        {
                            if (n < animation_len)
                            {
                                animation_step = icons[icon_idx].animation[n];

                                if (animation_step >= 'A')
                                {
                                    animation_step -= 'A';
                                }
                                else
                                {
                                    animation_step = 0;
                                }
                            }
                            else
                            {
                                animation_step = 0;
                            }

                            if ((cnt >> 2) >= animation_step)
                            {
                                if (brightness > 0)
                                {
                                    color_idx = icons[icon_idx].colors[n] - '0';
                                    SET_DSP_RGB(dsp_rgb, icon_colors[color_idx].red, icon_colors[color_idx].green, icon_colors[color_idx].blue, 0);
                                    calc_dimmed_colors (&dsp_rgb_dimmed, &dsp_rgb, brightness, TRUE);
                                    CALC_LED_RGB(led_rgb, dsp_rgb_dimmed);
                                    display_set_display_led ((row + row_offset) * WC_COLUMNS + (col + col_offset), &led_rgb, 0);
                                }
                                else
                                {
                                    display_set_display_led ((row + row_offset) * WC_COLUMNS + (col + col_offset), &led_rgb0, 0);
                                }
                            }

                            n++;
                        }
                    }
                    display_refresh_display_leds ();
                }
            }

            if (cnt == 256)
            {
                icon_idx = 0xFF;
                display.display_icon = 0xFF;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display animation - called every 1/64 sec
 *
 * Rainbow color animation:
 *
 *     Step     von         nach        Änd.    Phase   S1      S2
 *      1           rot     gelb        g+          1       1   6
 *      2           gelb        grün    r-          2       2   7
 *      3           grün        cyan    b+          3       3   8
 *      4           cyan        blau    g-          4       4   9
 *      5           blau        magenta r+          5       5   10
 *      6           magenta     rot         b-      6       6   11
 *      7           rot     gelb        g+          1       1   12
 *      8           gelb        weiß    b+          3*      2   13*
 *      9           weiß        cyan    r-          2*      3   14*
 *      10          cyan        blau    g-          4       4   1
 *      11          blau        magenta r+          5       5   2
 *      12          magenta     rot         b-      6       6   3
 *      13          rot     gelb        g+          1       1   4
 *      14          gelb        grün    r-          2       2   5
 *      15          grün        cyan    b+          3       3   6
 *      16          cyan        blau    g-          4       4   7
 *      17          blau        magenta r+          5       5   8
 *      18          magenta     rot         b-      6       6   9
 *      19          rot     gelb        g+          1       1   10
 *      20          gelb        grün    r-          2       2   11
 *      21          grün        cyan    b+          3       3   12
 *      22          cyan        weiß    r+          5*      4   13*
 *      23          weiß        magenta g-          4*      5   14*
 *      24          magenta     rot         b-      6       6   1
 *      25          rot     gelb        g+          1       1   2
 *      26          gelb        grün    r-          2       2   3
 *      27          grün        cyan    b+          3       3   4
 *      28          cyan        blau    g-          4       4   5
 *      29          blau        magenta r+          5       5   6
 *      30          magenta     rot         b-      6       6   7
 *      31          rot     gelb        g+          1       1   8
 *      32          gelb        grün    r-          2       2   9
 *      33          grün        cyan    b+          3       3   10
 *      34          cyan        blau    g-          4       4   11
 *      35          blau        magenta r+          5       5   12
 *      36          magenta     weiß    g+          1*      6   13*
 *      37          weiß        gelb    b-          6*      1   14*
 *      38          gelb        grün    r-          2       2   1
 *      39          grün        cyan    b+          3       3   2
 *      40          cyan        blau    g-          4       4   3
 *      41          blau        magenta r+          5       5   4
 *      42          magenta     rot         b-      6       6   5
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_animation (void)
{
    if (*ticker_ptr)
    {
        static uint_fast8_t cnt;

        cnt++;

        if (cnt >= display.ticker_deceleration)                 // 1/64 sec x ticker_deceleration (default is 3 -> 21msec = 50Hz)
        {
            cnt = 0;
            display_ticker ();
        }
    }
    else if (display.display_icon != 0xFF)
    {
        display_icon ();
    }
    else
    {
        if (display.animation_mode < ANIMATION_MODES && (display.power_is_on || display.animation_start_flag || ! display.animation_stop_flag))
        {                                                       // if power is off, display only animation if started or yet not stopped
            if (display.animation_start_flag)
            {
                display.animations[display.animation_mode].func ();
            }
            else if (! display.animation_stop_flag)
            {
                if (display.animation_mode == ANIMATION_MODE_RANDOM)    // random makes its own animation dependant deceleration
                {
                    display.animations[display.animation_mode].func ();
                }
                else
                {
                    static uint_fast16_t    deceleration_cnt;

                    deceleration_cnt++;

                    if (deceleration_cnt >= display.animations[display.animation_mode].deceleration)
                    {
                        deceleration_cnt = 0;
                        display.animations[display.animation_mode].func ();
                    }
                }
            }
        }

        if (display.animation_stop_flag)                                            // animation stopped
        {
            if (display.color_animation_mode == COLOR_ANIMATION_MODE_RAINBOW)
            {
                static uint_fast8_t     display_rainbow_state1 = 0;
                static uint_fast8_t     display_rainbow_state2 = 5;
                static uint_fast8_t     display_rainbow_phase;
                static uint_fast16_t    display_rainbow_cnt;
                uint_fast8_t            display_rainbow_changed = 0;

                display_rainbow_cnt++;

                if (display_rainbow_cnt >= 32 * display.color_animations[COLOR_ANIMATION_MODE_RAINBOW].deceleration)
                {
                    display_rainbow_cnt = 0;

                    switch (display_rainbow_phase)
                    {
                        case 1: //g+
                            if (display.display_colors.green < MAX_COLOR_STEPS - 1)
                            {
                                display.display_colors.green++;
                                display_rainbow_changed = 1;
                            }
                            break;

                        case 2: //r-
                            if (display.display_colors.red > 0)
                            {
                                display.display_colors.red--;
                                display_rainbow_changed = 1;
                            }
                            break;

                        case 3: //b+
                            if (display.display_colors.blue < MAX_COLOR_STEPS - 1)
                            {
                                display.display_colors.blue++;
                                display_rainbow_changed = 1;
                            }
                            break;

                        case 4: //g-
                            if (display.display_colors.green > 0)
                            {
                                display.display_colors.green--;
                                display_rainbow_changed = 1;
                            }
                            break;

                        case 5: //r+
                            if (display.display_colors.red < MAX_COLOR_STEPS - 1)
                            {
                                display.display_colors.red++;
                                display_rainbow_changed = 1;
                            }
                            break;

                        case 6: //b-
                            if (display.display_colors.blue > 0)
                            {
                                display.display_colors.blue--;
                                display_rainbow_changed = 1;
                            }
                            break;
                    }

                    if (display_rainbow_changed)
                    {
                        display_calc_dimmed_display_colors ();
                    }
                    else
                    {
                        display_rainbow_state1++;

                        if (display_rainbow_state1 == 7)
                        {
                            display_rainbow_state1 = 1;
                        }

                        display_rainbow_phase = display_rainbow_state1;

                        // bei S2= 13 / 14 weiß durch Tausch der Steps
                        // geht auch bei 7 / 8, dann kommt weiß öfter

                        display_rainbow_state2++;

                        if (display_rainbow_state2 == 13) // oder 7
                        {
                            display_rainbow_phase++;

                            if (display_rainbow_phase == 7)
                            {
                                display_rainbow_phase = 1;
                            }
                        }

                        if (display_rainbow_state2 == 14) // oder 8
                        {
                            display_rainbow_phase--;

                            if (display_rainbow_phase == 0)
                            {
                                display_rainbow_phase = 6;
                            }
                            display_rainbow_state2 = 0;
                        }
                    }
                }
            }

            if (display.ambilight_mode == AMBILIGHT_MODE_RAINBOW)
            {
                static uint_fast8_t     ambilight_rainbow_state1 = 0;
                static uint_fast8_t     ambilight_rainbow_state2 = 5;
                static uint_fast8_t     ambilight_rainbow_phase;
                static uint_fast16_t    ambilight_rainbow_cnt;
                uint_fast8_t            ambilight_rainbow_changed = 0;

                ambilight_rainbow_cnt++;

                if (ambilight_rainbow_cnt >= 32 * display.ambilight_modes[AMBILIGHT_MODE_RAINBOW].deceleration)
                {
                    ambilight_rainbow_cnt = 0;

                    switch (ambilight_rainbow_phase)
                    {
                        case 1: //g+
                            if (display.ambilight_colors.green < MAX_COLOR_STEPS - 1)
                            {
                                display.ambilight_colors.green++;
                                ambilight_rainbow_changed = 1;
                            }
                            break;

                        case 2: //r-
                            if (display.ambilight_colors.red > 0)
                            {
                                display.ambilight_colors.red--;
                                ambilight_rainbow_changed = 1;
                            }
                            break;

                        case 3: //b+
                            if (display.ambilight_colors.blue < MAX_COLOR_STEPS - 1)
                            {
                                display.ambilight_colors.blue++;
                                ambilight_rainbow_changed = 1;
                            }
                            break;

                        case 4: //g-
                            if (display.ambilight_colors.green > 0)
                            {
                                display.ambilight_colors.green--;
                                ambilight_rainbow_changed = 1;
                            }
                            break;

                        case 5: //r+
                            if (display.ambilight_colors.red < MAX_COLOR_STEPS - 1)
                            {
                                display.ambilight_colors.red++;
                                ambilight_rainbow_changed = 1;
                            }
                            break;

                        case 6: //b-
                            if (display.ambilight_colors.blue > 0)
                            {
                                display.ambilight_colors.blue--;
                                ambilight_rainbow_changed = 1;
                            }
                            break;
                    }

                    if (ambilight_rainbow_changed)
                    {
                        display_calc_dimmed_ambilight_colors ();
                    }
                    else
                    {
                        ambilight_rainbow_state1++;

                        if (ambilight_rainbow_state1 == 7)
                        {
                            ambilight_rainbow_state1 = 1;
                        }

                        ambilight_rainbow_phase = ambilight_rainbow_state1;

                        // bei S2= 13 / 14 weiß durch Tausch der Steps
                        // geht auch bei 7 / 8, dann kommt weiß öfter

                        ambilight_rainbow_state2++;

                        if (ambilight_rainbow_state2 == 13) // oder 7
                        {
                            ambilight_rainbow_phase++;

                            if (ambilight_rainbow_phase == 7)
                            {
                                ambilight_rainbow_phase = 1;
                            }
                        }

                        if (ambilight_rainbow_state2 == 14) // oder 8
                        {
                            ambilight_rainbow_phase--;

                            if (ambilight_rainbow_phase == 0)
                            {
                                ambilight_rainbow_phase = 6;
                            }
                            ambilight_rainbow_state2 = 0;
                        }
                    }
                }
            }

            if (! display.power_is_on)
            {
                power_off ();
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment red color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_display_color_red (uint_fast8_t do_sync)
{
    if (display.display_colors.red < MAX_COLOR_STEPS - 1)
    {
        display.display_colors.red++;

        if (display.display_colors.red < MAX_COLOR_STEPS - 1)
        {
            display.display_colors.red++;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement red color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_display_color_red (uint_fast8_t do_sync)
{
    if (display.display_colors.red > 0)
    {
        display.display_colors.red--;

        if (display.display_colors.red > 0)
        {
            display.display_colors.red--;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment green color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_display_color_green (uint_fast8_t do_sync)
{
    if (display.display_colors.green < MAX_COLOR_STEPS - 1)
    {
        display.display_colors.green++;

        if (display.display_colors.green < MAX_COLOR_STEPS - 1)
        {
            display.display_colors.green++;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement green color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_display_color_green (uint_fast8_t do_sync)
{
    if (display.display_colors.green > 0)
    {
        display.display_colors.green--;

        if (display.display_colors.green > 0)
        {
            display.display_colors.green--;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment blue color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_display_color_blue (uint_fast8_t do_sync)
{
    if (display.display_colors.blue < MAX_COLOR_STEPS - 1)
    {
        display.display_colors.blue++;

        if (display.display_colors.blue < MAX_COLOR_STEPS - 1)
        {
            display.display_colors.blue++;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement blue color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_display_color_blue (uint_fast8_t do_sync)
{
    if (display.display_colors.blue > 0)
    {
        display.display_colors.blue--;

        if (display.display_colors.blue > 0)
        {
            display.display_colors.blue--;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

#if DSP_USE_SK6812_RGBW == 1

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment white color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_display_color_white (uint_fast8_t do_sync)
{
    if (display.display_colors.white < MAX_COLOR_STEPS - 1)
    {
        display.display_colors.white++;

        if (display.display_colors.white < MAX_COLOR_STEPS - 1)
        {
            display.display_colors.white++;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement white color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_display_color_white (uint_fast8_t do_sync)
{
    if (display.display_colors.white > 0)
    {
        display.display_colors.white--;

        if (display.display_colors.white > 0)
        {
            display.display_colors.white--;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_colors ();
        }
    }
}

#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_display_colors (DSP_COLORS * rgb)
{
    COPY_DSP_RGB_SAFE (display.display_colors, rgb);
    display_calc_dimmed_display_colors ();

    display_save_display_colors ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display brightness
 *  MAX_BRIGHTNESS  = full brightness
 *   0              = lowest brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_display_brightness (uint_fast8_t new_brightness, uint_fast8_t do_sync, uint_fast8_t do_save)
{
    if (new_brightness != display.display_brightness)
    {
        if (new_brightness > MAX_BRIGHTNESS)
        {
            display.display_brightness = MAX_BRIGHTNESS;
        }
        else
        {
            display.display_brightness = new_brightness;
        }

        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_brightness ();
        }

        if (do_save)
        {
            display_save_brightness ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement display brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_display_brightness (uint_fast8_t do_sync)
{
    if (display.display_brightness > 0)
    {
        display.display_brightness--;
        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_brightness ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment display brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_display_brightness (uint_fast8_t do_sync)
{
    if (display.display_brightness < MAX_BRIGHTNESS)
    {
        display.display_brightness++;
        display_calc_dimmed_display_colors ();

        if (do_sync)
        {
            var_send_display_brightness ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight_brightness
 *  MAX_BRIGHTNESS  = full brightness
 *   0              = lowest brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_ambilight_brightness (uint_fast8_t new_brightness, uint_fast8_t do_sync, uint_fast8_t do_save)
{
    if (new_brightness != display.ambilight_brightness)
    {
        if (new_brightness > MAX_BRIGHTNESS)
        {
            display.ambilight_brightness = MAX_BRIGHTNESS;
        }
        else
        {
            display.ambilight_brightness = new_brightness;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_brightness ();
        }

        if (do_save)
        {
            display_save_ambilight_brightness  ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement ambilight_brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_ambilight_brightness (uint_fast8_t do_sync)
{
    if (display.ambilight_brightness > 0)
    {
        display.ambilight_brightness--;
        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_brightness ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment ambilight_brightness
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_ambilight_brightness (uint_fast8_t do_sync)
{
    if (display.ambilight_brightness < MAX_BRIGHTNESS)
    {
        display.ambilight_brightness++;
        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_brightness ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment red ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_ambilight_color_red (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.red < MAX_COLOR_STEPS - 1)
    {
        display.ambilight_colors.red++;

        if (display.ambilight_colors.red < MAX_COLOR_STEPS - 1)
        {
            display.ambilight_colors.red++;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement red ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_ambilight_color_red (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.red > 0)
    {
        display.ambilight_colors.red--;

        if (display.ambilight_colors.red > 0)
        {
            display.ambilight_colors.red--;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment green ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_ambilight_color_green (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.green < MAX_COLOR_STEPS - 1)
    {
        display.ambilight_colors.green++;

        if (display.ambilight_colors.green < MAX_COLOR_STEPS - 1)
        {
            display.ambilight_colors.green++;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement green ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_ambilight_color_green (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.green > 0)
    {
        display.ambilight_colors.green--;

        if (display.ambilight_colors.green > 0)
        {
            display.ambilight_colors.green--;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment blue ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_ambilight_color_blue (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.blue < MAX_COLOR_STEPS - 1)
    {
        display.ambilight_colors.blue++;

        if (display.ambilight_colors.blue < MAX_COLOR_STEPS - 1)
        {
            display.ambilight_colors.blue++;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement blue ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_ambilight_color_blue (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.blue > 0)
    {
        display.ambilight_colors.blue--;

        if (display.ambilight_colors.blue > 0)
        {
            display.ambilight_colors.blue--;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

#if DSP_USE_SK6812_RGBW == 1

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * increment white ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_increment_ambilight_color_white (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.white < MAX_COLOR_STEPS - 1)
    {
        display.ambilight_colors.white++;

        if (display.ambilight_colors.white < MAX_COLOR_STEPS - 1)
        {
            display.ambilight_colors.white++;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * decrement white ambilight color by 2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_decrement_ambilight_color_white (uint_fast8_t do_sync)
{
    if (display.ambilight_colors.white > 0)
    {
        display.ambilight_colors.white--;

        if (display.ambilight_colors.white > 0)
        {
            display.ambilight_colors.white--;
        }

        display_calc_dimmed_ambilight_colors ();

        if (do_sync)
        {
            var_send_ambilight_colors ();
        }
    }
}

#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight colors
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_ambilight_colors (DSP_COLORS * rgb)
{
    COPY_DSP_RGB_SAFE (display.ambilight_colors, rgb);
    display_calc_dimmed_ambilight_colors ();
    display_save_ambilight_colors ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display animation deceleration
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_animation_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)
{
    display.animations[idx].deceleration = deceleration;
    display_save_animation (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set display animation flags
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_animation_flags (uint_fast8_t idx, uint_fast8_t flags)
{
    display.animations[idx].flags = flags;
    display_save_animation (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set color animation deceleration
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_color_animation_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)
{
    display.color_animations[idx].deceleration = deceleration;
    display_save_color_animation (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight mode deceleration
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_ambilight_mode_deceleration (uint_fast8_t idx, uint_fast8_t deceleration)
{
    display.ambilight_modes[idx].deceleration = deceleration;
    display_save_ambilight_mode_deceleration (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set ambilight mode flags
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_ambilight_mode_flags (uint_fast8_t idx, uint_fast8_t flags)
{
    display.ambilight_modes[idx].flags = flags;
    display_save_ambilight_mode_deceleration (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set an overlay interval
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_set_overlay_interval (uint_fast8_t idx, uint_fast8_t val)
{
    display.overlays.intervals[idx] = val;
    display_save_overlay_intervals ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * Test all LEDs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_test (void)
{
    LED_RGB      rgb;
    uint_fast16_t   i;
    uint_fast16_t   j;

#if DSP_USE_SK6812_RGBW == 1

    for (i = 1; i < 16; i++)
    {
        rgb.red     = (i & 0x01) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;
        rgb.green   = (i & 0x02) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;
        rgb.blue    = (i & 0x04) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;
        rgb.white   = (i & 0x08) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;

        for (j = 0; j < DSP_MAX_LEDS; j++)
        {
            led_set_led (j, &rgb);
        }

        display_refresh_ambilight_leds ();
        delay_sec (3);
    }

#else

    for (i = 1; i < 8; i++)
    {
        rgb.red     = (i & 0x01) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;
        rgb.green   = (i & 0x02) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;
        rgb.blue    = (i & 0x04) ? pwmtable8[MAX_COLOR_STEPS / 2] : 0;

        for (j = 0; j < DSP_MAX_LEDS; j++)
        {
            led_set_led (j, &rgb);
        }

        display_refresh_ambilight_leds ();
        delay_sec (3);
    }

#endif

    display_animation_flush (FALSE);
    display_set_status_led (0, 0, 0);
    display_calc_dimmed_ambilight_colors ();                                // calc & refresh ambilight
}

#if WCLOCK24H == 1
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display temperature
 *
 *   index ==   0  ->   0°C
 *   index == 250  -> 125°C
 *
 * the first temperature we can show is 10,0°C (index =  0, temperature_index ==  20)
 * the last  temperature we can show is 39,5°C (index = 79, temperature_index == 159)
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_temperature (uint_fast8_t temperature_index)
{
    if (display.power_is_on)
    {
        uint_fast8_t   temp_mode = MODES_COUNT - 1;

        if (temperature_index >= 20 && temperature_index < 80)
        {
            uint8_t                         minute_mode;
            const struct MinuteDisplay *    tbl_minute;
            uint_fast16_t                   idx;

            temperature_index -= 20;                                            // subtract 10°C (20 units)
            display_animation_flush (FALSE);
            minute_mode = tbl_modes[temp_mode].minute_txt;
            tbl_minute  = &tbl_minutes[minute_mode][temperature_index];

            display_reset_led_states ();

            display_word_on (WP_ES);
            display_word_on (WP_IST);

            for (idx = 0; idx < MAX_MINUTE_WORDS && tbl_minute->wordIdx[idx] != 0; idx++)
            {
                display_word_on (tbl_minute->wordIdx[idx]);
            }

            display.animation_start_flag = 1;
        }
    }
}

#else

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display temperature on WC12h with 2 big digits
 *
 *   index ==   0  ->   0°C
 *   index == 250  -> 125°C
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_temperature (uint_fast8_t temperature_index)
{
    if (display.power_is_on)
    {
        char buf[4];
        uint_fast8_t     temp;
        uint_fast8_t     temp_fraction;
        uint_fast8_t     start_line;
        uint_fast8_t     start_col;

        temp = temperature_index / 2;
        temp_fraction = temperature_index % 2;

        sprintf (buf, "%u", temp);

        display_animation_flush (FALSE);
        display_reset_led_states ();

        start_line = (WC_ROWS - TICKER_LINES) / 2;
        start_col  = 0;

        display_show_ticker_char (start_line, start_col, buf[0], 0, TRUE);
        start_col += TICKER_COLS + 1;                                       // +1 = gap between letters
        display_show_ticker_char (start_line, start_col, buf[1], 0, TRUE);
        display_minute_leds (display.power_is_on, 2 * temp_fraction);
        display.animation_start_flag = 1;
    }
}

#endif // WCLOCK24H == 1

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize LED display
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
display_init (void)
{
    led_init ();
}
