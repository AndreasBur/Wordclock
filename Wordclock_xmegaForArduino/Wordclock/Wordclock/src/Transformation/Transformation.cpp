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
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftLeftFast(boolean Rotate)
{
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        shiftRowLeftFast(Row, Rotate);
    }
} /* shiftLeftFast */


/******************************************************************************************************************************************************
  shiftRowLeftFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftRowLeftFast(byte Row, boolean Rotate)
{
    Display::PixelType FirstPixel;

    if(Rotate) FirstPixel = Display::getInstance().getPixelFast(0, Row);

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column + 1, Row));
    }
    if(Rotate) Display::getInstance().writePixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, FirstPixel);
    else Display::getInstance().clearPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row);
} /* shiftRowLeftFast */


/******************************************************************************************************************************************************
  shiftLeft()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftLeft(boolean Rotate)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(shiftRowLeft(Row, Rotate) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    }
    return ReturnValue;
} /* shiftLeft */


/******************************************************************************************************************************************************
  shiftRowLeft()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftRowLeft(byte Row, boolean Rotate)
{
    Display::PixelType FirstPixel;
    Display::PixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(Display::getInstance().getPixel(0, Row, &FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
        if(Display::getInstance().getPixel(Column + 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftRowLeft */


/******************************************************************************************************************************************************
  shiftRightFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftRightFast(boolean Rotate)
{
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        shiftRowRightFast(Row, Rotate);
    }
} /* shiftRightFast */


/******************************************************************************************************************************************************
  shiftRowRightFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftRowRightFast(byte Row, boolean Rotate)
{
    Display::PixelType LastPixel;

    if(Rotate) LastPixel = Display::getInstance().getPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row);

    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column - 1, Row));
    }
    if(Rotate) Display::getInstance().writePixelFast(0, Row, LastPixel);
    else Display::getInstance().clearPixelFast(0, Row);
} /* shiftRowRightFast */


/******************************************************************************************************************************************************
  shiftRight()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftRight(boolean Rotate)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(shiftRowRight(Row, Rotate) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    }
    return ReturnValue;
} /* shiftRight */


/******************************************************************************************************************************************************
  shiftRowRight()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftRowRight(byte Row, boolean Rotate)
{
    Display::PixelType LastPixel;
    Display::PixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(Display::getInstance().getPixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, &LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
        if(Display::getInstance().getPixel(Column - 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(0, Row, LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(Display::getInstance().clearPixel(0, Row) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftRowRight */


/******************************************************************************************************************************************************
  shiftUpFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftUpFast(boolean Rotate)
{
    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        shiftColumnUpFast(Column, Rotate);
    }
} /* shiftUpFast */


/******************************************************************************************************************************************************
  shiftColumnUpFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftColumnUpFast(byte Column, boolean Rotate)
{
    Display::PixelType FirstPixel;

    if(Rotate) FirstPixel = Display::getInstance().getPixelFast(Column, 0);

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column, Row + 1));
    }
    if(Rotate) Display::getInstance().writePixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1, FirstPixel);
    else Display::getInstance().clearPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1);
} /* shiftColumnUpFast */


/******************************************************************************************************************************************************
  shiftUp()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftUp(boolean Rotate)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(shiftColumnUp(Column, Rotate) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    }
    return ReturnValue;
} /* shiftUp */


/******************************************************************************************************************************************************
  shiftColumnUp()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftColumnUp(byte Column, boolean Rotate)
{
    Display::PixelType FirstPixel;
    Display::PixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(Display::getInstance().getPixel(Column, 0, &FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
        if(Display::getInstance().getPixel(Column, Row + 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(Column, DISPLAY_NUMBER_OF_ROWS - 1, FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftColumnUp */


/******************************************************************************************************************************************************
  shiftDownFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftDownFast(boolean Rotate)
{
    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        shiftColumnDownFast(Column, Rotate);
    }
} /* shiftDownFast */


/******************************************************************************************************************************************************
  shiftColumnDownFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Transformation::shiftColumnDownFast(byte Column, boolean Rotate)
{
    Display::PixelType LastPixel;

    if(Rotate) LastPixel = Display::getInstance().getPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1);

    for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
        Display::getInstance().writePixelFast(Column, Row, Display::getInstance().getPixelFast(Column, Row - 1));
    }
    if(Rotate) Display::getInstance().writePixelFast(Column, 0, LastPixel);
    else Display::getInstance().clearPixelFast(Column, 0);
} /* shiftColumnDownFast */


/******************************************************************************************************************************************************
  shiftDown()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftDown(boolean Rotate)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(shiftColumnDown(Column, Rotate) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    }
    return ReturnValue;
} /* shiftDown */


/******************************************************************************************************************************************************
  shiftColumnDown()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Transformation::shiftColumnDown(byte Column, boolean Rotate)
{
    Display::PixelType LastPixel;
    Display::PixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(Display::getInstance().getPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1, &LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
        if(Display::getInstance().getPixel(Column, Row - 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(Display::getInstance().writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(Display::getInstance().writePixel(Column, 0, LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(Display::getInstance().clearPixel(Column, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftColumnDown */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 