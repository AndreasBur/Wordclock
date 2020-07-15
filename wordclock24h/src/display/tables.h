/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables.h - WC24H tables
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "wclock24h-config.h"

#if WCLOCK24H == 1

#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

#define MODES_COUNT         18                                  // count of different display modes
#define IT_IS_MODES_COUNT    2                                  // count of different it_is modes
#define MAX_HOUR_WORDS       4                                  // how many words for hour display (no end token)
#define HOUR_COUNT          25                                  // 24 plus one to distinguish between before and after full hour
#define MAX_MINUTE_WORDS     7                                  // how many words for minute display (no end token)
#define MINUTE_COUNT        60
#define WC_COLUMNS          18
#define WC_ROWS             16

struct WordIllu
{
    uint8_t row;
    uint8_t col;
    uint8_t len;
};

struct Modes
{
    uint8_t minute_txt;
    uint8_t hour_txt;
    const char * description;
};

// possible values of flags in struct MinuteDisplay:
#define MDF_IT_IS_GERMAN        0x00
#define MDF_IT_IS_ENGLISH       0x01
#define MDF_IT_IS_MASK          0x01                    // 1 bit reserved for it-is language, correct IT_IS_MODES_COUNT if you extend languages!

#define MDF_HOUR_OFFSET_0       0x00
#define MDF_HOUR_OFFSET_1       0x02
#define MDF_HOUR_OFFSET_MASK    0x02                    // 1 bit reserved for hour offset mask

struct MinuteDisplay
{
    uint8_t flags;
    uint8_t wordIdx[MAX_MINUTE_WORDS];
};

enum HourMode
{
    HM_0,                                                       // 0 = Mode 0: "leer, fuer Temperaturanzeige"
    HM_1,                                                       // 1 = Mode 1: "MM - HH (12)"
    HM_2,                                                       // 2 = Mode 2: "MM - HH UHR (12)"
    HM_3,                                                       // 3 = Mode 3: "MM - HH UHR (12) NACHTS"
    HM_4,                                                       // 4 = Mode 4: "MM - HH UHR (24)"
    HM_5,                                                       // 5 = Mode 5: "HH UHR (12) - MM"
    HM_6,                                                       // 6 = Mode 6: "HH UHR 24) - MM"
    HM_7,                                                       // 7 = Mode 7: "MITTERNACHT (0 UHR)"
    HOUR_MODES_COUNT                                            // Number of HourModes
};

enum MinuteMode
{
    MM_1,                                                       // 0 = Mode 1: "MM NACH"
    MM_2,                                                       // 1 = Mode 2: "MM MINUTEN NACH"
    MM_3,                                                       // 2 = Mode 3: "OSSI - MM MINUTEN NACH (VIERTEL NACH, HALB, VIERTEL VOR)"
    MM_4,                                                       // 3 = Mode 4: "OESI - MINUTEN NACH (VIERTEL NACH, HALB, DREIVIERTEL)"
    MM_5,                                                       // 4 = Mode 5: "RHEIN/ RUHR - MINUTEN NACH (VIERTEL, HALB, DREIVIERTEL)"
    MM_6,                                                       // 5 = Mode 6: "SCHWABEN - MM MINUTEN NACH (VIERTEL NACH, HALB, DREIVIERTEL)"
    MM_7,                                                       // 6 = Mode 7: "WESSI - MM MINUTEN NACH (VIERTEL, HALB, DREIVIERTEL)"
    MM_8,                                                       // 7 = Mode 8: "MM"
    MM_9,                                                       // 8 = Mode 9: "UND MM MINUTEN"
    MM_10,                                                      // 9 = Mode 10: "MM MINUTEN VOR"
    MM_11,                                                      // 10 = Mode 11: "TEMPERATUR "CC GRAD""
    MINUTE_MODES_COUNT                                          // Number of MinuteModes
};

