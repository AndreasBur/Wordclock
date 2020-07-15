/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables12h.h - extern declarations for WordClock12h
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "wclock24h-config.h"
#if WCLOCK24H == 0

#ifndef TABLES12H_H
#define TABLES12H_H

#include <stdint.h>

#define WC_ROWS                 10
#define WC_COLUMNS              11

#define MODES_COUNT             8                       // 4 german, 2 swiss, 2 english
#define MINUTE_COUNT            12

#define HOUR_MODES_COUNT        10                      // 2 german, 2 swiss1, 2 swiss2, 2 english1, 2 english2
#define HOUR_COUNT              12
#define MAX_HOUR_WORDS          3                       // max is 3: "five o clock"

#define IT_IS_MODES_COUNT       3

#define MAX_MINUTE_WORDS        4                       // 3 + 1 for termination

// possible values of flags in struct MinuteDisplay12:
#define MDF_IT_IS_GERMAN        0x00                    // GERMAN
#define MDF_IT_IS_SWISS         0x01                    // SWISS1 and SWISS2
#define MDF_IT_IS_ENGLISH       0x02                    // ENGLISH1 and ENGLISH2
#define MDF_IT_IS_MASK          0x03                    // 2 bits reserved for it-is language, correct IT_IS_MODES_COUNT if you extend languages!

#define MDF_HOUR_OFFSET_0       0x00
#define MDF_HOUR_OFFSET_1       0x04
#define MDF_HOUR_OFFSET_MASK    0x04                    // 1 bit reserved for hour offset mask

struct MinuteDisplay12
{
    uint8_t hour_mode;
    uint8_t flags;
    uint8_t wordIdx[MAX_MINUTE_WORDS];
};

struct WordIllu
{
    uint8_t row;
    uint8_t col;
    uint8_t len;
};

extern const char * tbl_modes[MODES_COUNT];

enum wc12h_words
{
    WP_END_OF_WORDS,

    // GERMAN:
    WP_ES,
    WP_IST,
    WP_FUENF_1,
    WP_ZEHN_1,
    WP_ZWANZIG,
    WP_DREI_1,
    WP_VIER,
    WP_VIERTEL,
    WP_DREIVIERTEL,
    WP_NACH,
    WP_VOR,
    WP_HALB,
    WP_ZWOELF,
    WP_ZWEI,
    WP_EIN,
    WP_EINS,
    WP_SIEBEN,
    WP_DREI_2,
    WP_FUENF_2,
    WP_ELF,
    WP_NEUN,
    WP_VIER_2,
    WP_ACHT,
    WP_ZEHN_2,
    WP_SECHS,
    WP_UHR,

    // SWISS1
    WP_ES_CH,
    WP_ISCH_CH,
    WP_FUEF_CH,
    WP_VIERTU_CH,
    WP_ZAEAE_CH,
    WP_ZWAENZG_CH,
    WP_VOR_CH,
    WP_AB_CH,
    WP_HAUBI_CH,
    WP_EIS_CH,
    WP_ZWOEI_CH,
    WP_DRUE_CH,
    WP_VIERI_CH,
    WP_FUEFI_CH,
    WP_SAECHSI_CH,
    WP_SIBNI_CH,
    WP_ACHTI_CH,
    WP_NUENI_CH,
    WP_ZAENI_CH,
    WP_EUFI_CH,
    WP_ZWOEUFI_CH,
    WP_UHR_CH,

    // SWISS2
    WP_ES_CH2,
    WP_ESCH_CH2,
    WP_FOIF_CH2,
    WP_VIERTU_CH2,
    WP_ZAEAE_CH2,
    WP_ZWAENZ_CH2,
    WP_VOR_CH2,
    WP_AB_CH2,
    WP_HAUBI_CH2,
    WP_EIS_CH2,
    WP_ZWOEI_CH2,
    WP_DRUE_CH2,
    WP_VIERI_CH2,
    WP_FOEIFI_CH2,
    WP_SAECHSI_CH2,
    WP_SEBNI_CH2,
    WP_ACHTI_CH2,
    WP_NUENI_CH2,
    WP_ZAENI_CH2,
    WP_ELFI_CH2,
    WP_ZWOELFI_CH2,
    WP_UHR_CH2,

    // ENGLISH1
    WP_IT_EN,
    WP_IS_EN,
    WP_AM_EN,
    WP_PM_EN,
    WP_QUARTER_EN,
    WP_TWENTY_EN,
    WP_FIVE_1_EN,
    WP_HALF_EN,
    WP_TEN_1_EN,
    WP_TO_EN,
    WP_PAST_EN,
    WP_NINE_EN,
    WP_ONE_EN,
    WP_SIX_EN,
    WP_THREE_EN,
    WP_FOUR_EN,
    WP_FIVE_2_EN,
    WP_TWO_EN,
    WP_EIGHT_EN,
    WP_ELEVEN_EN,
    WP_SEVEN_EN,
    WP_TWELVE_EN,
    WP_TEN_2_EN,
    WP_OCLOCK_EN,

    // ENGLISH2
    WP_IT_EN2,
    WP_IS_EN2,
    WP_QUARTER_EN2,
    WP_TWENTY_EN2,
    WP_FIVE_1_EN2,
    WP_HALF_EN2,
    WP_TEN_1_EN2,
    WP_TO_EN2,
    WP_PAST_EN2,
    WP_NINE_EN2,
    WP_ONE_EN2,
    WP_SIX_EN2,
    WP_THREE_EN2,
    WP_FOUR_EN2,
    WP_FIVE_2_EN2,
    WP_TWO_EN2,
    WP_EIGHT_EN2,
    WP_ELEVEN_EN2,
    WP_SEVEN_EN2,
    WP_TWELVE_EN2,
    WP_TEN_2_EN2,
    WP_O_EN2,
    WP_CLOCK_EN2,
    WP_COUNT
};

extern const char *     tbl_modes[MODES_COUNT];
extern const uint8_t    tbl_it_is[IT_IS_MODES_COUNT][2];
extern const uint8_t    tbl_hours[HOUR_MODES_COUNT][HOUR_COUNT][MAX_HOUR_WORDS];
extern const struct     MinuteDisplay12 tbl_minutes[MODES_COUNT][MINUTE_COUNT];
extern const struct     WordIllu illumination[WP_COUNT];

#endif // TABLES12H_H

#endif
