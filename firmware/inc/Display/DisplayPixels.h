// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A buffer of the pixels of the grid, to compose in and then write to the display.
 */

#ifndef _DISPLAY_PIXELS_H_
#define _DISPLAY_PIXELS_H_

#include "StandardTypes.h"
#include "Bit.h"
#include "Arduino.h"
#include "DisplayCharacters.h"
#include "Display.h"

/* DisplayPixels configuration parameter */

/* DisplayPixels parameter */

class DisplayPixels
{
  public:
    using PixelRowType = Display::PixelRowType;
    using PixelColumnType = Display::PixelColumnType;
    using WordIdType = DisplayWords::WordIdType;
    using IndexType = Display::IndexType;
    using PixelsBufferType = std::array<Bit<PixelColumnType>, DISPLAY_NUMBER_OF_ROWS>;

  private:
    PixelsBufferType pixels_buffer_;
    DisplayWords words_;

  public:
    DisplayPixels() : pixels_buffer_{} {}
    ~DisplayPixels() {}

	// get methods
    boolean GetPixel(byte column, byte row) const { return pixels_buffer_[row][column]; }

	// set methods
	void SetPixel(byte column, byte row) { pixels_buffer_[row][column] = true; }
	void SetPixel(IndexType index) {
	    byte column, row;
	    Display::IndexToColumnAndRow(index, column, row);
	    SetPixel(column, row);
    }

	// methods
	void ClearPixel(byte column, byte row) { pixels_buffer_[row][column] = false; }
	void WritePixel(byte column, byte row, bool value) { pixels_buffer_[row][column] = value; }

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
                pixels_buffer_[row][column] = Display::GetInstance().GetPixel(column, row);
            }
        }
	}
};

#endif
