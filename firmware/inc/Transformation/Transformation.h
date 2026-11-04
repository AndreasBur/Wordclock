// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Shifts of the whole display or of one row or column, optionally rotating.
 */

#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief Shifts of the pixels of the display: of the whole display, or of one row or column.
 *
 * Each shift moves the pixels by one step and writes them through Display. A pixel moves as lit
 * or dark: a lit one is written in the colour the display writes now, not in the colour it had.
 */
class Transformation
{
  public:

  private:

  public:
    /*! \brief Makes the transformation, which holds no state. */
    constexpr Transformation() { }

    /*! \brief Destroys the transformation. */
    ~Transformation() { }

    /*!
     * \brief Shifts the whole display one column to the left.
     *
     * \param[in] rotate True to bring the leftmost column in again at the right; false turns the
     *                   new column at the right off.
     * \return kEOk, or kENotOk if the display refuses to read or write a pixel; the other rows are
     *         shifted all the same.
     */
    static StdReturnType ShiftLeft(bool rotate = false);

    /*!
     * \brief Shifts the whole display one column to the right.
     *
     * \param[in] rotate True to bring the rightmost column in again at the left; false turns the
     *                   new column at the left off.
     * \return kEOk, or kENotOk if the display refuses to read or write a pixel; the other rows are
     *         shifted all the same.
     */
    static StdReturnType ShiftRight(bool rotate = false);

    /*!
     * \brief Shifts the whole display one row up.
     *
     * \param[in] rotate True to bring the top row in again at the bottom; false turns the new row
     *                   at the bottom off.
     * \return kEOk, or kENotOk if the display refuses to read or write a pixel; the other columns
     *         are shifted all the same.
     */
    static StdReturnType ShiftUp(bool rotate = false);

    /*!
     * \brief Shifts the whole display one row down.
     *
     * \param[in] rotate True to bring the bottom row in again at the top; false turns the new row
     *                   at the top off.
     * \return kEOk, or kENotOk if the display refuses to read or write a pixel; the other columns
     *         are shifted all the same.
     */
    static StdReturnType ShiftDown(bool rotate = false);

    /*!
     * \brief Shifts one row one column to the left.
     *
     * \param[in] row The row, counted from 0.
     * \param[in] rotate True to bring the leftmost pixel in again at the right; false turns the new
     *                   pixel at the right off.
     * \return kEOk, or kENotOk if `row` lies off the display; the pixels are then left as they
     *         were.
     */
    static StdReturnType ShiftRowLeft(byte row, bool rotate = false);

    /*!
     * \brief Shifts one row one column to the right.
     *
     * \param[in] row The row, counted from 0.
     * \param[in] rotate True to bring the rightmost pixel in again at the left; false turns the new
     *                   pixel at the left off.
     * \return kEOk, or kENotOk if `row` lies off the display; the pixels are then left as they
     *         were.
     */
    static StdReturnType ShiftRowRight(byte row, bool rotate = false);

    /*!
     * \brief Shifts one column one row up.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] rotate True to bring the top pixel in again at the bottom; false turns the new
     *                   pixel at the bottom off.
     * \return kEOk, or kENotOk if `column` lies off the display; the pixels are then left as they
     *         were.
     */
    static StdReturnType ShiftColumnUp(byte column, bool rotate = false);

    /*!
     * \brief Shifts one column one row down.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] rotate True to bring the bottom pixel in again at the top; false turns the new
     *                   pixel at the top off.
     * \return kEOk, or kENotOk if `column` lies off the display; the pixels are then left as they
     *         were.
     */
    static StdReturnType ShiftColumnDown(byte column, bool rotate = false);
};

#endif
