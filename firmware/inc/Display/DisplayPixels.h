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

/*!
 * \brief A buffer of the pixels of the grid, lit or not, to compose a picture in before it is
 *        written to the display.
 *
 * It holds one bit per pixel rather than a colour: SetPixelsToDisplay() lights the pixels in the
 * display's colour. Columns and rows count from 0, as on the display, and a new buffer has every
 * pixel unlit.
 *
 * \warning No function checks its column, row or index. A row off the grid reads or writes past the
 *          end of the buffer. A column off the grid but inside the 16 bits of a row reaches bits
 *          the display does not show, and one beyond them is undefined behaviour.
 */
class DisplayPixels
{
  public:
    /*! \brief The pixels of one row as bits, as Display has it. */
    using PixelRowType = Display::PixelRowType;

    /*! \brief The pixels of one column as bits, as Display has it. */
    using PixelColumnType = Display::PixelColumnType;

    /*! \brief The id of a word, as DisplayWords has it. */
    using WordIdType = DisplayWords::WordIdType;

    /*! \brief The index of a pixel, as Display has it. */
    using IndexType = Display::IndexType;

    /*! \brief The buffer: one set of bits per row, one bit per column. */
    using PixelsBufferType = std::array<Bit<PixelColumnType>, DISPLAY_NUMBER_OF_ROWS>;

  private:
    PixelsBufferType pixels_buffer_;
    DisplayWords words_;

  public:
    /*! \brief Makes a buffer with every pixel unlit. */
    DisplayPixels() : pixels_buffer_{} {}

    /*! \brief Destroys the buffer. */
    ~DisplayPixels() {}

    /*!
     * \brief Returns whether the pixel at `column` and `row` is lit.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return True if the pixel is lit.
     */
    boolean GetPixel(byte column, byte row) const { return pixels_buffer_[row][column]; }

    /*!
     * \brief Lights the pixel at `column` and `row`.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     */
	void SetPixel(byte column, byte row) { pixels_buffer_[row][column] = true; }

    /*!
     * \brief Lights the pixel at `index`.
     *
     * \param[in] index The index of the pixel, counted from 0 row by row, as
     *                  Display::IndexToColumnAndRow() maps it.
     */
	void SetPixel(IndexType index) {
	    byte column, row;
	    Display::IndexToColumnAndRow(index, column, row);
	    SetPixel(column, row);
    }

    /*!
     * \brief Clears the pixel at `column` and `row`.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     */
	void ClearPixel(byte column, byte row) { pixels_buffer_[row][column] = false; }

    /*!
     * \brief Lights or clears the pixel at `column` and `row`.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \param[in] value True to light the pixel, false to clear it.
     */
	void WritePixel(byte column, byte row, bool value) { pixels_buffer_[row][column] = value; }

    /*!
     * \brief Lights the letters of the word `word_id`.
     *
     * Every other pixel stays as it was. An id that is not a word lights nothing.
     *
     * \param[in] word_id The id of the word.
     */
	void SetWord(WordIdType word_id) {
        DisplayWord word = words_.GetDisplayWord(word_id);
        for(IndexType index = 0u; index < word.GetLength(); index++) { SetPixel(word.GetColumn() + index,  word.GetRow()); }
    }

    /*!
     * \brief Writes every pixel of the buffer to the display: a lit one in the display's colour, an
     *        unlit one cleared.
     *
     * It writes through Display::WritePixel() and does not call Display::Show().
     */
	void SetPixelsToDisplay() const
	{
        for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
            for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
                Display::GetInstance().WritePixel(column, row, GetPixel(column, row));
            }
        }
	}

    /*!
     * \brief Replaces the buffer with the pixels of the display, each lit where the display's is.
     *
     * It reads through Display::GetPixel(), which counts a pixel as lit where any of its channels
     * is not 0.
     */
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
