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


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
void Display::init()
{
    clear();
    PixelStripe.init(DISPLAY_DATA_PIN);
    State = STATE_INIT;
} /* init */

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
void Display::setBrightness(byte sBrightness)
{
    Brightness.setBrightness(sBrightness);
    Color.dimmColors(sBrightness);

    for(IndexType Index = 0; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) {
        // update all pixels to new brightness
        if(getPixelFast(Index)) { setPixelFast(Index); }
    }
} /* setBrightness */
#endif

/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
StdReturnType Display::setWord(WordIdType WordId, byte MaxLength)
{
    StdReturnType returnValue{E_NOT_OK};
    byte length;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        returnValue = E_OK;
        DisplayWord word = Words.getDisplayWordFast(WordId);
        byte WordLength = word.getLength();

        if(MaxLength == WordLengthUnlimited || MaxLength >= WordLength) { length = WordLength; }
        else { length = MaxLength; }

        for(IndexType Index = 0; Index < length; Index++) {
            if(setPixel(word.getColumn() + Index,  word.getRow()) == E_NOT_OK) returnValue = E_NOT_OK;
        }
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* setWord */


/******************************************************************************************************************************************************
  setWordFast()
******************************************************************************************************************************************************/
void Display::setWordFast(WordIdType WordId, byte MaxLength)
{
    byte length;
    DisplayWord word = Words.getDisplayWordFast(WordId);

    if(MaxLength == WordLengthUnlimited) length = word.getLength();
    else length = MaxLength;

    for(IndexType Index = 0; Index < length; Index++) { setPixelFast(word.getColumn() + Index,  word.getRow()); }
} /* setWordFast */


/******************************************************************************************************************************************************
  clearWord()
******************************************************************************************************************************************************/
StdReturnType Display::clearWord(WordIdType WordId)
{
    StdReturnType returnValue{E_NOT_OK};

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        returnValue = E_OK;
        DisplayWord Word = Words.getDisplayWordFast(WordId);

        for(IndexType Index = 0; Index < Word.getLength(); Index++) {
            if(clearPixel(Word.getColumn() + Index,  Word.getRow()) == E_NOT_OK) returnValue = E_NOT_OK;
        }
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* clearWord */


/******************************************************************************************************************************************************
  clearWordFast()
******************************************************************************************************************************************************/
void Display::clearWordFast(WordIdType WordId)
{
    DisplayWord word = Words.getDisplayWordFast(WordId);

    for(IndexType index = 0; index < word.getLength(); index++) {
        clearPixelFast(word.getColumn() + index,  word.getRow());
    }
} /* clearWordFast */


/******************************************************************************************************************************************************
  clearAllWords()
******************************************************************************************************************************************************/
StdReturnType Display::clearWords()
{
    StdReturnType returnValue{E_OK};

    for(byte i = DisplayWords::WORD_ES; i < DisplayWords::WORD_NUMBER_OF_WORDS; i++) {
        if(clearWord((WordIdType) i) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    return returnValue;
} /* clearAllWords */


/******************************************************************************************************************************************************
  clearAllWordsFast()
******************************************************************************************************************************************************/
void Display::clearWordsFast()
{
    for(byte i = DisplayWords::WORD_ES; i < DisplayWords::WORD_NUMBER_OF_WORDS; i++) clearWordFast((WordIdType) i);
} /* clearAllWordsFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::getPixel(IndexType Index, PixelValueType& Value) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixel(column, row, Value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
Display::PixelValueType Display::getPixelFast(IndexType Index) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixelFast(column, row);
} /* getPixelFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::getPixel(byte Column, byte Row, PixelValueType& Value)  const
{
    StdReturnType returnValue{E_NOT_OK};
    Pixel pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    returnValue = PixelStripe.getPixel(transformToSerpentine(Column,  Row), pixel);
#else
    returnValue = PixelStripe.getPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, pixel);
#endif
    if(returnValue == E_OK) {
        /* Pixel is only off when all colors are zero */
        if(pixel.getRed() == 0 && pixel.getGreen() == 0 && pixel.getBlue() == 0) Value = false;
        else Value = true;
    }
    return returnValue;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
Display::PixelValueType Display::getPixelFast(byte Column, byte Row)  const
{
    Pixel pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    pixel = PixelStripe.getPixelFast(transformToSerpentine(Column,  Row));
#else
    pixel = PixelStripe.getPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
    if(pixel.getRed() == 0u && pixel.getGreen() == 0u && pixel.getBlue() == 0u) return false;
    else return true;
} /* getPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    return PixelStripe.setPixel(transformToSerpentine(Column,  Row), Color);
# else
    return PixelStripe.setPixel(transformToSerpentine(Column,  Row), Color.getColorDimmed());
# endif
#else
# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    return PixelStripe.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
# else
    return PixelStripe.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color.getColorDimmed());
# endif
#endif
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    PixelStripe.setPixelFast(transformToSerpentine(Column,  Row), Color);
# else
    PixelStripe.setPixelFast(transformToSerpentine(Column,  Row), Color.getColorDimmed());
# endif
#else
# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    PixelStripe.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
# else
    PixelStripe.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color.getColorDimmed());
# endif
#endif
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return setPixel(column,  row);
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Display::setPixelFast(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    setPixelFast(column,  row);
} /* setPixelFast */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::clearPixel(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    return PixelStripe.clearPixel(transformToSerpentine(Column,  Row));
#else
    return PixelStripe.clearPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    PixelStripe.clearPixelFast(transformToSerpentine(Column,  Row));
#else
    PixelStripe.clearPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixelFast */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::clearPixel(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return clearPixel(column,  row);
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
void Display::clearPixelFast(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    clearPixelFast(column,  row);
} /* clearPixelFast */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
StdReturnType Display::togglePixel(byte Column, byte Row)
{
    PixelValueType pixelValue{false};

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    byte index = transformToSerpentine(Column,  Row);
    getPixel(index, &pixelValue);
    if(pixelValue) { return clearPixel(index); }
    else { return setPixel(index); }
#else
    byte index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    getPixel(index, pixelValue);
    if(pixelValue) { return clearPixel(index); }
    else { return setPixel(index); }
#endif
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixelFast()
******************************************************************************************************************************************************/
void Display::togglePixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    byte index = transformToSerpentine(Column,  Row);
    if(getPixelFast(index)) clearPixelFast(index);
    else setPixelFast(index);
#else
    byte index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    if(getPixelFast(index)) clearPixelFast(index);
    else setPixelFast(index);
#endif
} /* togglePixelFast */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
StdReturnType Display::togglePixel(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return togglePixel(column,  row);
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixelFast()
******************************************************************************************************************************************************/
void Display::togglePixelFast(IndexType Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    togglePixelFast(column, row);
} /* togglePixelFast */


/******************************************************************************************************************************************************
  getPixelRow()
******************************************************************************************************************************************************/
StdReturnType Display::getPixelRow(byte Row, PixelRowType& PixelRow) const
{
    StdReturnType returnValue{E_OK};
    PixelValueType pixelValue;

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(getPixel(column, Row, pixelValue) == E_OK) {
            WRITE_BIT(PixelRow, column, pixelValue);
        } else {
            returnValue = E_NOT_OK;
        }
    }
    return returnValue;
} /* getPixelRow */


/******************************************************************************************************************************************************
  getPixelRowFast()
******************************************************************************************************************************************************/
Display::PixelRowType Display::getPixelRowFast(byte Row)  const
{
    PixelRowType pixelRow{0};

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        WRITE_BIT(pixelRow, column, getPixelFast(column, Row));
    }
    return pixelRow;
} /* getPixelRowFast */


/******************************************************************************************************************************************************
  getPixelColumn()
******************************************************************************************************************************************************/
StdReturnType Display::getPixelColumn(byte Column, PixelRowType& PixelColumn)  const
{
    StdReturnType returnValue{E_OK};
    PixelValueType pixelValue;

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(getPixel(Column, row, pixelValue) == E_OK) {
            WRITE_BIT(PixelColumn, row, pixelValue);
        } else {
            returnValue = E_NOT_OK;
        }
    }
    return returnValue;
} /* getPixelColumn */


/******************************************************************************************************************************************************
  getPixelColumnFast()
******************************************************************************************************************************************************/
Display::PixelColumnType Display::getPixelColumnFast(byte Column)  const
{
    Display::PixelColumnType pixelColumn{0};

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        WRITE_BIT(pixelColumn, row, getPixelFast(Column, row));
    }
    return pixelColumn;
} /* getPixelColumnFast */


/******************************************************************************************************************************************************
  setPixelRow()
******************************************************************************************************************************************************/
StdReturnType Display::setPixelRow(byte Row, PixelRowType PixelRow)
{
    StdReturnType returnValue{E_OK};

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(writePixel(column, Row, READ_BIT(PixelRow, column)) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    return returnValue;
} /* setPixelRow */


/******************************************************************************************************************************************************
  setPixelRowFast()
******************************************************************************************************************************************************/
void Display::setPixelRowFast(byte Row, PixelRowType PixelRow)
{
    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        writePixelFast(column, Row, READ_BIT(PixelRow, column));
    }
} /* setPixelRowFast */


/******************************************************************************************************************************************************
  setPixelColumn()
******************************************************************************************************************************************************/
StdReturnType Display::setPixelColumn(byte Column, PixelRowType PixelColumn)
{
    StdReturnType returnValue{E_OK};
    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(writePixel(Column, row, READ_BIT(PixelColumn, row)) == E_NOT_OK) returnValue = E_NOT_OK;
    }
    return returnValue;
} /* setPixelColumn */


/******************************************************************************************************************************************************
  setPixelColumnFast()
******************************************************************************************************************************************************/
void Display::setPixelColumnFast(byte Column, PixelRowType PixelColumn)
{
    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        writePixelFast(Column, row, READ_BIT(PixelColumn, row));
    }
} /* setPixelColumnFast */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of Display
******************************************************************************************************************************************************/
Display::Display(Pixel sColor) : PixelStripe{Pixels::getInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
: PixelStripe()
#endif
{
    State = STATE_UNINIT;
    Color.setColor(sColor);

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    Brightness.setBrightness(255u);
#endif
} /* Display */


/******************************************************************************************************************************************************
  Constructor of Display
******************************************************************************************************************************************************/
Display::Display(ColorType Red, ColorType Green, ColorType Blue) : PixelStripe{Pixels::getInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
: PixelStripe()
#endif
{
    Color.setColorRed(Red);
    Color.setColorGreen(Green);
    Color.setColorBlue(Blue);
    State = STATE_UNINIT;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    Brightness.setBrightness(255u);
#endif
} /* Display */


/******************************************************************************************************************************************************
  Destructor of Display
******************************************************************************************************************************************************/
Display::~Display()
{

} /* ~Display */

/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
byte Display::transformToSerpentine(byte Column, byte Row) const
{
    byte index;

    if(IS_BIT_CLEARED(Row, 0)) index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    else index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - Column - 1);

    return index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
byte Display::transformToSerpentine(IndexType Index) const
{
    byte column = indexToColumn(Index);
    byte row = indexToRow(Index);

    return transformToSerpentine(column, row);
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
