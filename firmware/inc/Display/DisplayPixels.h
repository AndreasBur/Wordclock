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
/**     \file       DisplayPixels.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _DISPLAY_PIXELS_H_
#define _DISPLAY_PIXELS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayCharacters.h"
#include "Display.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DisplayPixels configuration parameter */



/* DisplayPixels parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y   P I X E L S
******************************************************************************************************************************************************/
class DisplayPixels
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using PixelRowType = Display::PixelRowType;
    using PixelColumnType = Display::PixelColumnType;
    using WordIdType = DisplayWords::WordIdType;
    using IndexType = Display::IndexType;
    using PixelsBufferType = std::array<PixelColumnType, DISPLAY_NUMBER_OF_ROWS>;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    PixelsBufferType PixelsBuffer;
    DisplayWords Words;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    DisplayPixels() : PixelsBuffer{0u} {}
    ~DisplayPixels() {}

	// get methods
    boolean getPixel(byte Column, byte Row) const { return readBit(PixelsBuffer[Row], Column); }

	// set methods
	void setPixel(byte Column, byte Row) { PixelsBuffer[Row] = writeBit(PixelsBuffer[Row], Column, true); }
	void setPixel(IndexType Index) {
	    byte Column, Row;
	    Display::indexToColumnAndRow(Index, Column, Row);
	    setPixel(Column, Row);
    }

	// methods
	void clearPixel(byte Column, byte Row) { PixelsBuffer[Row] = writeBit(PixelsBuffer[Row], Column, false); }
	void writePixel(byte Column, byte Row, bool Value) { PixelsBuffer[Row] = writeBit(PixelsBuffer[Row], Column, Value); }

	void setWord(WordIdType WordId) {
        DisplayWord word = Words.getDisplayWordFast(WordId);
        for(IndexType Index = 0u; Index < word.getLength(); Index++) { setPixel(word.getColumn() + Index,  word.getRow()); }
    }

	void setPixelsToDisplay() const
	{
        for(byte Column = 0u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            for(byte Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
                Display::getInstance().writePixel(Column, Row, getPixel(Column, Row));
            }
        }
	}

	void getPixelsFromDisplay()
	{
        for(byte Column = 0u; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            for(byte Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
                PixelsBuffer[Row] = writeBit(PixelsBuffer[Row], Column, Display::getInstance().getPixelFast(Column, Row));
            }
        }
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
