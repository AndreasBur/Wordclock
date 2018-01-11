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
  Constructor of Transformation
******************************************************************************************************************************************************/
/*! \brief          Transformation Constructor
 *  \details        Instantiation of the Template library
 *
 *  \return         -
******************************************************************************************************************************************************/
Transformation::Transformation(Display* Display)
{
    pDisplay = Display;
} /* Transformation */


/******************************************************************************************************************************************************
  Destructor of Transformation
******************************************************************************************************************************************************/
Transformation::~Transformation()
{

} /* ~Transformation */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
//void Transformation::init()
//{

//} /* init */


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

    if(Rotate) FirstPixel = pDisplay->getPixelFast(0, Row);

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
        pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column + 1, Row));
    }
    if(Rotate) pDisplay->writePixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, FirstPixel);
    else pDisplay->clearPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row);
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

    if(Rotate) if(pDisplay->getPixel(0, Row, &FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
        if(pDisplay->getPixel(Column + 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(pDisplay->writePixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(pDisplay->clearPixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row) == E_NOT_OK) ReturnValue = E_NOT_OK; }

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

    if(Rotate) LastPixel = pDisplay->getPixelFast(DISPLAY_NUMBER_OF_COLUMNS - 1, Row);

    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
        pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column - 1, Row));
    }
    if(Rotate) pDisplay->writePixelFast(0, Row, LastPixel);
    else pDisplay->clearPixelFast(0, Row);
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

    if(Rotate) if(pDisplay->getPixel(DISPLAY_NUMBER_OF_COLUMNS - 1, Row, &LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
        if(pDisplay->getPixel(Column - 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(pDisplay->writePixel(0, Row, LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(pDisplay->clearPixel(0, Row) == E_NOT_OK) ReturnValue = E_NOT_OK; }

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

    if(Rotate) FirstPixel = pDisplay->getPixelFast(Column, 0);

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
        pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column, Row + 1));
    }
    if(Rotate) pDisplay->writePixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1, FirstPixel);
    else pDisplay->clearPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1);
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

    if(Rotate) if(pDisplay->getPixel(Column, 0, &FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
        if(pDisplay->getPixel(Column, Row + 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(pDisplay->writePixel(Column, DISPLAY_NUMBER_OF_ROWS - 1, FirstPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(pDisplay->clearPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1) == E_NOT_OK) ReturnValue = E_NOT_OK; }

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

    if(Rotate) LastPixel = pDisplay->getPixelFast(Column, DISPLAY_NUMBER_OF_ROWS - 1);

    for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
        pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column, Row - 1));
    }
    if(Rotate) pDisplay->writePixelFast(Column, 0, LastPixel);
    else pDisplay->clearPixelFast(Column, 0);
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

    if(Rotate) if(pDisplay->getPixel(Column, DISPLAY_NUMBER_OF_ROWS - 1, &LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
        if(pDisplay->getPixel(Column, Row - 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    if(Rotate) { if(pDisplay->writePixel(Column, 0, LastPixel) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else { if(pDisplay->clearPixel(Column, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftColumnDown */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 