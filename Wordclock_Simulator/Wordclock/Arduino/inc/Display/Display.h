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
#define DISPLAY_DATA_PIN                        10
#define DISPLAY_LED_STRIPE_SERPENTINE           STD_OFF

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS                  10
#define DISPLAY_NUMBER_OF_COLUMNS               11
#define DISPLAY_NUMBER_OF_LEDS                  WS2812_NUMBER_OF_LEDS

#if((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
    #error "Display: LED number missmatch"
#endif


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/* type which describes the internal state of the Display */
typedef enum {
	DISPLAY_STATE_NONE,
	DISPLAY_STATE_UNINIT,
	DISPLAY_STATE_INIT,
	DISPLAY_STATE_READY
} DisplayStateType;

/* type which describes the  */
typedef struct {
	byte Row;
	byte Column;
	byte Length;
} DisplayWordIlluminationType;

/* mapping to underlying hardware */
typedef WS2812PixelType PixelType;
typedef WS2812 Stripe;

/******************************************************************************************************************************************************
 *  CLASS  Display
******************************************************************************************************************************************************/
class Display
{
  private:
	DisplayStateType State;
    Stripe Pixels;
    PixelType Color;

    static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];
    static const DisplayWordIlluminationType WordIlluminationTable[];

    // functions
    byte transformToSerpentine(byte, byte);


  public:
    Display(PixelType);
	Display(byte, byte, byte);
    ~Display();

    // get methods
	PixelType getColor() { return Color; }
	DisplayStateType getState() { return State; }


    // set methods
    void setColor(PixelType sColor) { Color = sColor; }


    // char methods
    stdReturnType setCharacter(DisplayCharactersType Character) { return Pixels.setPixel(Character, Color); }
    stdReturnType clearCharacter(DisplayCharactersType Character) { return Pixels.setPixel(Character, 0, 0, 0); }
    stdReturnType getCharacter(DisplayCharactersType, boolean*);
    stdReturnType getCharacter(byte, byte, char*);
    stdReturnType getCharacter(byte, char*);

    // word methods
    stdReturnType setWord(DisplayWordsType);
    stdReturnType clearWord(DisplayWordsType);
    stdReturnType clearAllWords();

	// pixel methods
	stdReturnType writePixel(byte Column, byte Row, boolean Value) { if(Value) return setPixel(Column, Row); else return clearPixel(Column, Row); }
	stdReturnType WritePixel(byte Index, boolean Value) { if(Value) return setPixel(Index); else return clearPixel(Index); }
    stdReturnType setPixel(byte, byte);
    stdReturnType setPixel(byte);
    stdReturnType clearPixel(byte, byte);
    stdReturnType clearPixel(byte);
    stdReturnType getPixel(byte, byte, boolean*);
    stdReturnType getPixel(byte, boolean*);

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
