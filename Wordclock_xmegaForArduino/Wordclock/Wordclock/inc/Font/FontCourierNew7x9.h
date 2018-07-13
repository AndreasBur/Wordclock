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
/**     \file       FontCourierNew7x9.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _FONT_COURIER_NEW_7X9_H_
#define _FONT_COURIER_NEW_7X9_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "FontCommon.h"
#include "FontChar.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* FontCourierNew7x9 configuration parameter */


/* FontCourierNew7x9 parameter */
#define FONT_COURIER_NEW_7X9_WIDTH                       7
#define FONT_COURIER_NEW_7X9_HEIGHT                      9
#define FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE             102


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   F O N T C O U R I E R N E W 7 X 9
******************************************************************************************************************************************************/
class FontCourierNew7x9 : public FontCommon<FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>, FONT_COURIER_NEW_7X9_FONT_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using FontCharType = FontCharHorizontal<byte, FONT_COURIER_NEW_7X9_HEIGHT>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    static const FontTableType FontTable;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    FontCourierNew7x9();
    ~FontCourierNew7x9();

	// get methods


	// set methods

	// methods
    void init();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
