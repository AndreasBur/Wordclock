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


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
void Display::init()
{
    clear();
    Pixels.init(DISPLAY_DATA_PIN);
    State = STATE_INIT;
} /* init */

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
void Display::setBrightness(byte sBrightness)
{
    Brightness = sBrightness;
    byte BrightnessCorrected;
    
    if(BrightnessAutomatic == true) {
        BrightnessCorrected = GCorrection.getCorrectedValue(calculateBrightnessAutomaticCorrected(Brightness));
    } else {
        BrightnessCorrected = GCorrection.getCorrectedValue(Brightness);
    }

    ColorDimmed.Red = dimmColor(Color.Red, BrightnessCorrected);
    ColorDimmed.Green = dimmColor(Color.Green, BrightnessCorrected);
    ColorDimmed.Blue = dimmColor(Color.Blue, BrightnessCorrected);

    for(byte Index = 0; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) {
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
    byte Length;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        returnValue = E_OK;
        DisplayWord word = Words.getDisplayWordFast(WordId);

        if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) Length = word.getLength();
        else Length = MaxLength;

        for(byte Index = 0; Index < Length; Index++) {
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

    if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) length = word.getLength();
    else length = MaxLength;

    for(byte Index = 0; Index < length; Index++) { setPixelFast(word.getColumn() + Index,  word.getRow()); }
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

        for(byte Index = 0; Index < Word.getLength(); Index++) {
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

    for(byte Index = 0; Index < word.getLength(); Index++) { 
        clearPixelFast(word.getColumn() + Index,  word.getRow()); 
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
StdReturnType Display::getPixel(byte Index, bool& Value) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixel(column, row, Value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
bool Display::getPixelFast(byte Index) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixelFast(column, row);
} /* getPixelFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::getPixel(byte Column, byte Row, bool& Value)  const
{
    StdReturnType returnValue{E_NOT_OK};
    PixelColorType pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    returnValue = Pixels.getPixel(transformToSerpentine(Column,  Row), pixel);
#else
    returnValue = Pixels.getPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, pixel);
#endif
    if(returnValue == E_OK) {
        /* Pixel is only off when all colors are zero */
        if(pixel.Red == 0 && pixel.Green == 0 && pixel.Blue == 0) Value = false;
        else Value = true;
    }
    return returnValue;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
bool Display::getPixelFast(byte Column, byte Row)  const
{
    PixelColorType pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    pixel = Pixels.getPixelFast(transformToSerpentine(Column,  Row));
#else
    pixel = Pixels.getPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
    if(pixel.Red == 0 && pixel.Green == 0 && pixel.Blue == 0) return false;
    else return true;
} /* getPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
# if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    return Pixels.setPixel(transformToSerpentine(Column,  Row), Color);
# else
    return Pixels.setPixel(transformToSerpentine(Column,  Row), ColorDimmed);
# endif
#else
# if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
# else
    return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, ColorDimmed);
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
# if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    Pixels.setPixelFast(transformToSerpentine(Column,  Row), Color);
# else
    Pixels.setPixelFast(transformToSerpentine(Column,  Row), ColorDimmed);
# endif
#else
# if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    Pixels.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
# else
    Pixels.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, ColorDimmed);
# endif
#endif
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(byte Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return setPixel(column,  row);
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Index)
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
    return Pixels.clearPixel(transformToSerpentine(Column,  Row));
#else
    return Pixels.clearPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    Pixels.clearPixelFast(transformToSerpentine(Column,  Row));
#else
    Pixels.clearPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixelFast */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::clearPixel(byte Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return clearPixel(column,  row);
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Index)
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
    bool pixel{false};

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    byte index = transformToSerpentine(Column,  Row);
    getPixel(index, &pixel);
    if(pixel) { return clearPixel(index); }
    else { return setPixel(index); }
#else
    byte index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    getPixel(index, pixel);
    if(pixel) { return clearPixel(index); }
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
StdReturnType Display::togglePixel(byte Index)
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return togglePixel(column,  row);
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixelFast()
******************************************************************************************************************************************************/
void Display::togglePixelFast(byte Index)
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
    PixelType pixel;

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(getPixel(column, Row, pixel) == E_OK) {
            WRITE_BIT(PixelRow, column, pixel);
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
    PixelType pixel;

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(getPixel(Column, row, pixel) == E_OK) {
            WRITE_BIT(PixelColumn, row, pixel);
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
Display::Display(PixelColorType sColor) 
#ifdef SIMULATOR
: Pixels(0L, _("Wordclock Simulator"))
#elif (WS2812_IS_SINGLETON == STD_OFF)
: Pixels()
#endif
{
    BrightnessAutomatic = false;
    State = STATE_UNINIT;
    Color = sColor;
    
#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    Brightness = 255;
    ColorDimmed = Color;
#endif

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
    Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  Constructor of Display
******************************************************************************************************************************************************/
Display::Display(ColorType Red, ColorType Green, ColorType Blue)
#ifdef SIMULATOR
: Pixels(0L, _("Wordclock Simulator"))
#elif (WS2812_IS_SINGLETON == STD_OFF)
: Pixels()
#endif
{
    Color.Red = Red;
    Color.Green = Green;
    Color.Blue = Blue;
    State = STATE_UNINIT;

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    Brightness = 255;
    ColorDimmed = Color;
#endif

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
    Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  Destructor of Display
******************************************************************************************************************************************************/
Display::~Display()
{

} /* ~Display */

/******************************************************************************************************************************************************
  calculateBrightnessAutomaticCorrected()
******************************************************************************************************************************************************/
byte Display::calculateBrightnessAutomaticCorrected(byte sBrightness) const
{
     IlluminanceType Illuminance = BH1750::getInstance().getIlluminance();
     IlluminanceType IlluminanceMax = BH1750::getInstance().getCalibrationValuesMaxValue();
     float IlluminanceFactor = static_cast<float>(Illuminance) / IlluminanceMax;
     
     return static_cast<byte>(sBrightness * IlluminanceFactor * DISPLAY_BRIGHTNESS_AUTOMATIC_CORRECTION_FACTOR);
} /* calculateBrightnessAutomaticCorrected */ 

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
byte Display::transformToSerpentine(byte Index) const
{
    byte column = indexToColumn(Index);
    byte row = indexToRow(Index);
  
    return transformToSerpentine(column, row);
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
