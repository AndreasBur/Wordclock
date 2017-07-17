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
/**     \file       Display.cpp
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
const char Display::DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1] PROGMEM =
{
    "ESKISTLF�NF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZW�LFP",
    "ZWEINSIEBEN",
    "KDREIRHF�NF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
};


const DisplayWordIlluminationType Display::WordIlluminationTable[] PROGMEM =
{
    {0,0,0},                                //  0 = DISPLAY_WORD_NONE           = ""
    {0,0,2},                                //  1 = DISPLAY_WORD_ES             = "ES"
    {0,3,3},                                //  2 = DISPLAY_WORD_IST            = "IST"
    {0,7,4},                                //  3 = DISPLAY_WORD_FUENF          = "F�NF"
    {1,0,4},                                //  4 = DISPLAY_WORD_ZEHN           = "ZEHN"
    {1,4,7},                                //  5 = DISPLAY_WORD_ZWANZIG        = "ZWANZIG"
    {2,0,4},                                //  6 = DISPLAY_WORD_DREI           = "DREI"
    {2,4,4},                                //  7 = DISPLAY_WORD_VIER           = "VIER"
    {2,4,7},                                //  8 = DISPLAY_WORD_VIERTEL        = "VIERTEL"
    {2,0,11},                               //  9 = DISPLAY_WORD_DREIVIERTEL    = "DREIVIERTEL"
    {3,2,4},                                // 10 = DISPLAY_WORD_NACH           = "NACH"
    {3,6,3},                                // 11 = DISPLAY_WORD_VOR            = "VOR"
    {4,0,4},                                // 12 = DISPLAY_WORD_HALB           = "HALB"
    {4,5,5},                                // 13 = DISPLAY_WORD_HOUR_ZWOELF    = "ZW�LF"
    {5,0,4},                                // 14 = DISPLAY_WORD_HOUR_ZWEI      = "ZWEI"
    {5,2,3},                                // 15 = DISPLAY_WORD_HOUR_EIN       = "EIN"
    {5,2,4},                                // 16 = DISPLAY_WORD_HOUR_EINS      = "EINS"
    {5,5,6},                                // 17 = DISPLAY_WORD_HOUR_SIEBEN    = "SIEBEN"
    {6,1,4},                                // 18 = DISPLAY_WORD_HOUR_DREI      = "DREI"
    {6,7,4},                                // 19 = DISPLAY_WORD_HOUR_FUENF     = "F�NF"
    {7,0,3},                                // 20 = DISPLAY_WORD_HOUR_ELF       = "ELF"
    {7,3,4},                                // 21 = DISPLAY_WORD_HOUR_NEUN      = "NEUN"
    {7,7,4},                                // 22 = DISPLAY_WORD_HOUR_VIER      = "VIER"
    {8,1,4},                                // 23 = DISPLAY_WORD_HOUR_ACHT      = "ACHT"
    {8,5,4},                                // 24 = DISPLAY_WORD_HOUR_ZEHN      = "ZEHN"
    {9,1,5},                                // 25 = DISPLAY_WORD_HOUR_SECHS     = "SECHS"
    {9,8,3},                                // 26 = DISPLAY_WORD_UHR            = "UHR"
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  CONSTRUCTOR OF Display
******************************************************************************************************************************************************/
/*! \brief          Display Constructor
 *  \details        Instantiation of the Display library
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::Display(PixelType sColor)
#ifdef SIMULATOR
 : Pixels(0L, _("Wordclock Simulator"))
#else
 : Pixels(DISPLAY_DATA_PIN)
#endif
{
    Color = sColor;
	State = DISPLAY_STATE_UNINIT;

#ifdef SIMULATOR
	Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  CONSTRUCTOR OF Display
******************************************************************************************************************************************************/
/*! \brief          Display Constructor
 *  \details        Instantiation of the Display library
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::Display(byte Red, byte Green, byte Blue)
#ifdef SIMULATOR
: Pixels(0L, _("Wordclock Simulator"))
#else
: Pixels(DISPLAY_DATA_PIN)
#endif
{
    Color.Red = Red;
	Color.Green = Green;
	Color.Blue = Blue;

#ifdef SIMULATOR
Pixels.Show();
#endif
} /* Display */


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
	clear();
	State = DISPLAY_STATE_INIT;
} /* init */


