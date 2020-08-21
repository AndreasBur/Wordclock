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
  shiftLeftFast()
******************************************************************************************************************************************************/
void Transformation::shiftLeftFast(bool Rotate)
{
    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        shiftRowLeftFast(row, Rotate);
    }
} /* shiftLeftFast */


/******************************************************************************************************************************************************
  shiftRowLeftFast()
******************************************************************************************************************************************************/
void Transformation::shiftRowLeftFast(byte Row, bool Rotate)
{
    Display::PixelType firstPixel{0u};

    if(Rotate) firstPixel = Display::getInstance().getPixelFast(0u, Row);

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS - 1u; column++) {
        Display::getInstance().writePixelFast(column, Row, Display::getInstance().getPixelFast(column + 1u, Row));
    }
    if(Rotate) Display::getInstance().writePixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row, firstPixel);
    else Display::getInstance().clearPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row);
} /* shiftRowLeftFast */


/******************************************************************************************************************************************************
  shiftLeft()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftLeft(bool Rotate)
{
    StdReturnType returnValue{E_OK};

    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(shiftRowLeft(row, Rotate) == E_NOT_OK) { returnValue = E_NOT_OK; }
    }
    return returnValue;
} /* shiftLeft */


/******************************************************************************************************************************************************
  shiftRowLeft()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftRowLeft(byte Row, bool Rotate)
{
    Display::PixelType firstPixel;
    Display::PixelType pixel;
    StdReturnType returnValue{E_OK};

    if(Rotate) if(Display::getInstance().getPixel(0, Row, firstPixel) == E_NOT_OK) returnValue = E_NOT_OK;

    for(byte Column = 0u; Column < DISPLAY_NUMBER_OF_COLUMNS - 1u; Column++) {
        if(Display::getInstance().getPixel(Column + 1u, Row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row, firstPixel) == E_NOT_OK) returnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row) == E_NOT_OK) returnValue = E_NOT_OK; }

    return returnValue;
} /* shiftRowLeft */


/******************************************************************************************************************************************************
  shiftRightFast()
******************************************************************************************************************************************************/
void Transformation::shiftRightFast(bool Rotate)
{
    for(byte row = 0u; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        shiftRowRightFast(row, Rotate);
    }
} /* shiftRightFast */


/******************************************************************************************************************************************************
  shiftRowRightFast()
******************************************************************************************************************************************************/
void Transformation::shiftRowRightFast(byte Row, bool Rotate)
{
    Display::PixelType lastPixel{0u};

    if(Rotate) lastPixel = Display::getInstance().getPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row);

    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1u; Column > 0; Column--) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column - 1u, Row));
    }
    if(Rotate) Display::getInstance().writePixelFast(0u, Row, lastPixel);
    else Display::getInstance().clearPixelFast(0u, Row);
} /* shiftRowRightFast */


/******************************************************************************************************************************************************
  shiftRight()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftRight(bool Rotate)
{
    StdReturnType returnValue = E_OK;

    for(byte Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(shiftRowRight(Row, Rotate) == E_NOT_OK) { returnValue = E_NOT_OK; }
    }
    return returnValue;
} /* shiftRight */


/******************************************************************************************************************************************************
  shiftRowRight()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftRowRight(byte Row, bool Rotate)
{
    Display::PixelType lastPixel;
    Display::PixelType pixel;
    StdReturnType returnValue{E_OK};

    if(Rotate) if(Display::getInstance().getPixel(DISPLAY_NUMBER_OF_COLUMNS - 1u, Row, lastPixel) == E_NOT_OK) returnValue = E_NOT_OK;

    for(int8_t column = DISPLAY_NUMBER_OF_COLUMNS - 1u; column > 0; column--) {
        if(Display::getInstance().getPixel(column - 1u, Row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(column, Row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(0u, Row, lastPixel) == E_NOT_OK) returnValue = E_NOT_OK; }
    else{ if(Display::getInstance().clearPixel(0u, Row) == E_NOT_OK) returnValue = E_NOT_OK; }

    return returnValue;
} /* shiftRowRight */


