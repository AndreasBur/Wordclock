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
/**     \file       Display_Character.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_CHARACTER_H_
#define _DISPLAY_CHARACTER_H_

/******************************************************************************************************************************************************
*  INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"


/******************************************************************************************************************************************************
*  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/

 
/******************************************************************************************************************************************************
*  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
*  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
*  GLOBAL FUNCTION PROTOTYPES
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 *  CLASS  DisplayCharacter
******************************************************************************************************************************************************/
class DisplayCharacter
{
  private:
	Display* pDisplay;
	static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];

  public:
	DisplayCharacter();
	~DisplayCharacter();

	// get methods


	// set methods

	// methods
	stdReturnType getChar(byte Row, byte Column, char* Char);
	stdReturnType getChar(byte Index, char* Char);
	//stdReturnType setChar(byte Index) { return pDisplay->setLed(Index); }
};

#endif

/******************************************************************************************************************************************************
*  E N D   O F   F I L E
******************************************************************************************************************************************************/
