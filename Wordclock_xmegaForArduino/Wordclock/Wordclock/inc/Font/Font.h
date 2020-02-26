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
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* FontCommon configuration parameter */


/* FontCommon parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  G L O B A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
enum class Orientation {
    ORIENTATION_HORIZONTAL,
    ORIENTATION_VERTICAL,
    ORIENTATION_NONE
};

/******************************************************************************************************************************************************
 *  C L A S S   F O N T
******************************************************************************************************************************************************/
template <typename FontCharType, size_t FontTableSize> class Font
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using FontTableElementType = FontCharType;
    using FontTableType = std::array<FontCharType, FontTableSize>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
	const FontTableType& FontTable;
  
    // functions
    FontTableElementType getFontTableElement(byte Index) const {
        FontTableElementType fontTableElement;
        memcpy_P(&fontTableElement, &FontTable[Index], sizeof(FontTableElementType));
        return fontTableElement;
    }
	
/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  protected:	


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Font(const FontTableType& sFontTable) : FontTable(sFontTable) {

    }
	
	~Font() {

	}

	// get methods
    //virtual Orientation getOrientation() const = 0;
    //virtual byte getWidth() const = 0;
    //virtual byte getHeight() const = 0;
    //virtual const FontTableType& getFontTable() const = 0;

    FontCharType getCharFast(byte Index) const { return getFontTableElement(Index); }
    byte getCharWidthFast(byte Index) const { return getFontTableElement(Index).getWidth(); }

    stdReturnType getChar(byte Index, FontCharType& FontChar) const {
        if(Index < FontTableSize) {
            FontChar = getFontTableElement(Index);
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }
    stdReturnType getCharWidth(byte Index, byte& Width) {
        if(Index < FontTableSize) {
            Width = getFontTableElement(Index).getWidth();
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }

	// set methods

	// methods

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