/******************************************************************************************************************************************************
  shiftUpFast()
******************************************************************************************************************************************************/
void Transformation::shiftUpFast(bool Rotate)
{
    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        shiftColumnUpFast(column, Rotate);
    }
} /* shiftUpFast */


/******************************************************************************************************************************************************
  shiftColumnUpFast()
******************************************************************************************************************************************************/
void Transformation::shiftColumnUpFast(byte Column, bool Rotate)
{
    Display::PixelType firstPixel{0u};

    if(Rotate) firstPixel = Display::getInstance().getPixelFast(Column, 0u);

    for(byte Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS - 1u; Row++) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column, Row + 1u));
    }
    if(Rotate) Display::getInstance().writePixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1u, firstPixel);
    else Display::getInstance().clearPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1u);
} /* shiftColumnUpFast */


/******************************************************************************************************************************************************
  shiftUp()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftUp(bool Rotate)
{
    StdReturnType returnValue{E_OK};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(shiftColumnUp(column, Rotate) == E_NOT_OK) { returnValue = E_NOT_OK; }
    }
    return returnValue;
} /* shiftUp */


/******************************************************************************************************************************************************
  shiftColumnUp()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftColumnUp(byte Column, bool Rotate)
{
    Display::PixelType firstPixel;
    Display::PixelType pixel;
    StdReturnType returnValue{E_OK};

    if(Rotate) if(Display::getInstance().getPixel(Column, 0u, firstPixel) == E_NOT_OK) returnValue = E_NOT_OK;

    for(byte Row = 0u; Row < DISPLAY_NUMBER_OF_ROWS - 1u; Row++) {
        if(Display::getInstance().getPixel(Column, Row + 1u, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(Column, DISPLAY_NUMBER_OF_ROWS - 1u, firstPixel) == E_NOT_OK) returnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1u) == E_NOT_OK) returnValue = E_NOT_OK; }

    return returnValue;
} /* shiftColumnUp */


/******************************************************************************************************************************************************
  shiftDownFast()
******************************************************************************************************************************************************/
void Transformation::shiftDownFast(bool Rotate)
{
    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        shiftColumnDownFast(column, Rotate);
    }
} /* shiftDownFast */


/******************************************************************************************************************************************************
  shiftColumnDownFast()
******************************************************************************************************************************************************/
void Transformation::shiftColumnDownFast(byte Column, bool Rotate)
{
    Display::PixelType lastPixel{0u};

    if(Rotate) lastPixel = Display::getInstance().getPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1u);

    for(int8_t row = DISPLAY_NUMBER_OF_ROWS - 1u; row > 0; row--) {
        Display::getInstance().writePixelFast(Column, row, Display::getInstance().getPixelFast(Column, row - 1u));
    }
    if(Rotate) Display::getInstance().writePixelFast(Column, 0u, lastPixel);
    else Display::getInstance().clearPixelFast(Column, 0u);
} /* shiftColumnDownFast */


/******************************************************************************************************************************************************
  shiftDown()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftDown(bool Rotate)
{
    StdReturnType returnValue{E_OK};

    for(byte column = 0u; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(shiftColumnDown(column, Rotate) == E_NOT_OK) { returnValue = E_NOT_OK; }
    }
    return returnValue;
} /* shiftDown */


/******************************************************************************************************************************************************
  shiftColumnDown()
******************************************************************************************************************************************************/
StdReturnType Transformation::shiftColumnDown(byte Column, bool Rotate)
{
    Display::PixelType lastPixel;
    Display::PixelType pixel;
    StdReturnType returnValue{E_OK};

    if(Rotate) if(Display::getInstance().getPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1u, lastPixel) == E_NOT_OK) returnValue = E_NOT_OK;

    for(int8_t row = DISPLAY_NUMBER_OF_ROWS - 1u; row > 0; row--) {
        if(Display::getInstance().getPixel(Column, row - 1u, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, row, pixel) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(Column, 0u, lastPixel) == E_NOT_OK) returnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(Column, 0u) == E_NOT_OK) returnValue = E_NOT_OK; }

    return returnValue;
} /* shiftColumnDown */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 