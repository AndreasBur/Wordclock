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

/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
void Display::setBrightness(byte sBrightness)
{
    /* only the wish is stored here, what reaches the LEDs is calcBrightness() */
    Brightness.setBrightness(sBrightness);
    applyBrightness();
} /* setBrightness */


/******************************************************************************************************************************************************
  applyBrightness()
******************************************************************************************************************************************************/
void Display::applyBrightness()
{
    /* Before the shortcut below and on every run, because what the cap depends on moves
       without the setting moving: a clock face that lights more letters than the last one
       draws more current at the same brightness. This task is the only place that notices. */
    applyCurrentLimit();

    const byte brightness = Brightness.calcBrightness();

    /* Nothing to do while the calculated value stays put, which is the normal case: only
       the automatic makes it move on its own. Without this check the task would rewrite
       every lit pixel on every run. */
    if(brightness == AppliedBrightness) { return; }
    AppliedBrightness = brightness;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    /* Where the strip does all of the dimming, the master carries the setting and the cap at
       once, so the two are combined here rather than kept apart as applyCurrentLimit() does
       for the other path. */
    const byte Limit = getCurrentLimit();
    PixelStripe.setBrightness((brightness < Limit) ? brightness : Limit);
    AppliedCurrentLimit = Limit;
#else
    applyColor();
#endif
} /* applyBrightness */


/******************************************************************************************************************************************************
  applyColor()
******************************************************************************************************************************************************/
/* The dimmed color is derived from the color and the brightness, and what the pixels carry
   is the dimmed one - so a write to either invalidates both. applyBrightness() above has
   always done this for its own half; the color setters did not, which left a color change
   invisible until the brightness happened to move, redraw or no redraw.

   Nothing to do where the strip does the dimming: there the pixels carry the color itself,
   and the next redraw picks a new one up on its own. */
void Display::applyColor()
{
#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    Color.dimmColors(AppliedBrightness);

    /* the pixels already on the display still carry the previous color */
    for(IndexType Index = 0; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) {
        if(getPixel(Index)) { setPixel(Index); }
    }
#endif
} /* applyColor */


/******************************************************************************************************************************************************
  applyColorAndCurrentLimit()
*******************************************************************************************************************************************************/
/* The colour reaches the pixels, and then the cap is recomputed for it: white at the same
   brightness draws three times what amber does, so a colour change moves the current as much
   as a brightness change and must not wait for the next task run to be noticed. */
void Display::applyColorAndCurrentLimit()
{
    applyColor();
    applyCurrentLimit();
} /* applyColorAndCurrentLimit */


/******************************************************************************************************************************************************
  getNumberOfLitPixels()
*******************************************************************************************************************************************************/
/* Counted from the strip's buffer rather than kept as a running total beside it. The
   animations and setPixel() write there directly, so a counter would be the copy that
   falls behind - the same argument Persistence::gather() is built on. One pass over the
   display per task run, next to the pass applyColor() already makes. */
byte Display::getNumberOfLitPixels() const
{
    static_assert(DISPLAY_NUMBER_OF_PIXELS <= 255u, "Display: a larger display needs a wider lit count");
    byte Count{0u};

    for(IndexType Index = 0u; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) {
        if(getPixel(Index)) { Count++; }
    }
    return Count;
} /* getNumberOfLitPixels */


/******************************************************************************************************************************************************
  getCurrentLimit()
******************************************************************************************************************************************************/
/* Built from the colour and a count rather than from the pixel values, and that is not a
   shortcut: which dimming path is compiled decides whether the buffer holds the dimmed colour
   or the plain one, and only the plain one says what the strip would draw at full brightness.
   Every lit pixel carries the same colour, so a count is all the buffer has to give.

   An upper bound on purpose. setPixel() with its own brightness leaves a pixel dimmer than
   the colour, and a limiter that guessed low would be one that does not protect.

   The colour it asks for is the one being *shown*, not the one that was set: with the colour
   cycle running those are different, and reading the setting would let a saturated hue on the
   strip be budgeted as whatever pale colour happens to be stored underneath it - which is a
   guess in the one direction this must never guess. */
byte Display::getCurrentLimit() const
{
    const Pixel Shown = Color.getColorToShow();
    const uint16_t ChannelSum = DisplayCurrentLimit::toChannelSum(Shown.getRed(), Shown.getGreen(), Shown.getBlue());

    return DisplayCurrentLimit::toBrightnessLimit(DISPLAY_NUMBER_OF_PIXELS, getNumberOfLitPixels(), ChannelSum);
} /* getCurrentLimit */


/******************************************************************************************************************************************************
  applyCurrentLimit()
******************************************************************************************************************************************************/
/* The cap rides on the strip's own master rather than on the colour written into the buffer,
   and that placement is the whole reason this is cheap. The buffer keeps the colour somebody
   asked for at the brightness they asked for; the master says how much of it the supply lets
   out. So a cap that moves needs no redraw, and a clock face restored after a full display
   comes back at the brightness it had rather than at the one the full display was held to.

   Writing the master marks the frame dirty, which is why the remembered value is compared
   first: without that, every task run would queue a transmission that changes nothing. */
