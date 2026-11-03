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
/**     \file       Transformation.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _TRANSFORMATION_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "Transformation.h"
#include "Display.h"


/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
  shiftLeft()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftLeft(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(ShiftRowLeft(row, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
} /* shiftLeft */


/******************************************************************************************************************************************************
  shiftRowLeft()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftRowLeft(byte row, bool rotate)
{
    Display::PixelValueType first_pixel_value;
    Display::PixelValueType pixel_value;
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(0, row, first_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS - 1u; column++) {
        if(Display::GetInstance().GetPixel(column + 1u, row, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row, first_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row) == kENotOk) return_value = kENotOk; }

    return return_value;
} /* shiftRowLeft */


/******************************************************************************************************************************************************
  shiftRight()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftRight(bool rotate)
{
    StdReturnType return_value = kEOk;

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(ShiftRowRight(row, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
} /* shiftRight */


/******************************************************************************************************************************************************
  shiftRowRight()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftRowRight(byte row, bool rotate)
{
    Display::PixelValueType last_pixel_value;
    Display::PixelValueType pixel_value;
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, row, last_pixel_value) == kENotOk) return_value = kENotOk;

    for(int8_t column = DISPLAY_NUMBER_OF_COLUMNS - 1u; column > 0; column--) {
        if(Display::GetInstance().GetPixel(column - 1u, row, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(0u, row, last_pixel_value) == kENotOk) return_value = kENotOk; }
    else{ if(Display::GetInstance().ClearPixel(0u, row) == kENotOk) return_value = kENotOk; }

    return return_value;
} /* shiftRowRight */


/******************************************************************************************************************************************************
  shiftUp()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftUp(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(ShiftColumnUp(column, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
} /* shiftUp */


/******************************************************************************************************************************************************
  shiftColumnUp()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftColumnUp(byte column, bool rotate)
{
    Display::PixelValueType first_pixel_value;
    Display::PixelValueType pixel_value;
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(column, 0u, first_pixel_value) == kENotOk) return_value = kENotOk;

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS - 1u; row++) {
        if(Display::GetInstance().GetPixel(column, row + 1u, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(column, DISPLAY_NUMBER_OF_ROWS - 1u, first_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(column, DISPLAY_NUMBER_OF_ROWS - 1u) == kENotOk) return_value = kENotOk; }

    return return_value;
} /* shiftColumnUp */


/******************************************************************************************************************************************************
  shiftDown()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftDown(bool rotate)
{
    StdReturnType return_value{kEOk};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(ShiftColumnDown(column, rotate) == kENotOk) { return_value = kENotOk; }
    }
    return return_value;
} /* shiftDown */


/******************************************************************************************************************************************************
  shiftColumnDown()
******************************************************************************************************************************************************/
StdReturnType Transformation::ShiftColumnDown(byte column, bool rotate)
{
    Display::PixelValueType last_pixel_value;
    Display::PixelValueType pixel_value;
    StdReturnType return_value{kEOk};

    if(rotate) if(Display::GetInstance().GetPixel(column, DISPLAY_NUMBER_OF_ROWS - 1u, last_pixel_value) == kENotOk) return_value = kENotOk;

    for(int8_t row = DISPLAY_NUMBER_OF_ROWS - 1u; row > 0; row--) {
        if(Display::GetInstance().GetPixel(column, row - 1u, pixel_value) == kENotOk) return_value = kENotOk;
        if(Display::GetInstance().WritePixel(column, row, pixel_value) == kENotOk) return_value = kENotOk;
    }
    if(rotate) { if(Display::GetInstance().WritePixel(column, 0u, last_pixel_value) == kENotOk) return_value = kENotOk; }
    else { if(Display::GetInstance().ClearPixel(column, 0u) == kENotOk) return_value = kENotOk; }

    return return_value;
} /* shiftColumnDown */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
