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
#if defined(SIMULATOR) && (defined(__APPLE__ ) || defined(__linux__))
#include "WordclockIcon.xpm"
#endif

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
    "ESKISTLFÜNF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZWÖLFP",
    "ZWEINSIEBEN",
    "KDREIRHFÜNF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
};


const DisplayWordIlluminationType Display::WordIlluminationTable[] PROGMEM =
{
    {0,0,0},                                //  0 = DISPLAY_WORD_NONE           = ""
    {0,0,2},                                //  1 = DISPLAY_WORD_ES             = "ES"
    {0,3,3},                                //  2 = DISPLAY_WORD_IST            = "IST"
    {0,7,4},                                //  3 = DISPLAY_WORD_FUENF          = "FÜNF"
    {1,0,4},                                //  4 = DISPLAY_WORD_ZEHN           = "ZEHN"
    {1,4,7},                                //  5 = DISPLAY_WORD_ZWANZIG        = "ZWANZIG"
    {2,0,4},                                //  6 = DISPLAY_WORD_DREI           = "DREI"
    {2,4,4},                                //  7 = DISPLAY_WORD_VIER           = "VIER"
    {2,4,7},                                //  8 = DISPLAY_WORD_VIERTEL        = "VIERTEL"
    {2,0,11},                               //  9 = DISPLAY_WORD_DREIVIERTEL    = "DREIVIERTEL"
    {3,2,4},                                // 10 = DISPLAY_WORD_NACH           = "NACH"
    {3,6,3},                                // 11 = DISPLAY_WORD_VOR            = "VOR"
    {4,0,4},                                // 12 = DISPLAY_WORD_HALB           = "HALB"
    {4,5,5},                                // 13 = DISPLAY_WORD_HOUR_ZWOELF    = "ZWÖLF"
    {5,0,4},                                // 14 = DISPLAY_WORD_HOUR_ZWEI      = "ZWEI"
    {5,2,3},                                // 15 = DISPLAY_WORD_HOUR_EIN       = "EIN"
    {5,2,4},                                // 16 = DISPLAY_WORD_HOUR_EINS      = "EINS"
    {5,5,6},                                // 17 = DISPLAY_WORD_HOUR_SIEBEN    = "SIEBEN"
    {6,1,4},                                // 18 = DISPLAY_WORD_HOUR_DREI      = "DREI"
    {6,7,4},                                // 19 = DISPLAY_WORD_HOUR_FUENF     = "FÜNF"
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
Display::Display(PixelColorType sColor) : Pixels(DISPLAY_DATA_PIN)
{
    Color = sColor;
	State = DISPLAY_STATE_UNINIT;

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
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
Display::Display(byte Red, byte Green, byte Blue) : Pixels(DISPLAY_DATA_PIN)
{
    Color.Red = Red;
	Color.Green = Green;
	Color.Blue = Blue;

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
    Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  DESTRUCTOR OF Display
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
  getWordIllumination()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getWordIllumination(DisplayWordType Word, DisplayWordIlluminationType* WordIllu)
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		*WordIllu = getWordIlluminationFast(Word);
		ReturnValue = E_OK;
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* getWordIllumination */


/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setWord(DisplayWordType Word, byte MaxLength)
{
    stdReturnType ReturnValue = E_NOT_OK;
	DisplayWordIlluminationType WordIllu;
	byte Length;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		ReturnValue = E_OK;
		WordIllu = getWordIlluminationFast(Word);

		if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) Length = WordIllu.Length;
		else Length = MaxLength;

        for(byte Index = 0; Index < Length; Index++) {
			if(setPixel(WordIllu.Column + Index,  WordIllu.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
		}
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setWord */


/******************************************************************************************************************************************************
  setWordFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setWordFast(DisplayWordType Word, byte MaxLength)
{
	byte Length;

	DisplayWordIlluminationType WordIllu = getWordIlluminationFast(Word);

	if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) Length = WordIllu.Length;
	else Length = MaxLength;

    for(byte Index = 0; Index < Length; Index++) { setPixelFast(WordIllu.Column + Index,  WordIllu.Row); }
} /* setWordFast */


/******************************************************************************************************************************************************
  clearWord()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearWord(DisplayWordType Word)
{
    stdReturnType ReturnValue = E_NOT_OK;
	DisplayWordIlluminationType WordIllu;

    if(Word < DISPLAY_WORD_NUMBER_OF_WORDS) {
		ReturnValue = E_OK;
		WordIllu = getWordIlluminationFast(Word);

        for(byte Index = 0; Index < WordIllu.Length; Index++) {
			if(clearPixel(WordIllu.Column + Index,  WordIllu.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
		}
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* clearWord */


/******************************************************************************************************************************************************
  clearWordFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearWordFast(DisplayWordType Word)
{
	DisplayWordIlluminationType WordIllu = getWordIlluminationFast(Word);

    for(byte Index = 0; Index < WordIllu.Length; Index++) { clearPixelFast(WordIllu.Column + Index,  WordIllu.Row); }
} /* clearWordFast */


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

    for(byte i = DISPLAY_WORD_ES; i < DISPLAY_WORD_NUMBER_OF_WORDS; i++) if(clearWord((DisplayWordType) i) == E_NOT_OK) ReturnValue = E_NOT_OK;
    return ReturnValue;
} /* clearAllWords */


/******************************************************************************************************************************************************
  clearAllWordsFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearAllWordsFast()
{
    for(byte i = DISPLAY_WORD_ES; i < DISPLAY_WORD_NUMBER_OF_WORDS; i++) clearWordFast((DisplayWordType) i);
} /* clearAllWordsFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixel(byte Index, boolean* Value)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return getPixel(Row, Column, Value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean Display::getPixelFast(byte Index)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    return getPixelFast(Row, Column);
} /* getPixelFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixel(byte Column, byte Row, boolean* Value)
{
	stdReturnType ReturnValue = E_NOT_OK;
    PixelColorType Pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    ReturnValue = Pixels.getPixel(transformToSerpentine(Column,  Row), &Pixel);
#else
    ReturnValue = Pixels.getPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, &Pixel);
#endif
    if(ReturnValue == E_OK) {
        /* Pixel is only off when all colors are zero */
        if(Pixel.Red == 0 && Pixel.Green == 0 && Pixel.Blue == 0) *Value = false;
        else *Value = true;
    }
	return ReturnValue;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean Display::getPixelFast(byte Column, byte Row)
{
	PixelColorType Pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
	/* if led stripe is snake or serpentine then odd row: count from right to left */
	Pixel = Pixels.getPixelFast(transformToSerpentine(Column,  Row));
#else
	Pixel = Pixels.getPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
	if(Pixel.Red == 0 && Pixel.Green == 0 && Pixel.Blue == 0) return false;
	else return true;
} /* getPixelFast */


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
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
    return Pixels.setPixel(transformToSerpentine(Column,  Row), Color);
#else
    return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
#endif
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
    Pixels.setPixelFast(transformToSerpentine(Column,  Row), Color);
#else
    Pixels.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
#endif
} /* setPixelFast */


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
  setPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Index)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    setPixelFast(Column,  Row);
} /* setPixelFast */


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
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    return Pixels.setPixel(transformToSerpentine(Column,  Row), 0, 0, 0);
#else
    return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, 0, 0, 0);
