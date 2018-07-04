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
/**     \file       Font.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _FONT_H_
#define _FONT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Font configuration parameter */
#define FONT_SUPPORT_FONT_5X8                   STD_ON
#define FONT_SUPPORT_FONT_7X9                   STD_ON
#define FONT_SUPPORT_FONT_7X10                  STD_ON
#define FONT_SUPPORT_FONT_9X10                  STD_ON
#define FONT_SUPPORT_FONT_10X10                 STD_ON

/* Font parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class Font
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum FontType {
#if(FONT_SUPPORT_FONT_5X8 == STD_ON)
        FONT_5X8,
#endif
#if(FONT_SUPPORT_FONT_7X9 == STD_ON)
        FONT_7X9,
#endif
#if(FONT_SUPPORT_FONT_7X10 == STD_ON)
        FONT_7X10,
#endif
#if(FONT_SUPPORT_FONT_9X10 == STD_ON)
        FONT_9X10,
#endif
#if(FONT_SUPPORT_FONT_10X10 == STD_ON)
        FONT_10X10,
#endif
        FONT_NONE
    };
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:

  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Font();
    ~Font();

	// get methods


	// set methods

	// methods
    void init();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/