void Display::applyCurrentLimit()
{
    /* Off stays off - the master is also what disable() uses. enable() asks for the cap again
       on its way back, so nothing is lost by leaving it alone here. */
    if(!Enabled) { return; }

    const byte Limit = getCurrentLimit();

    if(Limit == AppliedCurrentLimit) { return; }
    AppliedCurrentLimit = Limit;
    PixelStripe.setBrightness(Limit);
} /* applyCurrentLimit */


/******************************************************************************************************************************************************
  resumeAtCurrentLimit()
******************************************************************************************************************************************************/
/* What enable() uses instead of enablePixels(). The remembered value is set from the same
   computation that is written, so the guard in applyCurrentLimit() agrees with the strip
   afterwards. */
void Display::resumeAtCurrentLimit()
{
    AppliedCurrentLimit = getCurrentLimit();
    PixelStripe.setBrightness(AppliedCurrentLimit);
} /* resumeAtCurrentLimit */

/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
StdReturnType Display::setWord(WordIdType WordId, byte MaxLength)
{
    StdReturnType returnValue{E_NOT_OK};
    byte length;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        returnValue = E_OK;
        DisplayWord word = Words.getDisplayWord(WordId);
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
  clearWord()
******************************************************************************************************************************************************/
StdReturnType Display::clearWord(WordIdType WordId)
{
    StdReturnType returnValue{E_NOT_OK};

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        returnValue = E_OK;
        DisplayWord Word = Words.getDisplayWord(WordId);

        for(IndexType Index = 0; Index < Word.getLength(); Index++) {
            if(clearPixel(Word.getColumn() + Index,  Word.getRow()) == E_NOT_OK) returnValue = E_NOT_OK;
        }
    } else {
        returnValue = E_NOT_OK;
    }
    return returnValue;
} /* clearWord */


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
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::getPixel(IndexType Index, PixelValueType& Value) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixel(column, row, Value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
Display::PixelValueType Display::getPixel(IndexType Index) const
{
    byte row, column;
    indexToColumnAndRow(Index, column, row);
    return getPixel(column, row);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::getPixel(byte Column, byte Row, PixelValueType& Value)  const
{
    if(!isColumnAndRowValid(Column, Row)) { return E_NOT_OK; }

    Value = getPixel(Column, Row);
    return E_OK;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
Display::PixelValueType Display::getPixel(byte Column, byte Row)  const
{
    if(!isColumnAndRowValid(Column, Row)) { return false; }

    const Pixel pixel = PixelStripe.getPixel(toIndex(Column, Row));

    /* Pixel is only off when all colors are zero */
    return !(pixel.getRed() == 0u && pixel.getGreen() == 0u && pixel.getBlue() == 0u);
} /* getPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(byte Column, byte Row)
{
    if(!isColumnAndRowValid(Column, Row)) { return E_NOT_OK; }

#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    return PixelStripe.setPixel(toIndex(Column, Row), Color);
#else
    return PixelStripe.setPixel(toIndex(Column, Row), Color.getColorDimmed());
#endif
} /* setPixel */


/******************************************************************************************************************************************************
  getColorDimmed()
******************************************************************************************************************************************************/
Pixel Display::getColorDimmed(byte Brightness)
{
    /* the display color stays the only color, the brightness just dimms it. It comes
       on top of the display brightness, which is already part of the color here. */
#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    Pixel pixelColor = Color.getColor();
#else
    Pixel pixelColor = Color.getColorDimmed();
#endif
    pixelColor.dimmPixel(Brightness);

    return pixelColor;
} /* getColorDimmed */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::setPixel(byte Column, byte Row, byte Brightness)
{
    if(!isColumnAndRowValid(Column, Row)) { return E_NOT_OK; }

    return PixelStripe.setPixel(toIndex(Column, Row), getColorDimmed(Brightness));
} /* setPixel */


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
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::clearPixel(byte Column, byte Row)
{
    if(!isColumnAndRowValid(Column, Row)) { return E_NOT_OK; }

    return PixelStripe.clearPixel(toIndex(Column, Row));
} /* clearPixel */


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
  togglePixel()
******************************************************************************************************************************************************/
StdReturnType Display::togglePixel(byte Column, byte Row)
{
    if(!isColumnAndRowValid(Column, Row)) { return E_NOT_OK; }

    /* The column and the row are what is toggled, not the index they map to: the serpentine
       branch used to transform them into an index and hand that to functions which transform
       it a second time, so on an odd row it toggled the pixel mirrored about the middle of
       the display. Nothing showed it, because nothing here is built with the serpentine
       wiring - and the same branch passed a pointer where a reference was expected, so it
       would not have compiled if anything had. */
    if(getPixel(Column, Row)) { return clearPixel(Column, Row); }
    else { return setPixel(Column, Row); }
} /* togglePixel */


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
  getPixelRow()
******************************************************************************************************************************************************/
Display::PixelRowType Display::getPixelRow(byte Row)  const
{
    PixelRowType pixelRow{0};

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        WRITE_BIT(pixelRow, column, getPixel(column, Row));
    }
    return pixelRow;
} /* getPixelRow */


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
  getPixelColumn()
******************************************************************************************************************************************************/
Display::PixelColumnType Display::getPixelColumn(byte Column)  const
{
    Display::PixelColumnType pixelColumn{0};

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        WRITE_BIT(pixelColumn, row, getPixel(Column, row));
    }
    return pixelColumn;
} /* getPixelColumn */


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
