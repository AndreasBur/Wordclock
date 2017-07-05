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
/**     \file       Display.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayCharacters.h"
#include "DisplayWords.h"
#include "WS2812.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN						10
#define DISPLAY_LED_STRIPE_SERPENTINE			STD_ON

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS					10
#define DISPLAY_NUMBER_OF_COLUMNS				11
#define DISPLAY_NUMBER_OF_LEDS					WS2812_NUMBER_OF_LEDS

#if((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
	#error "Display: LED number missmatch"
#endif

//#define DisplayLedRgbType						WS2812PixelType


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
typedef struct {
	byte Row;
	byte Column;
	byte Length;
} DisplayWordIlluminationType;


/******************************************************************************************************************************************************
 *  CLASS  Display
******************************************************************************************************************************************************/
class Display
{
  private:
	WS2812 Pixels;
	WS2812PixelType DisplayColor;

	static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];
	static const DisplayWordIlluminationType WordIlluminationTable[];
	
	// functions
	byte transformToSerpentine(byte, byte);
	stdReturnType setPixel(byte Row, byte Column);
	stdReturnType setPixel(byte Index);
	stdReturnType clearPixel(byte Row, byte Column);
	stdReturnType clearPixel(byte Index);
	stdReturnType getPixel(byte, byte, boolean*);
	stdReturnType getPixel(byte, boolean*);

  public:
	Display();
	~Display();

	// get methods


	// set methods
	

	// char methods
	stdReturnType setCharacter(DisplayCharactersType Character) { return Pixels.setPixel(Character, DisplayColor); }
	stdReturnType clearCharacter(DisplayCharactersType Character) { return Pixels.setPixel(Character, 0, 0, 0); }
	stdReturnType getCharacter(DisplayCharactersType Character, boolean* Value);
	stdReturnType getCharacter(byte Row, byte Column, char* Character);
	stdReturnType getCharacter(byte Index, char* Character);

	// word methods
	stdReturnType setWord(DisplayWordsType);
	stdReturnType clearWord(DisplayWordsType);
	stdReturnType clearAllWords();

	// methods
	void init();
	void show() { Pixels.show(); }
	void test();
	void clear() { Pixels.clearAllPixels(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