/******************************************************************************************************************************************************
  getCharacter()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getCharacter(DisplayCharactersType Character, boolean* Value)
{
	stdReturnType ReturnValue = E_NOT_OK;
    PixelType Pixel;
    
    if(Character < DISPLAY_CHARACTER_NUMBER_OF_CHARACTERS) {
		ReturnValue = E_OK;
        if(Pixels.getPixel(Character, &Pixel) == E_NOT_OK) ReturnValue = E_NOT_OK;
        /* Pixel is only off when all colors are zero */
        if(Pixel.Red == 0 && Pixel.Green == 0 && Pixel.Blue == 0) *Value = true;
        else *Value = false;
    } else {
        ReturnValue = E_NOT_OK;
    }
	return ReturnValue;
} /* getCharacter */


/******************************************************************************************************************************************************
  getCharacter()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getCharacter(byte Column, byte Row, char* Character)
{
    if(Row < DISPLAY_NUMBER_OF_ROWS && Column < DISPLAY_NUMBER_OF_COLUMNS) {
        *Character =  getCharacterFast(Row, Column);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getCharacter */


/******************************************************************************************************************************************************
  getCharacter()
  ******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getCharacter(byte Index, char* Character)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if(Index < DISPLAY_NUMBER_OF_LEDS) {
        *Character =  getCharacterFast(Row, Column);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getChar */


/******************************************************************************************************************************************************
  getWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getWord(DisplayWordsType Word)
{
    stdReturnType ReturnValue = E_NOT_OK;
	DisplayWordIlluminationType WordIllu;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		ReturnValue = E_OK;
		memcpy_P(&Word, &WordIlluminationTable[Word], sizeof(WordIllu));

        for(byte Index = 0; Index < WordIllu.Length; Index++) {
			if(setPixel(WordIllu.Column + Index,  WordIllu.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
		}
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setWord */


/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setWord(DisplayWordsType Word)
{
    stdReturnType ReturnValue = E_NOT_OK;
	DisplayWordIlluminationType WordIllu;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		ReturnValue = E_OK;
		memcpy_P(&Word, &WordIlluminationTable[Word], sizeof(WordIllu));

        for(byte Index = 0; Index < WordIllu.Length; Index++) {
			if(setPixel(WordIllu.Column + Index,  WordIllu.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
		}
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setWord */


/******************************************************************************************************************************************************
  clearWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearWord(DisplayWordsType Word)
{
    stdReturnType ReturnValue = E_NOT_OK;
	DisplayWordIlluminationType WordIllu;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		ReturnValue = E_OK;
		memcpy_P(&Word, &WordIlluminationTable[Word], sizeof(WordIllu));

        for(byte Index = 0; Index < WordIllu.Length; Index++) {
			if(clearPixel(WordIllu.Column + Index,  WordIllu.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
		}
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* clearWord */


/******************************************************************************************************************************************************
  clearAllWords()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearAllWords()
{
    stdReturnType ReturnValue = E_OK;

    for(byte i = DISPLAY_WORD_ES; i < DISPLAY_WORD_NUMBER_OF_WORDS; i++) if(clearWord((DisplayWordsType) i) == E_NOT_OK) ReturnValue = E_NOT_OK;
    return ReturnValue;
} /* clearAllWords */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixel(byte Index)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return setPixel(Column,  Row);
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixel(byte Column, byte Row)
{
    if(Row < DISPLAY_NUMBER_OF_ROWS && Column < DISPLAY_NUMBER_OF_COLUMNS) {
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
        /* if led stripe is snake or serpentine the odd row: count from right to left */
        return Pixels.setPixel(transformToSerpentine(Column,  Row), Color);
#else
        return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
#endif
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixel */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearPixel(byte Column, byte Row)
{
    if(Row < DISPLAY_NUMBER_OF_ROWS && Column < DISPLAY_NUMBER_OF_COLUMNS) {
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
        /* if led stripe is snake or serpentine then odd row: count from right to left */
        return Pixels.setPixel(transformToSerpentine(Column,  Row), 0, 0, 0);
#else
        return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, 0, 0, 0);
#endif
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearPixel(byte Index)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return clearPixel(Column,  Row);
} /* clearPixel */



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
byte Display::transformToSerpentine(byte Column, byte Row)
{
    byte Index;

    if(isBitCleared(Row, 0)) Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    else Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - Column - 1);
    
    return Index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 