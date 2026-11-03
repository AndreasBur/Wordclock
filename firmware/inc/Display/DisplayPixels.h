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
    PixelsBufferType pixels_buffer_;
    DisplayWords words_;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    DisplayPixels() : pixels_buffer_{0u} {}
    ~DisplayPixels() {}

	// get methods
    boolean GetPixel(byte column, byte row) const { return ReadBit(pixels_buffer_[row], column); }

	// set methods
	void SetPixel(byte column, byte row) { pixels_buffer_[row] = WriteBit(pixels_buffer_[row], column, true); }
	void SetPixel(IndexType index) {
	    byte column, row;
	    Display::IndexToColumnAndRow(index, column, row);
	    SetPixel(column, row);
    }

	// methods
	void ClearPixel(byte column, byte row) { pixels_buffer_[row] = WriteBit(pixels_buffer_[row], column, false); }
	void WritePixel(byte column, byte row, bool value) { pixels_buffer_[row] = WriteBit(pixels_buffer_[row], column, value); }

	void SetWord(WordIdType word_id) {
        DisplayWord word = words_.GetDisplayWord(word_id);
        for(IndexType index = 0u; index < word.GetLength(); index++) { SetPixel(word.GetColumn() + index,  word.GetRow()); }
    }

	void SetPixelsToDisplay() const
	{
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
                Display::GetInstance().WritePixel(column, row, GetPixel(column, row));
            }
        }
	}

	void GetPixelsFromDisplay()
	{
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
                pixels_buffer_[row] = WriteBit(pixels_buffer_[row], column, Display::GetInstance().GetPixel(column, row));
            }
        }
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
