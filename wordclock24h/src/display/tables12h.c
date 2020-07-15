/*-------------------------------------------------------------------------------------------------------------------------------------------
 * tables12h.c - tables for WordClock12h
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * Supported Layouts:
 *
 * German:
 *   ESKISTLFÜNF
 *   ZEHNZWANZIG
 *   DREIVIERTEL
 *   TGNACHVORJM
 *   HALBQZWÖLFP
 *   ZWEINSIEBEN
 *   KDREIRHFÜNF
 *   ELFNEUNVIER
 *   WACHTZEHNRS
 *   BSECHSFMUHR
 *
 * Swiss German Variant 1:
 *   ESKISCHAFÜF
 *   VIERTUBFZÄÄ
 *   ZWÄNZGSIVOR
 *   ABOHAUBIEPM
 *   EISZWÖISDRÜ
 *   VIERIFÜFIQT
 *   SÄCHSISIBNI
 *   ACHTINÜNIEL
 *   ZÄNIERBEUFI
 *   ZWÖUFIAMUHR
 *
 * Swiss German Variant 2:
 *   ESKESCHAZÄÄ
 *   FÖIFCVIERTU
 *   ZWÄNZGSIVOR
 *   ABOHAUBIEGE
 *   EISZWÖISDRÜ
 *   VIERITFÖIFI
 *   SÄCHSISEBNI
 *   ACHTIENÜNIL
 *   ZÄNIERBRLFI
 *   ZWÖLFINAUHR
 *
 * English Variant 1:
 *   ITLISASAMPM
 *   ACQUARTERDC
 *   TWENTYFIVEX
 *   HALFSTENFTO
 *   PASTERUNINE
 *   ONESIXTHREE
 *   FOURFIVETWO
 *   EIGHTELEVEN
 *   SEVENTWELVE
 *   TENSEOCLOCK
 *
 * English Variant 2:
 *   ITKISGHALFE
 *   TENYQUARTER
 *   DTWENTYFIVE
 *   TOPASTEFOUR
 *   FIVETWONINE
 *   THREETWELVE
 *   BELEVENONES
 *   SEVENWEIGHT
 *   ITENSIXTIES
 *   TINEOICLOCK
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "wclock24h-config.h"
#if WCLOCK24H == 0

#include "tables12h.h"

const char * tbl_modes[MODES_COUNT] =
{
    "WESSI",
    "OSSI",
    "RHEIN-RUHR",
    "SCHWABEN",
    "SCHWEIZERDEUTSCH 1",
    "SCHWEIZERDEUTSCH 2",
    "ENGLISCH 1",
    "ENGLISCH 2",
};

const uint8_t tbl_it_is[IT_IS_MODES_COUNT][2] =
{
    { WP_ES, WP_IST },
    { WP_ES_CH, WP_ISCH_CH },
    { WP_IT_EN, WP_IS_EN }
};

const uint8_t tbl_hours[HOUR_MODES_COUNT][HOUR_COUNT][MAX_HOUR_WORDS] =
{
    // GERMAN
    {                                                               // tbl_hours[0][] = hh:00
        {WP_ZWOELF,   WP_UHR},                                      // 00:00
        {WP_EIN,      WP_UHR},                                      // 01:00
        {WP_ZWEI,     WP_UHR},                                      // 02:00
        {WP_DREI_2,   WP_UHR},                                      // 03:00
        {WP_VIER_2,   WP_UHR},                                      // 04:00
        {WP_FUENF_2,  WP_UHR},                                      // 05:00
        {WP_SECHS,    WP_UHR},                                      // 06:00
        {WP_SIEBEN,   WP_UHR},                                      // 07:00
        {WP_ACHT,     WP_UHR},                                      // 08:00
        {WP_NEUN,     WP_UHR},                                      // 09:00
        {WP_ZEHN_2,   WP_UHR},                                      // 10:00
        {WP_ELF,      WP_UHR}                                       // 11:00
    },
    {                                                               // tbl_hours[1][] = hh:mm (not hh:00)
        {WP_ZWOELF},                                                // 00:mm
        {WP_EINS},                                                  // 01:mm
        {WP_ZWEI},                                                  // 02:mm
        {WP_DREI_2},                                                // 03:mm
        {WP_VIER_2},                                                // 04:mm
        {WP_FUENF_2},                                               // 05:mm
        {WP_SECHS},                                                 // 06:mm
        {WP_SIEBEN},                                                // 07:mm
        {WP_ACHT},                                                  // 08:mm
        {WP_NEUN},                                                  // 09:mm
        {WP_ZEHN_2},                                                // 10:mm
        {WP_ELF}                                                    // 11:mm
    },

    // SWISS1
    {                                                               // tbl_hours[2][] = hh:00
        {WP_ZWOEUFI_CH, WP_UHR_CH},                                 // 00:00
        {WP_EIS_CH,     WP_UHR_CH},                                 // 01:00
        {WP_ZWOEI_CH,   WP_UHR_CH},                                 // 02:00
        {WP_DRUE_CH,    WP_UHR_CH},                                 // 03:00
        {WP_VIERI_CH,   WP_UHR_CH},                                 // 04:00
        {WP_FUEFI_CH,   WP_UHR_CH},                                 // 05:00
        {WP_SAECHSI_CH, WP_UHR_CH},                                 // 06:00
        {WP_SIBNI_CH,   WP_UHR_CH},                                 // 07:00
        {WP_ACHTI_CH,   WP_UHR_CH},                                 // 08:00
        {WP_NUENI_CH,   WP_UHR_CH},                                 // 09:00
        {WP_ZAENI_CH,   WP_UHR_CH},                                 // 10:00
        {WP_EUFI_CH,    WP_UHR_CH}                                  // 11:00
    },
    {                                                               // tbl_hours[3][] = hh:mm (not hh:00)
        {WP_ZWOEUFI_CH},                                            // 00:mm
        {WP_EIS_CH},                                                // 01:mm
        {WP_ZWOEI_CH},                                              // 02:mm
        {WP_DRUE_CH},                                               // 03:mm
        {WP_VIERI_CH},                                              // 04:mm
        {WP_FUEFI_CH},                                              // 05:mm
        {WP_SAECHSI_CH},                                            // 06:mm
        {WP_SIBNI_CH},                                              // 07:mm
        {WP_ACHTI_CH},                                              // 08:mm
        {WP_NUENI_CH},                                              // 09:mm
        {WP_ZAENI_CH},                                              // 10:mm
        {WP_EUFI_CH}                                                // 11:mm
    },

#if 0
    // SWISS2
    {                                                               // tbl_hours[4][] = hh:00
        {WP_ZWOELFI_CH2, WP_UHR_CH2},                               // 00:00
        {WP_EIS_CH2,     WP_UHR_CH2},                               // 01:00
        {WP_ZWOEI_CH2,   WP_UHR_CH2},                               // 02:00
        {WP_DRUE_CH2,    WP_UHR_CH2},                               // 03:00
        {WP_VIERI_CH2,   WP_UHR_CH2},                               // 04:00
        {WP_FOEIFI_CH2,  WP_UHR_CH2},                               // 05:00
        {WP_SAECHSI_CH2, WP_UHR_CH2},                               // 06:00
        {WP_SEBNI_CH2,   WP_UHR_CH2},                               // 07:00
        {WP_ACHTI_CH2,   WP_UHR_CH2},                               // 08:00
        {WP_NUENI_CH2,   WP_UHR_CH2},                               // 09:00
        {WP_ZAENI_CH2,   WP_UHR_CH2},                               // 10:00
        {WP_ELFI_CH2,    WP_UHR_CH2}                                // 11:00
    },
#else
    // SWISS2
    {                                                               // tbl_hours[4][] = hh:00
        {WP_ZWOELFI_CH2, },                                         // 00:00
        {WP_EIS_CH2,     },                                         // 01:00
        {WP_ZWOEI_CH2,   },                                         // 02:00
        {WP_DRUE_CH2,    },                                         // 03:00
        {WP_VIERI_CH2,   },                                         // 04:00
        {WP_FOEIFI_CH2,  },                                         // 05:00
        {WP_SAECHSI_CH2, },                                         // 06:00
        {WP_SEBNI_CH2,   },                                         // 07:00
        {WP_ACHTI_CH2,   },                                         // 08:00
        {WP_NUENI_CH2,   },                                         // 09:00
        {WP_ZAENI_CH2,   },                                         // 10:00
        {WP_ELFI_CH2,    }                                          // 11:00
    },
#endif
    {                                                               // tbl_hours[5][] = hh:mm (not hh:00)
        {WP_ZWOELFI_CH2},                                           // 00:mm
        {WP_EIS_CH2},                                               // 01:mm
        {WP_ZWOEI_CH2},                                             // 02:mm
        {WP_DRUE_CH2},                                              // 03:mm
        {WP_VIERI_CH2},                                             // 04:mm
        {WP_FOEIFI_CH2},                                            // 05:mm
        {WP_SAECHSI_CH2},                                           // 06:mm
        {WP_SEBNI_CH2},                                             // 07:mm
        {WP_ACHTI_CH2},                                             // 08:mm
        {WP_NUENI_CH2},                                             // 09:mm
        {WP_ZAENI_CH2},                                             // 10:mm
        {WP_ELFI_CH2}                                               // 11:mm
    },

    // ENGLISH1
    {                                                               // tbl_hours[6][] = hh:00
        {WP_TWELVE_EN, WP_OCLOCK_EN},                               // 00:00
        {WP_ONE_EN,    WP_OCLOCK_EN},                               // 01:00
        {WP_TWO_EN,    WP_OCLOCK_EN},                               // 02:00
        {WP_THREE_EN,  WP_OCLOCK_EN},                               // 03:00
        {WP_FOUR_EN,   WP_OCLOCK_EN},                               // 04:00
        {WP_FIVE_2_EN, WP_OCLOCK_EN},                               // 05:00
        {WP_SIX_EN,    WP_OCLOCK_EN},                               // 06:00
        {WP_SEVEN_EN,  WP_OCLOCK_EN},                               // 07:00
        {WP_EIGHT_EN,  WP_OCLOCK_EN},                               // 08:00
        {WP_NINE_EN,   WP_OCLOCK_EN},                               // 09:00
        {WP_TEN_2_EN,  WP_OCLOCK_EN},                               // 10:00
        {WP_ELEVEN_EN, WP_OCLOCK_EN}                                // 11:00
    },
    {                                                               // tbl_hours[7][] = hh:mm (not hh:00)
        {WP_TWELVE_EN},                                             // 00:mm
        {WP_ONE_EN},                                                // 01:mm
        {WP_TWO_EN},                                                // 02:mm
        {WP_THREE_EN},                                              // 03:mm
        {WP_FOUR_EN},                                               // 04:mm
        {WP_FIVE_2_EN},                                             // 05:mm
        {WP_SIX_EN},                                                // 06:mm
        {WP_SEVEN_EN},                                              // 07:mm
        {WP_EIGHT_EN},                                              // 08:mm
        {WP_NINE_EN},                                               // 09:mm
        {WP_TEN_2_EN},                                              // 10:mm
        {WP_ELEVEN_EN}                                              // 11:mm
    },

    // ENGLISH2
    {                                                               // tbl_hours[8][] = hh:00
        {WP_TWELVE_EN2, WP_O_EN2, WP_CLOCK_EN2},                    // 00:00
        {WP_ONE_EN2,    WP_O_EN2, WP_CLOCK_EN2},                    // 01:00
        {WP_TWO_EN2,    WP_O_EN2, WP_CLOCK_EN2},                    // 02:00
        {WP_THREE_EN2,  WP_O_EN2, WP_CLOCK_EN2},                    // 03:00
        {WP_FOUR_EN2,   WP_O_EN2, WP_CLOCK_EN2},                    // 04:00
        {WP_FIVE_2_EN2, WP_O_EN2, WP_CLOCK_EN2},                    // 05:00
        {WP_SIX_EN2,    WP_O_EN2, WP_CLOCK_EN2},                    // 06:00
        {WP_SEVEN_EN2,  WP_O_EN2, WP_CLOCK_EN2},                    // 07:00
        {WP_EIGHT_EN2,  WP_O_EN2, WP_CLOCK_EN2},                    // 08:00
        {WP_NINE_EN2,   WP_O_EN2, WP_CLOCK_EN2},                    // 09:00
        {WP_TEN_2_EN2,  WP_O_EN2, WP_CLOCK_EN2},                    // 10:00
        {WP_ELEVEN_EN2, WP_O_EN2, WP_CLOCK_EN2}                     // 11:00
    },
    {                                                               // tbl_hours[9][] = hh:mm (not hh:00)
        {WP_TWELVE_EN2},                                            // 00:mm
        {WP_ONE_EN2},                                               // 01:mm
        {WP_TWO_EN2},                                               // 02:mm
        {WP_THREE_EN2},                                             // 03:mm
        {WP_FOUR_EN2},                                              // 04:mm
        {WP_FIVE_2_EN2},                                            // 05:mm
        {WP_SIX_EN2},                                               // 06:mm
        {WP_SEVEN_EN2},                                             // 07:mm
        {WP_EIGHT_EN2},                                             // 08:mm
        {WP_NINE_EN2},                                              // 09:mm
        {WP_TEN_2_EN2},                                             // 10:mm
        {WP_ELEVEN_EN2}                                             // 11:mm
    }
};

const struct MinuteDisplay12 tbl_minutes[MODES_COUNT][MINUTE_COUNT] =
{
    {                                                               // tbl_minutes[0][] = WESSI
        {0, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_FUENF_1, WP_NACH                          }},         // 05
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZEHN_1,  WP_NACH                          }},         // 10
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_VIERTEL, WP_NACH                          }},         // 15
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,  WP_VOR,  WP_HALB                 }},         // 20
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1, WP_VOR,  WP_HALB                 }},         // 25
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_HALB                                      }},         // 30
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1, WP_NACH, WP_HALB                 }},         // 35
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,  WP_NACH, WP_HALB                 }},         // 40
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_VIERTEL, WP_VOR                           }},         // 45
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,  WP_VOR                           }},         // 50
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1, WP_VOR                           }},         // 55
    },

    {                                                               // tbl_minutes[1][] = OSSI
        {0, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_FUENF_1,     WP_NACH                      }},         // 05
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZEHN_1,      WP_NACH                      }},         // 10
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_VIERTEL,                                  }},         // 45
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,      WP_VOR,  WP_HALB             }},         // 20
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR,  WP_HALB             }},         // 25
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_HALB                                      }},         // 30
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_NACH, WP_HALB             }},         // 35
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,      WP_NACH, WP_HALB             }},         // 40
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_DREIVIERTEL                               }},         // 15
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,      WP_VOR                       }},         // 50
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR                       }},         // 55
    },

    {                                                               // tbl_minutes[2][] = RHEIN-RUHR
        {0, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_FUENF_1,     WP_NACH                      }},         // 05
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZEHN_1,      WP_NACH                      }},         // 10
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_VIERTEL,     WP_NACH                      }},         // 15
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZWANZIG,     WP_NACH                      }},         // 20
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR,  WP_HALB             }},         // 25
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_HALB                                      }},         // 30
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_NACH, WP_HALB             }},         // 35
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZWANZIG,     WP_VOR                       }},         // 40
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_VIERTEL,     WP_VOR                       }},         // 45
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,      WP_VOR                       }},         // 50
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR                       }},         // 55
    },

    {                                                               // tbl_minutes[3][] = SCHWABEN
        {0, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_FUENF_1,     WP_NACH                      }},         // 05
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZEHN_1,      WP_NACH                      }},         // 10
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_VIERTEL,                                  }},         // 45
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_0, {WP_ZWANZIG,     WP_NACH                      }},         // 20
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR,  WP_HALB             }},         // 25
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_HALB                                      }},         // 30
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_NACH, WP_HALB             }},         // 35
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZWANZIG,     WP_VOR                       }},         // 40
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_DREIVIERTEL,                              }},         // 15
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_ZEHN_1,      WP_VOR                       }},         // 50
        {1, MDF_IT_IS_GERMAN | MDF_HOUR_OFFSET_1, {WP_FUENF_1,     WP_VOR                       }},         // 55
    },
    {                                                               // tbl_minutes[4][] = SCHWEIZERDEUTSCH1
        {2, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_FUEF_CH,    WP_AB_CH                      }},         // 05
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_ZAEAE_CH,   WP_AB_CH                      }},         // 10
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_VIERTU_CH,  WP_AB_CH                      }},         // 15
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_ZWAENZG_CH, WP_AB_CH                      }},         // 20
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FUEF_CH,    WP_VOR_CH, WP_HAUBI_CH        }},         // 25
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_HAUBI_CH                                  }},         // 30
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FUEF_CH,    WP_AB_CH,  WP_HAUBI_CH        }},         // 35
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_ZWAENZG_CH, WP_VOR_CH                     }},         // 40
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_VIERTU_CH,  WP_VOR_CH                     }},         // 45
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_ZAEAE_CH,   WP_VOR_CH                     }},         // 50
        {3, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FUEF_CH,    WP_VOR_CH                     }},         // 55
    },
    {                                                               // tbl_minutes[5][] = SCHWEIZERDEUTSCH2
        {4, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {0}},                                                     // 00
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_FOIF_CH2,    WP_AB_CH2                    }},         // 05
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_ZAEAE_CH2,   WP_AB_CH2                    }},         // 10
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_VIERTU_CH2,  WP_AB_CH2                    }},         // 15
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_0, {WP_ZWAENZ_CH2,  WP_AB_CH2                    }},         // 20
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FOIF_CH2,    WP_VOR_CH2, WP_HAUBI_CH2     }},         // 25
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_HAUBI_CH2                                 }},         // 30
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FOIF_CH2,    WP_AB_CH2,  WP_HAUBI_CH2     }},         // 35
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_ZWAENZ_CH2,  WP_VOR_CH2                   }},         // 40
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_VIERTU_CH2,  WP_VOR_CH2                   }},         // 45
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_ZAEAE_CH2,   WP_VOR_CH2                   }},         // 50
        {5, MDF_IT_IS_SWISS  | MDF_HOUR_OFFSET_1, {WP_FOIF_CH2,    WP_VOR_CH2                   }},         // 55
    },
    {                                                               // tbl_minutes[6][] = ENGLISH var. 1
        {6, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {0}},                                                    // 00
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_FIVE_1_EN,  WP_PAST_EN                   }},         // 05
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TEN_1_EN,   WP_PAST_EN                   }},         // 10
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_QUARTER_EN, WP_PAST_EN                   }},         // 15
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TWENTY_EN,  WP_PAST_EN                   }},         // 20
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TWENTY_EN,  WP_FIVE_1_EN, WP_PAST_EN     }},         // 25
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_HALF_EN                                  }},         // 30
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TWENTY_EN,  WP_FIVE_1_EN, WP_TO_EN       }},         // 35
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TWENTY_EN,  WP_TO_EN                     }},         // 40
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_QUARTER_EN, WP_TO_EN                     }},         // 45
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TEN_1_EN,   WP_TO_EN                     }},         // 50
        {7, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_FIVE_1_EN,  WP_TO_EN                     }},         // 55
    },
    {                                                               // tbl_minutes[7][] = ENGLISH var. 2
        {8, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {0}},                                                    // 00
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_FIVE_1_EN2,  WP_PAST_EN2                 }},         // 05
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TEN_1_EN2,   WP_PAST_EN2                 }},         // 10
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_QUARTER_EN2, WP_PAST_EN2                 }},         // 15
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TWENTY_EN2,  WP_PAST_EN2                 }},         // 20
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_TWENTY_EN2,  WP_FIVE_1_EN2, WP_PAST_EN2  }},         // 25
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_0, {WP_HALF_EN2                                 }},         // 30
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TWENTY_EN2,  WP_FIVE_1_EN2, WP_TO_EN2    }},         // 35
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TWENTY_EN2,  WP_TO_EN2                   }},         // 40
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_QUARTER_EN2, WP_TO_EN2                   }},         // 45
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_TEN_1_EN2,   WP_TO_EN2                   }},         // 50
        {9, MDF_IT_IS_ENGLISH | MDF_HOUR_OFFSET_1, {WP_FIVE_1_EN2,  WP_TO_EN2                   }},         // 55
    }
};

const struct WordIllu illumination[WP_COUNT] =
{
    {0,0,0},                                                        //  0 = WP_END_OF_WORDS = ""
    {0,0,2},                                                        //  1 = WP_ES           = "ES"
    {0,3,3},                                                        //  2 = WP_IST          = "IST"
    {0,7,4},                                                        //  3 = WP_FUENF_1      = "FÜNF"
    {1,0,4},                                                        //  4 = WP_ZEHN_1       = "ZEHN"
    {1,4,7},                                                        //  5 = WP_ZWANZIG      = "ZWANZIG"
    {2,0,4},                                                        //  6 = WP_DREI_1       = "DREI"
    {2,4,4},                                                        //  7 = WP_VIER         = "VIER"
    {2,4,7},                                                        //  8 = WP_VIERTEL      = "VIERTEL"
    {2,0,11},                                                       //  9 = WP_DREIVIERTEL  = "DREIVIERTEL"
    {3,2,4},                                                        // 10 = WP_NACH         = "NACH"
    {3,6,3},                                                        // 11 = WP_VOR          = "VOR"
    {4,0,4},                                                        // 12 = WP_HALB         = "HALB"
    {4,5,5},                                                        // 13 = WP_ZWOELF       = "ZWÖLF"
    {5,0,4},                                                        // 14 = WP_ZWEI         = "ZWEI"
    {5,2,3},                                                        // 15 = WP_EIN          = "EIN"
    {5,2,4},                                                        // 16 = WP_EINS         = "EINS"
    {5,5,6},                                                        // 17 = WP_SIEBEN       = "SIEBEN"
    {6,1,4},                                                        // 18 = WP_DREI_2       = "DREI"
    {6,7,4},                                                        // 19 = WP_FUENF_2      = "FÜNF"
    {7,0,3},                                                        // 20 = WP_ELF          = "ELF"
    {7,3,4},                                                        // 21 = WP_NEUN         = "NEUN"
    {7,7,4},                                                        // 22 = WP_VIER_2       = "VIER"
    {8,1,4},                                                        // 23 = WP_ACHT         = "ACHT"
    {8,5,4},                                                        // 24 = WP_ZEHN_2       = "ZEHN"
    {9,1,5},                                                        // 25 = WP_SECHS        = "SECHS"
    {9,8,3},                                                        // 26 = WP_UHR          = "UHR"

    {0,0,2},                                                        // 27 = WP_ES_CH        = "ES"
    {0,3,4},                                                        // 28 = WP_ISCH_CH      = "ISCH"
    {0,8,3},                                                        // 29 = WP_FUEF_CH      = "FÜF"
    {1,0,6},                                                        // 30 = WP_VIERTU_CH    = "VIERTU"
    {1,8,3},                                                        // 31 = WP_ZAEAE_CH     = "ZÄÄ"
    {2,0,6},                                                        // 32 = WP_ZWAENZG_CH   = "ZWÄNZG"
    {2,8,3},                                                        // 33 = WP_VOR_CH       = "VOR"
    {3,0,2},                                                        // 34 = WP_AB_CH        = "AB"
    {3,3,5},                                                        // 35 = WP_HAUBI_CH     = "HAUBI"
    {4,0,3},                                                        // 36 = WP_EIS_CH       = "EIS"
    {4,3,4},                                                        // 37 = WP_ZWOEI_CH     = "ZWÖI"
    {4,8,3},                                                        // 38 = WP_DRUE_CH      = "DRÜ"
    {5,0,5},                                                        // 39 = WP_VIERI_CH     = "VIERI"
    {5,5,4},                                                        // 40 = WP_FUEFI_CH     = "FÜFI"
    {6,0,6},                                                        // 41 = WP_SAECHSI_CH   = "SÄCHSI"
    {6,6,5},                                                        // 42 = WP_SIBNI_CH     = "SIBNI"
    {7,0,5},                                                        // 43 = WP_ACHTI_CH     = "ACHTI"
    {7,5,4},                                                        // 44 = WP_NUENI_CH     = "NÜNI"
    {8,0,4},                                                        // 45 = WP_ZAENI_CH     = "ZÄNI"
    {8,7,4},                                                        // 46 = WP_EUFI_CH      = "EUFI"
    {9,0,6},                                                        // 47 = WP_ZWOEFI_CH    = "ZWÖUFI"
    {9,8,3},                                                        // 48 = WP_UHR_CH       = "UHR"

    {0,0,2},                                                        // 49 = WP_ES_CH2       = "ES"
    {0,3,4},                                                        // 50 = WP_ESCH_CH2     = "ESCH"
    {1,0,4},                                                        // 51 = WP_FOIF_CH2     = "FOIF"
    {1,5,6},                                                        // 52 = WP_VIERTU_CH2   = "VIERTU"
    {0,8,3},                                                        // 53 = WP_ZAEAE_CH2    = "ZÄÄ"
    {2,0,6},                                                        // 54 = WP_ZWAENZ_CH2   = "ZWÄNZ"
    {2,8,3},                                                        // 55 = WP_VOR_CH2      = "VOR"
    {3,0,2},                                                        // 56 = WP_AB_CH2       = "AB"
    {3,3,5},                                                        // 57 = WP_HAUBI_CH2    = "HAUBI"
    {4,0,3},                                                        // 58 = WP_EIS_CH2      = "EIS"
    {4,3,4},                                                        // 59 = WP_ZWOEI_CH2    = "ZWÖI"
    {4,8,3},                                                        // 60 = WP_DRUE_CH2     = "DRÜ"
    {5,0,5},                                                        // 61 = WP_VIERI_CH2    = "VIERI"
    {5,6,5},                                                        // 62 = WP_FOEIFI_CH2   = "FÖIFI"
    {6,0,6},                                                        // 63 = WP_SAECHSI_CH2  = "SÄCHSI"
    {6,6,5},                                                        // 64 = WP_SEBNI_CH2    = "SEBNI"
    {7,0,5},                                                        // 65 = WP_ACHTI_CH2    = "ACHTI"
    {7,6,4},                                                        // 66 = WP_NUENI_CH2    = "NÜNI"
    {8,0,4},                                                        // 67 = WP_ZAENI_CH2    = "ZÄNI"
    {8,7,4},                                                        // 68 = WP_ELFI_CH2     = "ELFI"
    {9,0,6},                                                        // 69 = WP_ZWOELFI_CH2  = "ZWÖLFI"
    {9,8,3},                                                        // 70 = WP_UHR_CH2      = "UHR"

    {0,0,2},                                                        // 71 = WP_IT_EN        = "IT"
    {0,3,2},                                                        // 72 = WP_IS_EN        = "IS"
    {0,7,2},                                                        // 73 = WP_AM_EN        = "AM"
    {0,9,2},                                                        // 74 = WP_PM_EN        = "PM"
    {1,2,7},                                                        // 75 = WP_QUARTER_EN   = "QUARTER"
    {2,0,6},                                                        // 76 = WP_TWENTY_EN    = "TWENTY"
    {2,6,4},                                                        // 77 = WP_FIVE_1_EN    = "FIVE"
    {3,0,4},                                                        // 78 = WP_HALF_EN      = "HALF"
    {3,5,3},                                                        // 79 = WP_TEN_EN       = "TEN"
    {3,9,2},                                                        // 80 = WP_TO_EN        = "TO"
    {4,0,4},                                                        // 81 = WP_PAST_EN      = "PAST"
    {4,7,4},                                                        // 82 = WP_NINE_EN      = "NINE"
    {5,0,3},                                                        // 83 = WP_ONE_EN       = "ONE"
    {5,3,3},                                                        // 84 = WP_SIX_EN       = "SIX"
    {5,6,5},                                                        // 85 = WP_THREE_EN     = "THREE"
    {6,0,4},                                                        // 86 = WP_FOUR_EN      = "FOUR"
    {6,4,4},                                                        // 87 = WP_FIVE_2_EN    = "FIVE"
    {6,8,3},                                                        // 88 = WP_TWO_EN       = "TWO"
    {7,0,5},                                                        // 89 = WP_EIGHT_EN     = "EIGHT"
    {7,5,6},                                                        // 90 = WP_ELEVEN_EN    = "ELEVEN"
    {8,0,5},                                                        // 91 = WP_SEVEN_EN     = "SEVEN"
    {8,5,6},                                                        // 92 = WP_TWELVE_EN    = "TWELVE"
    {9,0,3},                                                        // 93 = WP_TEN_2_EN     = "TEN"
    {9,5,6},                                                        // 94 = WP_CLOCK_EN     = "OCLOCK"

    {0,0,2},                                                        //  95 = WP_IT_EN2       = "IT"
    {0,3,2},                                                        //  96 = WP_IS_EN2       = "IS"
    {1,4,7},                                                        //  97 = WP_QUARTER_EN2  = "QUARTER"
    {2,1,6},                                                        //  98 = WP_TWENTY_EN2   = "TWENTY"
    {2,7,4},                                                        //  99 = WP_FIVE_1_EN2   = "FIVE"
    {0,6,4},                                                        // 100 = WP_HALF_EN2     = "HALF"
    {1,0,3},                                                        // 101 = WP_TEN_EN2      = "TEN"
    {3,0,2},                                                        // 102 = WP_TO_EN2       = "TO"
    {3,2,4},                                                        // 103 = WP_PAST_EN2     = "PAST"
    {4,7,4},                                                        // 104 = WP_NINE_EN2     = "NINE"
    {6,7,3},                                                        // 105 = WP_ONE_EN2      = "ONE"
    {8,4,3},                                                        // 106 = WP_SIX_EN2      = "SIX"
    {5,0,5},                                                        // 107 = WP_THREE_EN2    = "THREE"
    {3,7,4},                                                        // 108 = WP_FOUR_EN2     = "FOUR"
    {4,0,4},                                                        // 109 = WP_FIVE_2_EN2   = "FIVE"
    {4,4,3},                                                        // 110 = WP_TWO_EN2      = "TWO"
    {7,6,5},                                                        // 111 = WP_EIGHT_EN2    = "EIGHT"
    {6,1,6},                                                        // 112 = WP_ELEVEN_EN2   = "ELEVEN"
    {7,0,5},                                                        // 113 = WP_SEVEN_EN2    = "SEVEN"
    {5,5,6},                                                        // 114 = WP_TWELVE_EN2   = "TWELVE"
    {8,1,3},                                                        // 115 = WP_TEN_2_EN2    = "TEN"
    {9,4,1},                                                        // 116 = WP_O_EN2        = "O"
    {9,6,5},                                                        // 117 = WP_CLOCK_EN2    = "CLOCK"
};

#if 0 // GERMAN (just for test)
const char * display[WC_ROWS] =
{
    "ESKISTLFÜNF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZWÖLFP",
    "ZWEINSIEBEN",
    "KDREIRHFÜNF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
};
#endif // 0

#if 0 // SWISS1
const char * display[WC_ROWS] =
{
    "ESKISCHAFÜF",
    "VIERTUBFZÄÄ",
    "ZWÄNZGSIVOR",
    "ABOHAUBIEPM",
    "EISZWÖISDRÜ",
    "VIERIFÜFIQT",
    "SÄCHSISIBNI",
    "ACHTINÜNIEL",
    "ZÄNIERBEUFI",
    "ZWÖUFIAMUHR"
};
#endif // 0

#if 0 // SWISS2
const char * display[WC_ROWS] =
{
    "ESKESCHAZÄÄ",
    "FÖIFCVIERTU",
    "ZWÄNZGSIVOR",
    "ABOHAUBIEGE",
    "EISZWÖISDRÜ",
    "VIERITFÖIFI",
    "SÄCHSISEBNI",
    "ACHTIENÜNIL",
    "ZÄNIERBELFI",
    "ZWÖLFINAUHR"
};
#endif // 0

#if 0 // ENGLISH1
const char * display[WC_ROWS] =
{
    "ITLISASAMPM",
    "ACQUARTERDC",
    "TWENTYFIVEX",
    "HALFSTENFTO",
    "PASTERUNINE",
    "ONESIXTHREE",
    "FOURFIVETWO",
    "EIGHTELEVEN",
    "SEVENTWELVE",
    "TENSEOCLOCK"
};
#endif // 0

#if 0 // ENGLISH2
const char * display[WC_ROWS] =
{
    "ITKISGHALFE",
    "TENYQUARTER",
    "DTWENTYFIVE",
    "TOPASTEFOUR",
    "FIVETWONINE",
    "THREETWELVE",
    "BELEVENONES",
    "SEVENWEIGHT",
    "ITENSIXTIES",
    "TINEOICLOCK"
};
#endif // 0

#endif