enum WordPos
{
    WP_END_OF_WORDS,                                            // 0 = "0_END_OF_WORDS" = ""
    WP_ES,                                                      // 1 = "1_ES" = "ES"
    WP_IT,                                                      // 2 = "2_IT" = "IT"
    WP_IST,                                                     // 3 = "3_IST" = "IST"
    WP_IS,                                                      // 4 = "4_IS" = "IS"
    WP_VIERTEL_1,                                               // 5 = "3_VIERTEL" = "VIERTEL"
    WP_EIN_1,                                                   // 6 = "4_EIN" = "EIN"
    WP_EINS_1,                                                  // 7 = "5_EINS" = "EINS"
    WP_IN,                                                      // 8 = "6_IN" = "IN"
    WP_DREI_1,                                                  // 9 = "7_DREI" = "DREI"
    WP_EIN_2,                                                   // 10 = "8_EIN" = "EIN"
    WP_EINE_1,                                                  // 11 = "9_EINE" = "EINE"
    WP_EINER,                                                   // 12 = "10_EINER" = "EINER"
    WP_SECH_1,                                                  // 13 = "11_SECH" = "SECH"
    WP_SECHS_1,                                                 // 14 = "12_SECHS" = "SECHS"
    WP_SIEB_1,                                                  // 15 = "13_SIEB" = "SIEB"
    WP_SIEBEN_1,                                                // 16 = "14_SIEBEN" = "SIEBEN"
    WP_ELF_1,                                                   // 17 = "15_ELF" = "ELF"
    WP_FUENF_1,                                                 // 18 = "16_FUENF" = "FÜNF"
    WP_NEUN_1,                                                  // 19 = "17_NEUN" = "NEUN"
    WP_VIER_1,                                                  // 20 = "18_VIER" = "VIER"
    WP_ACHT_1,                                                  // 21 = "19_ACHT" = "ACHT"
    WP_NULL_1,                                                  // 22 = "20_NULL" = "NULL"
    WP_ZWEI_1,                                                  // 23 = "21_ZWEI" = "ZWEI"
    WP_ZWOELF_1,                                                // 24 = "22_ZWOELF" = "ZWÖLF"
    WP_ZEHN_1,                                                  // 25 = "23_ZEHN" = "ZEHN"
    WP_UND_1,                                                   // 26 = "24_UND" = "UND"
    WP_ZWANZIG_1,                                               // 27 = "25_ZWANZIG" = "ZWANZIG"
    WP_VIERZIG_1,                                               // 28 = "26_VIERZIG" = "VIERZIG"
    WP_DREISSIG_1,                                              // 29 = "27_DREISSIG" = "DREISSIG"
    WP_FUENFZIG_1,                                              // 30 = "28_FUENFZIG" = "FÜNFZIG"
    WP_UHR_1,                                                   // 31 = "29_UHR" = "UHR"
    WP_MINUTE_1,                                                // 32 = "30_MINUTE" = "MINUTE"
    WP_MINUTEN_1,                                               // 33 = "31_MINUTEN" = "MINUTEN"
    WP_VOR_1,                                                   // 34 = "32_VOR" = "VOR"
    WP_UND_2,                                                   // 35 = "33_UND" = "UND"
    WP_NACH_1,                                                  // 36 = "34_NACH" = "NACH"
    WP_EIN_3,                                                   // 37 = "35_EIN" = "EIN"
    WP_DREIVIERTEL,                                             // 38 = "36_DREIVIERTEL" = "DREIVIERTEL"
    WP_VIERTEL_2,                                               // 39 = "37_VIERTEL" = "VIERTEL"
    WP_HALB,                                                    // 40 = "38_HALB" = "HALB"
    WP_SIEB_2,                                                  // 41 = "39_SIEB" = "SIEB"
    WP_SIEBEN_2,                                                // 42 = "40_SIEBEN" = "SIEBEN"
    WP_NEUN_2,                                                  // 43 = "41_NEUN" = "NEUN"
    WP_NULL_2,                                                  // 44 = "42_NULL" = "NULL"
    WP_ZWEI_2,                                                  // 45 = "43_ZWEI" = "ZWEI"
    WP_EIN_4,                                                   // 46 = "44_EIN" = "EIN"
    WP_EINE_2,                                                  // 47 = "45_EINE" = "EINE"
    WP_FUENF_2,                                                 // 48 = "46_FUENF" = "FÜNF"
    WP_SECH_2,                                                  // 49 = "47_SECH" = "SECH"
    WP_SECHS_2,                                                 // 50 = "48_SECHS" = "SECHS"
    WP_ACHT_2,                                                  // 51 = "49_ACHT" = "ACHT"
    WP_VIER_2,                                                  // 52 = "50_VIER" = "VIER"
    WP_DREI_2,                                                  // 53 = "51_DREI" = "DREI"
    WP_EIN_5,                                                   // 54 = "52_EIN" = "EIN"
    WP_EINS_2,                                                  // 55 = "53_EINS" = "EINS"
    WP_UND_3,                                                   // 56 = "54_UND" = "UND"
    WP_ELF_2,                                                   // 57 = "55_ELF" = "ELF"
    WP_ZEHN_2,                                                  // 58 = "56_ZEHN" = "ZEHN"
    WP_ZWANZIG_2,                                               // 59 = "57_ZWANZIG" = "ZWANZIG"
    WP_GRAD,                                                    // 60 = "58_GRAD" = "GRAD"
    WP_DREISSIG_2,                                              // 61 = "59_DREISSIG" = "DREISSIG"
    WP_VIERZIG_2,                                               // 62 = "60_VIERZIG" = "VIERZIG"
    WP_ZWOELF_2,                                                // 63 = "61_ZWÖLF" = "ZWÖLF"
    WP_FUENFZIG_2,                                              // 64 = "62_FUENFZIG" = "FÜNFZIG"
    WP_MINUTE_2,                                                // 65 = "63_MINUTE" = "MINUTE"
    WP_MINUTEN_2,                                               // 66 = "64_MINUTEN" = "MINUTEN"
    WP_UHR_2,                                                   // 67 = "65_UHR" = "UHR"
    WP_FRUEH,                                                   // 68 = "66_FRUEH" = "FRÜH"
    WP_VOR_2,                                                   // 69 = "67_VOR" = "VOR"
    WP_ABENDS,                                                  // 70 = "68_ABENDS" = "ABENDS"
    WP_MITTERNACHT,                                             // 71 = "69_MITTERNACHT" = "MITTERNACHT"
    WP_NACH_2,                                                  // 72 = "70_NACH" = "NACH"
    WP_NACHTS,                                                  // 73 = "71_NACHTS" = "NACHTS"
    WP_MORGENS,                                                 // 74 = "72_MORGENS" = "MORGENS"
    WP_WARM,                                                    // 75 = "73_WARM" = "WARM"
    WP_MITTAGS,                                                 // 76 = "74_MITTAGS" = "MITTAGS"
    WP_COUNT,                                                   // number of words
};

extern const struct Modes tbl_modes[MODES_COUNT];
extern const uint8_t tbl_it_is[IT_IS_MODES_COUNT][2];
extern const uint8_t tbl_hours[HOUR_MODES_COUNT][HOUR_COUNT][MAX_HOUR_WORDS];
extern const struct MinuteDisplay tbl_minutes[MINUTE_MODES_COUNT][MINUTE_COUNT];
extern const struct WordIllu illumination[WP_COUNT];

#endif // TABLES_H
#endif // WCLOCK24H == 1
