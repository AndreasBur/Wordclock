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
/**     \file       FontLucidaSans9x10.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _FONT_LUCIDA_SANS_9X10_H_
#define _FONT_LUCIDA_SANS_9X10_H_

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
/* FontLucidaSans9x10 configuration parameter */


/* FontLucidaSans9x10 parameter */
#define FONT_LUCIDA_SANS_9X10_WIDTH                      9u
#define FONT_LUCIDA_SANS_9X10_HEIGHT                     10u
#define FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE            102u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   F O N T L U C I D A S A N S 9 X 1 0
******************************************************************************************************************************************************/
class FontLucidaSans9x10 : public Font<FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>, FONT_LUCIDA_SANS_9X10_FONT_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using FontCharType = FontCharVertical<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;
    using ColumnsType = std::array<uint16_t, FONT_LUCIDA_SANS_9X10_WIDTH>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const FontTableType FontTable;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr FontLucidaSans9x10() : Font(FontTable) { }
    ~FontLucidaSans9x10() { }

    // get methods
    byte getWidth() const { return FONT_LUCIDA_SANS_9X10_WIDTH; }
    byte getHeight() const { return FONT_LUCIDA_SANS_9X10_HEIGHT; }
    Orientation getOrientation() const { return Orientation::ORIENTATION_VERTICAL; }
    const FontTableType& getFontTable() const { return FontTable; }
                
    // set methods

    // methods

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