#endif
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    Pixels.setPixelFast(transformToSerpentine(Column,  Row), 0, 0, 0);
#else
    Pixels.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, 0, 0, 0);
#endif
} /* clearPixelFast */


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
  clearPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Index)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    clearPixelFast(Column,  Row);
} /* clearPixelFast */


/******************************************************************************************************************************************************
  getPixelRow()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixelRow(byte Row, DisplayPixelRowType* PixelRow)
{
    stdReturnType ReturnValue = E_OK;
    DisplayPixelType Pixel;

	for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(getPixel(Column, Row, &Pixel) == E_OK) {
            writeBit(*PixelRow, Column, Pixel);
        } else {
            ReturnValue = E_NOT_OK;
        }
	}
	return ReturnValue;
} /* getPixelRow */


/******************************************************************************************************************************************************
  getPixelRowFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
DisplayPixelRowType Display::getPixelRowFast(byte Row)
{
	DisplayPixelRowType PixelRow = 0;

	for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
		writeBit(PixelRow, Column, getPixelFast(Column, Row));
	}
	return PixelRow;
} /* getPixelRowFast */


/******************************************************************************************************************************************************
  getPixelColumn()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixelColumn(byte Column, DisplayPixelRowType* PixelColumn)
{
    stdReturnType ReturnValue = E_OK;
    DisplayPixelType Pixel;

	for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(getPixel(Column, Row, &Pixel) == E_OK) {
            writeBit(*PixelColumn, Row, Pixel);
        } else {
            ReturnValue = E_NOT_OK;
        }
	}
    return ReturnValue;
} /* getPixelColumn */


/******************************************************************************************************************************************************
  getPixelColumnFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
DisplayPixelColumnType Display::getPixelColumnFast(byte Column)
{
	DisplayPixelColumnType PixelColumn = 0;

	for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
		writeBit(PixelColumn, Row, getPixelFast(Column, Row));
	}
	return PixelColumn;
} /* getPixelColumnFast */


/******************************************************************************************************************************************************
  setPixelRow()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixelRow(byte Row, DisplayPixelRowType PixelRow)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(writePixel(Column, Row, readBit(PixelRow, Column)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setPixelRow */


/******************************************************************************************************************************************************
  setPixelRowFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelRowFast(byte Row, DisplayPixelRowType PixelRow)
{
	for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        writePixelFast(Column, Row, readBit(PixelRow, Column));
    }
} /* setPixelRowFast */


/******************************************************************************************************************************************************
  setPixelColumn()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixelColumn(byte Column, DisplayPixelRowType PixelColumn)
{
    stdReturnType ReturnValue = E_OK;
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(writePixel(Column, Row, readBit(PixelColumn, Row)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setPixelColumn */


/******************************************************************************************************************************************************
  setPixelColumnFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelColumnFast(byte Column, DisplayPixelRowType PixelColumn)
{
	for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        writePixelFast(Column, Row, readBit(PixelColumn, Row));
    }
} /* setPixelColumnFast */


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
