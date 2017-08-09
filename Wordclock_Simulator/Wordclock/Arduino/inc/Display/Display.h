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

#define DISPLAY_WORD_LENGTH_UNLIMITED			0


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

typedef boolean DisplayPixelType;

#if(DISPLAY_NUMBER_OF_ROWS > 16)
#error "Display: too many Rows, please extend DisplayPixelRowType"
#endif

typedef uint16_t DisplayPixelRowType;

#if(DISPLAY_NUMBER_OF_COLUMNS > 16)
#error "Display: too many Columns, please extend DisplayPixelColumnType"
#endif

typedef uint16_t DisplayPixelColumnType;

/* mapping to underlying hardware */
typedef WS2812PixelType PixelColorType;
typedef WS2812 Stripe;


/******************************************************************************************************************************************************
 *  CLASS  Display
******************************************************************************************************************************************************/
class Display
{
  private:
	DisplayStateType State;
    Stripe Pixels;
    PixelColorType Color;

    static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];
    static const DisplayWordIlluminationType WordIlluminationTable[];

    // functions
    byte transformToSerpentine(byte, byte);


  public:
    Display(PixelColorType);
	Display(byte, byte, byte);
    ~Display();

    // get methods
	PixelColorType getColor() { return Color; }
	DisplayStateType getState() { return State; }

    // set methods
    void setColor(PixelColorType sColor) { Color = sColor; }

    // char methods
    stdReturnType setCharacter(DisplayCharacterType Character) { return setPixel(Character); }
    stdReturnType clearCharacter(DisplayCharacterType Character) { return clearPixel(Character); }
    stdReturnType getCharacter(DisplayCharacterType Character, boolean* Value) { return getPixel(Character, Value); }
    stdReturnType getCharacter(byte, byte, char*);
    stdReturnType getCharacter(byte, char*);

	// char methods fast
    void setCharacterFast(DisplayCharacterType Character) { setPixelFast(Character); }
    void clearCharacterFast(DisplayCharacterType Character) { clearPixelFast(Character); }
    boolean getCharacterFast(DisplayCharacterType Character) { return getPixelFast(Character); }
	char getCharacterFast(byte Column, byte Row) { return pgm_read_byte(&DisplayCharacters[Row][Column]); }
	char getCharacterFast(byte Index) { return pgm_read_byte(&DisplayCharacters[Index / DISPLAY_NUMBER_OF_COLUMNS][Index % DISPLAY_NUMBER_OF_COLUMNS]); }

    // word methods
	stdReturnType getWordIllumination(DisplayWordType, DisplayWordIlluminationType*);
    stdReturnType setWord(DisplayWordType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    stdReturnType clearWord(DisplayWordType);
    stdReturnType clearAllWords();

	// word methods fast
	DisplayWordIlluminationType getWordIlluminationFast(DisplayWordType Word) { DisplayWordIlluminationType WordIllu; memcpy_P(&WordIllu, &WordIlluminationTable[Word], sizeof(WordIllu)); return WordIllu; }
    void setWordFast(DisplayWordType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    void clearWordFast(DisplayWordType);
    void clearAllWordsFast();

	// pixel methods
	stdReturnType writePixel(byte Column, byte Row, boolean Value) { if(Value) return setPixel(Column, Row); else return clearPixel(Column, Row); }
	stdReturnType writePixel(byte Index, boolean Value) { if(Value) return setPixel(Index); else return clearPixel(Index); }
    stdReturnType setPixel(byte, byte);
    stdReturnType setPixel(byte);
    stdReturnType clearPixel(byte, byte);
    stdReturnType clearPixel(byte);
    stdReturnType getPixel(byte, byte, boolean*);
    stdReturnType getPixel(byte, boolean*);
    stdReturnType getPixelRow(byte, DisplayPixelRowType*);
    stdReturnType getPixelColumn(byte, DisplayPixelColumnType*);
    stdReturnType setPixelRow(byte, DisplayPixelRowType);
    stdReturnType setPixelColumn(byte, DisplayPixelColumnType);

	// pixel methods fast
	void writePixelFast(byte Column, byte Row, boolean Value) { if(Value) setPixelFast(Column, Row); else clearPixelFast(Column, Row); }
	void writePixelFast(byte Index, boolean Value) { if(Value) setPixelFast(Index); else clearPixelFast(Index); }
	void setPixelFast(byte, byte);
	void setPixelFast(byte);
	void clearPixelFast(byte, byte);
	void clearPixelFast(byte);
	boolean getPixelFast(byte, byte);
	boolean getPixelFast(byte);
    DisplayPixelRowType getPixelRowFast(byte);
    DisplayPixelColumnType getPixelColumnFast(byte);
    void setPixelRowFast(byte, DisplayPixelRowType);
    void setPixelColumnFast(byte, DisplayPixelColumnType);

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
