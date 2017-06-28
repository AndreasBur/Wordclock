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
/**     \file       Template.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _DISPLAY_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
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
  CONSTRUCTOR OF TEMPLATE
******************************************************************************************************************************************************/
/*! \brief          Template Constructor
 *  \details        Instantiation of the Template library
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::Display() : WS2812Display(DISPLAY_DATA_PIN)
{
	
} /* Template */


/******************************************************************************************************************************************************
  DESTRUCTOR OF TEMPLATE
******************************************************************************************************************************************************/
Display::~Display()
{

} /* ~Template */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::init()
{

} /* init */


/******************************************************************************************************************************************************
  setChar()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setChar(byte Row, byte Column, byte Red, byte Green, byte Blue)
{
	if(Row < DISPLAY_NUMBER_OF_ROWS && Column < DISPLAY_NUMBER_OF_COLUMNS) {
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
		/* if led stripe is snake or serpentine the odd row: count from right to left */
		WS2812Display.setPixel(transformToSerpentine(Row, Column), Red, Green, Blue);
#else
		WS2812Display.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Red, Green, Blue);
#endif
		return E_OK;
	} else {
		return E_NOT_OK;
	}
} /* setChar */


/******************************************************************************************************************************************************
  setChar()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setChar(byte Index, byte Red, byte Green, byte Blue)
{
	byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
	byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

	return setChar(Row, Column, Red, Green, Blue);
} /* setChar */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
byte Display::transformToSerpentine(byte Row, byte Column)
{
	byte Index;

	if(isBitCleared(Row, 0)) Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
	else Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - Column - 1);
	
	return Index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 