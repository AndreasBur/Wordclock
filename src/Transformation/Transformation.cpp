// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The shifts of the display, of one row and of one column.
 */

#include "Transformation.h"
#include "Display.h"

StdReturnType Transformation::ShiftLeft(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(ShiftRowLeft(row, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
}

StdReturnType Transformation::ShiftRowLeft(byte row, bool rotate)
{
    Display::PixelValueType first_pixel_value{};
    Display::PixelValueType pixel_value{};
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(0, row, first_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS - 1u; column++) {
        if(Display::GetInstance().GetPixel(column + 1u, row, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row, first_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row) == kENotOk) return_value = kENotOk; }

    return return_value;
}

StdReturnType Transformation::ShiftRight(bool rotate)
{
    StdReturnType return_value = kEOk;

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(ShiftRowRight(row, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
}

StdReturnType Transformation::ShiftRowRight(byte row, bool rotate)
{
    Display::PixelValueType last_pixel_value{};
    Display::PixelValueType pixel_value{};
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row, last_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte column = DISPLAY_NUMBER_OF_COLUMNS - 1u; column > 0u; column--) {
        if(Display::GetInstance().GetPixel(static_cast<byte>(column - 1u), row, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(0u, row, last_pixel_value) == kENotOk) return_value = kENotOk; }
    else{ if(Display::GetInstance().ClearPixel(0u, row) == kENotOk) return_value = kENotOk; }

    return return_value;
}

StdReturnType Transformation::ShiftUp(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(ShiftColumnUp(column, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
}

StdReturnType Transformation::ShiftColumnUp(byte column, bool rotate)
{
    Display::PixelValueType first_pixel_value{};
    Display::PixelValueType pixel_value{};
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(column, 0u, first_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS - 1u; row++) {
        if(Display::GetInstance().GetPixel(column, row + 1u, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(column, DISPLAY_NUMBER_OF_ROWS - 1u, first_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(column, DISPLAY_NUMBER_OF_ROWS - 1u) == kENotOk) return_value = kENotOk; }

    return return_value;
}

StdReturnType Transformation::ShiftDown(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(ShiftColumnDown(column, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
}

StdReturnType Transformation::ShiftColumnDown(byte column, bool rotate)
{
    Display::PixelValueType last_pixel_value{};
    Display::PixelValueType pixel_value{};
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(column, DISPLAY_NUMBER_OF_ROWS - 1u, last_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte row = DISPLAY_NUMBER_OF_ROWS - 1u; row > 0u; row--) {
        if(Display::GetInstance().GetPixel(column, static_cast<byte>(row - 1u), pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(column, 0u, last_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(column, 0u) == kENotOk) return_value = kENotOk; }

    return return_value;
}
