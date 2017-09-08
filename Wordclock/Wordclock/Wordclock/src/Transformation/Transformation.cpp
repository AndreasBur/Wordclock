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
  CONSTRUCTOR OF Transformation
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
  DESTRUCTOR OF Transformation
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
    DisplayPixelColumnType PixelColumn;

    if(Rotate) PixelColumn = pDisplay->getPixelColumnFast(0);

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
            pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column + 1, Row));
        }
    }
    if(Rotate) pDisplay->setPixelColumnFast(DISPLAY_NUMBER_OF_COLUMNS - 1, PixelColumn);
    else pDisplay->setPixelColumnFast(DISPLAY_NUMBER_OF_COLUMNS - 1, 0);
} /* shiftLeftFast */


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
    DisplayPixelColumnType PixelColumn;
    DisplayPixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(pDisplay->getPixelColumn(0, &PixelColumn) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS - 1; Column++) {
            if(pDisplay->getPixel(Column + 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
            if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    if(Rotate){ if(pDisplay->setPixelColumn(DISPLAY_NUMBER_OF_COLUMNS - 1, PixelColumn) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(pDisplay->setPixelColumn(DISPLAY_NUMBER_OF_COLUMNS - 1, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftLeft */


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
    DisplayPixelColumnType PixelColumn;

    if(Rotate) PixelColumn = pDisplay->getPixelColumnFast(DISPLAY_NUMBER_OF_COLUMNS - 1);

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
            pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column - 1, Row));
        }
    }
    if(Rotate) pDisplay->setPixelColumnFast(0, PixelColumn);
    else pDisplay->setPixelColumnFast(0, 0);
} /* shiftRightFast */


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
    DisplayPixelColumnType PixelColumn;
    DisplayPixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(pDisplay->getPixelColumn(DISPLAY_NUMBER_OF_COLUMNS - 1, &PixelColumn) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(int8_t Column = DISPLAY_NUMBER_OF_COLUMNS - 1; Column > 0; Column--) {
            if(pDisplay->getPixel(Column - 1, Row, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
            if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    if(Rotate){ if(pDisplay->setPixelColumn(0, PixelColumn) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(pDisplay->setPixelColumn(0, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftRight */


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
    DisplayPixelRowType PixelRow;

    if(Rotate) PixelRow = pDisplay->getPixelRowFast(0);

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
            pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column, Row + 1));
        }
    }
    if(Rotate) pDisplay->setPixelRowFast(DISPLAY_NUMBER_OF_ROWS - 1, PixelRow);
    else pDisplay->setPixelRowFast(DISPLAY_NUMBER_OF_ROWS - 1, 0);
} /* shiftUpFast */


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
    DisplayPixelRowType PixelRow;
    DisplayPixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(pDisplay->getPixelRow(0, &PixelRow) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS - 1; Row++) {
            if(pDisplay->getPixel(Column, Row + 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
            if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    if(Rotate){ if(pDisplay->setPixelRow(DISPLAY_NUMBER_OF_ROWS - 1, PixelRow) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(pDisplay->setPixelRow(DISPLAY_NUMBER_OF_ROWS - 1, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftUp */


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
    DisplayPixelRowType PixelRow;

    if(Rotate) PixelRow = pDisplay->getPixelRowFast(DISPLAY_NUMBER_OF_ROWS - 1);

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
            pDisplay->writePixelFast(Column, Row, pDisplay->getPixelFast(Column, Row - 1));
        }
    }
    if(Rotate) pDisplay->setPixelRowFast(0, PixelRow);
    else pDisplay->setPixelRowFast(0, 0);
} /* shiftDownFast */


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
    DisplayPixelRowType PixelRow;
    DisplayPixelType Pixel;
    stdReturnType ReturnValue = E_OK;

    if(Rotate) if(pDisplay->getPixelRow(DISPLAY_NUMBER_OF_ROWS - 1, &PixelRow) == E_NOT_OK) ReturnValue = E_NOT_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        for(int8_t Row = DISPLAY_NUMBER_OF_ROWS - 1; Row > 0; Row--) {
            if(pDisplay->getPixel(Column, Row - 1, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
            if(pDisplay->writePixel(Column, Row, Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    }
    if(Rotate){ if(pDisplay->setPixelRow(0, PixelRow) == E_NOT_OK) ReturnValue = E_NOT_OK; }
    else{ if(pDisplay->setPixelRow(0, 0) == E_NOT_OK) ReturnValue = E_NOT_OK; }

    return ReturnValue;
} /* shiftDown */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 