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
/**     \file       FontCourierNew7x10.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _FONT_COURIER_NEW_7X10_H_
#define _FONT_COURIER_NEW_7X10_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Font.h"
#include "FontChar.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* FontCourierNew7x10 configuration parameter */


/* FontCourierNew7x10 parameter */
#define FONT_COURIER_NEW_7X10_WIDTH                     7u
#define FONT_COURIER_NEW_7X10_HEIGHT                    10u
#define FONT_COURIER_NEW_7X10_FONT_TABLE_SIZE           102u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   F O N T C O U R I E R N E W 7 X 1 0
******************************************************************************************************************************************************/
class FontCourierNew7x10 : public Font<FontCharHorizontal<byte, FONT_COURIER_NEW_7X10_HEIGHT>, FONT_COURIER_NEW_7X10_FONT_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using FontCharType = FontCharHorizontal<byte, FONT_COURIER_NEW_7X10_HEIGHT>;
    using RowsType = std::array<byte, FONT_COURIER_NEW_7X10_HEIGHT>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    static const FontTableType FontTable;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr FontCourierNew7x10() : Font(FontTable) {
        
    }
    
    ~FontCourierNew7x10() {
        
    }

    // get methods
    byte getWidth() const { return FONT_COURIER_NEW_7X10_WIDTH; }
    byte getHeight() const { return FONT_COURIER_NEW_7X10_HEIGHT; }
    Orientation getOrientation() const { return Orientation::ORIENTATION_HORIZONTAL; }
    const FontTableType& getFontTable() const { return FontTable; }
        
    // set methods

    // methods

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
