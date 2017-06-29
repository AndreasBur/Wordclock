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
/**     \file       Template.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
 *****************************************************************************************************************************************************/
#define _TEMPLATE_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
 *****************************************************************************************************************************************************/
#include "Display_Clock.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
const DisplayWordWordsType DisplayClock::ClockHoursTable[][DISPLAY_CLOCK_NUMBER_OF_HOURS][DISPLAY_CLOCK_MAX_NUMBER_OF_HOUR_WORDS] PROGMEM =
{
    {                                                               // ClockHoursTable[0][] = hh:00
        {DISPLAY_WORD_ZWOELF,  DISPLAY_WORD_UHR},                   // 00:00
        {DISPLAY_WORD_EIN,     DISPLAY_WORD_UHR},                   // 01:00
        {DISPLAY_WORD_ZWEI,    DISPLAY_WORD_UHR},                   // 02:00
        {DISPLAY_WORD_DREI_2,  DISPLAY_WORD_UHR},                   // 03:00
        {DISPLAY_WORD_VIER_2,  DISPLAY_WORD_UHR},                   // 04:00
        {DISPLAY_WORD_FUENF_2, DISPLAY_WORD_UHR},                   // 05:00
        {DISPLAY_WORD_SECHS,   DISPLAY_WORD_UHR},                   // 06:00
        {DISPLAY_WORD_SIEBEN,  DISPLAY_WORD_UHR},                   // 07:00
        {DISPLAY_WORD_ACHT,    DISPLAY_WORD_UHR},                   // 08:00
        {DISPLAY_WORD_NEUN,    DISPLAY_WORD_UHR},                   // 09:00
        {DISPLAY_WORD_ZEHN_2,  DISPLAY_WORD_UHR},                   // 10:00
        {DISPLAY_WORD_ELF,     DISPLAY_WORD_UHR}                    // 11:00
    },
    {                                                               // ClockHoursTable[1][] = hh:mm
        {DISPLAY_WORD_ZWOELF  },                                    // 00:mm
        {DISPLAY_WORD_EINS    },                                    // 01:mm
        {DISPLAY_WORD_ZWEI    },                                    // 02:mm
        {DISPLAY_WORD_DREI_2  },                                    // 03:mm
        {DISPLAY_WORD_VIER_2  },                                    // 04:mm
        {DISPLAY_WORD_FUENF_2 },                                    // 05:mm
        {DISPLAY_WORD_SECHS   },                                    // 06:mm
        {DISPLAY_WORD_SIEBEN  },                                    // 07:mm
        {DISPLAY_WORD_ACHT    },                                    // 08:mm
        {DISPLAY_WORD_NEUN    },                                    // 09:mm
        {DISPLAY_WORD_ZEHN_2  },                                    // 10:mm
        {DISPLAY_WORD_ELF     }                                     // 11:mm
    }
};


const DisplayClockMinutesType DisplayClock::ClockMinutesTable[][DISPLAY_CLOCK_NUMBER_OF_MINUTE_STEPS] PROGMEM =
{
    {                                                                                       // ClockMinutesTable[0][] = WESSI
        {0},                                                                                // 00
        {1, 0, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH                    }},            // 05
        {1, 0, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH                    }},            // 10
        {1, 0, {DISPLAY_WORD_VIERTEL,   DISPLAY_WORD_NACH                    }},            // 15
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 20
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 25
        {1, 1, {DISPLAY_WORD_HALB                                            }},            // 30
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 35
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 40
        {1, 1, {DISPLAY_WORD_VIERTEL,   DISPLAY_WORD_VOR                     }},            // 45
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR                     }},            // 50
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR                     }},            // 55
    },

    {                                                                                       // ClockMinutesTable[1][] = OSSI
        {0},                                                                                // 00
        {1, 0, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH                    }},            // 05
        {1, 0, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH                    }},            // 10
        {1, 1, {DISPLAY_WORD_VIERTEL,                                        }},            // 45
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 20
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 25
        {1, 1, {DISPLAY_WORD_HALB                                            }},            // 30
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 35
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 40
        {1, 1, {DISPLAY_WORD_DREIVIERTEL                                     }},            // 15
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR                     }},            // 50
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR                     }},            // 55
    },

    {                                                                                       // ClockMinutesTable[2][] = RHEIN-RUHR
        {0},                                                                                // 00
        {1, 0, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH                    }},            // 05
        {1, 0, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH                    }},            // 10
        {1, 0, {DISPLAY_WORD_VIERTEL,   DISPLAY_WORD_NACH                    }},            // 15
        {1, 0, {DISPLAY_WORD_ZWANZIG,   DISPLAY_WORD_NACH                    }},            // 20
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 25
        {1, 1, {DISPLAY_WORD_HALB                                            }},            // 30
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 35
        {1, 1, {DISPLAY_WORD_ZWANZIG,   DISPLAY_WORD_VOR                     }},            // 40
        {1, 1, {DISPLAY_WORD_VIERTEL,   DISPLAY_WORD_VOR                     }},            // 45
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR                     }},            // 50
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR                     }},            // 55
    },

    {                                                                                       // ClockMinutesTable[3][] = SCHWABEN
        {0},                                                                                // 00
        {1, 0, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH                    }},            // 05
        {1, 0, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_NACH                    }},            // 10
        {1, 1, {DISPLAY_WORD_VIERTEL,                                        }},            // 45
        {1, 0, {DISPLAY_WORD_ZWANZIG,   DISPLAY_WORD_NACH                    }},            // 20
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR,  DISPLAY_WORD_HALB }},            // 25
        {1, 1, {DISPLAY_WORD_HALB                                            }},            // 30
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_NACH, DISPLAY_WORD_HALB }},            // 35
        {1, 1, {DISPLAY_WORD_ZWANZIG,   DISPLAY_WORD_VOR                     }},            // 40
        {1, 1, {DISPLAY_WORD_DREIVIERTEL                                     }},            // 15
        {1, 1, {DISPLAY_WORD_ZEHN_1,    DISPLAY_WORD_VOR                     }},            // 50
        {1, 1, {DISPLAY_WORD_FUENF_1,   DISPLAY_WORD_VOR                     }},// 55
    }
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
//void init()
//{

//} /* init */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
 *****************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
 