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
void Display::Init()
{
    Clear();
    pixel_stripe_.Init(DISPLAY_DATA_PIN);
    state_ = kStateInit;
} /* init */

/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
void Display::SetBrightness(byte brightness)
{
    /* only the wish is stored here, what reaches the LEDs is calcBrightness() */
    brightness_.SetBrightness(brightness);
    ApplyBrightness();
} /* setBrightness */


/******************************************************************************************************************************************************
  applyBrightness()
******************************************************************************************************************************************************/
void Display::ApplyBrightness()
{
    /* Before the shortcut below and on every run, because what the cap depends on moves
       without the setting moving: a clock face that lights more letters than the last one
       draws more current at the same brightness. This task is the only place that notices. */
    ApplyCurrentLimit();

    const byte brightness = brightness_.CalcBrightness();

    /* Nothing to do while the calculated value stays put, which is the normal case: only
       the automatic makes it move on its own. Without this check the task would rewrite
       every lit pixel on every run. */
    if(brightness == applied_brightness_) { return; }
    applied_brightness_ = brightness;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    /* Where the strip does all of the dimming, the master carries the setting and the cap at
       once, so the two are combined here rather than kept apart as applyCurrentLimit() does
       for the other path. */
    const byte limit = GetCurrentLimit();
    pixel_stripe_.SetBrightness((brightness < limit) ? brightness : limit);
    applied_current_limit_ = limit;
#else
    ApplyColor();
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
void Display::ApplyColor()
{
#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    color_.DimmColors(applied_brightness_);

    /* the pixels already on the display still carry the previous color */
    for(IndexType index = 0; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
        if(GetPixel(index)) { SetPixel(index); }
    }
#endif
} /* applyColor */


/******************************************************************************************************************************************************
  applyColorAndCurrentLimit()
*******************************************************************************************************************************************************/
/* The colour reaches the pixels, and then the cap is recomputed for it: white at the same
   brightness draws three times what amber does, so a colour change moves the current as much
   as a brightness change and must not wait for the next task run to be noticed. */
void Display::ApplyColorAndCurrentLimit()
{
    ApplyColor();
    ApplyCurrentLimit();
} /* applyColorAndCurrentLimit */


/******************************************************************************************************************************************************
  getNumberOfLitPixels()
*******************************************************************************************************************************************************/
/* Counted from the strip's buffer rather than kept as a running total beside it. The
   animations and setPixel() write there directly, so a counter would be the copy that
   falls behind - the same argument Persistence::gather() is built on. One pass over the
   display per task run, next to the pass applyColor() already makes. */
byte Display::GetNumberOfLitPixels() const
{
    static_assert(DISPLAY_NUMBER_OF_PIXELS <= 255u, "Display: a larger display needs a wider lit count");
    byte count{0u};

    for(IndexType index = 0u; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
        if(GetPixel(index)) { count++; }
    }
    return count;
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
byte Display::GetCurrentLimit() const
{
    const Pixel shown = color_.GetColorToShow();
    const uint16_t channel_sum = DisplayCurrentLimit::ToChannelSum(shown.GetRed(), shown.GetGreen(), shown.GetBlue());

    return DisplayCurrentLimit::ToBrightnessLimit(DISPLAY_NUMBER_OF_PIXELS, GetNumberOfLitPixels(), channel_sum);
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
void Display::ApplyCurrentLimit()
{
    /* Off stays off - the master is also what disable() uses. enable() asks for the cap again
       on its way back, so nothing is lost by leaving it alone here. */
    if(!enabled_) { return; }

    const byte limit = GetCurrentLimit();

    if(limit == applied_current_limit_) { return; }
    applied_current_limit_ = limit;
    pixel_stripe_.SetBrightness(limit);
} /* applyCurrentLimit */


/******************************************************************************************************************************************************
  resumeAtCurrentLimit()
******************************************************************************************************************************************************/
/* What enable() uses instead of enablePixels(). The remembered value is set from the same
   computation that is written, so the guard in applyCurrentLimit() agrees with the strip
   afterwards. */
void Display::ResumeAtCurrentLimit()
{
    applied_current_limit_ = GetCurrentLimit();
    pixel_stripe_.SetBrightness(applied_current_limit_);
} /* resumeAtCurrentLimit */

/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
StdReturnType Display::SetWord(WordIdType word_id, byte max_length)
{
    StdReturnType return_value{kENotOk};
    byte length;

    if(word_id < DisplayWords::kWordNumberOfWords) {
        return_value = kEOk;
        DisplayWord word = words_.GetDisplayWord(word_id);
        byte word_length = word.GetLength();

        if(max_length == kWordLengthUnlimited || max_length >= word_length) { length = word_length; }
        else { length = max_length; }

        for(IndexType index = 0; index < length; index++) {
            if(SetPixel(word.GetColumn() + index,  word.GetRow()) == kENotOk) return_value = kENotOk;
        }
    } else {
        return_value = kENotOk;
    }
    return return_value;
} /* setWord */


/******************************************************************************************************************************************************
  clearWord()
******************************************************************************************************************************************************/
StdReturnType Display::ClearWord(WordIdType word_id)
{
    StdReturnType return_value{kENotOk};

    if(word_id < DisplayWords::kWordNumberOfWords) {
        return_value = kEOk;
        DisplayWord word = words_.GetDisplayWord(word_id);

        for(IndexType index = 0; index < word.GetLength(); index++) {
            if(ClearPixel(word.GetColumn() + index,  word.GetRow()) == kENotOk) return_value = kENotOk;
        }
    } else {
        return_value = kENotOk;
    }
    return return_value;
} /* clearWord */


/******************************************************************************************************************************************************
  clearAllWords()
******************************************************************************************************************************************************/
StdReturnType Display::ClearWords()
{
    StdReturnType return_value{kEOk};

    for(byte i = DisplayWords::kWordEs; i < DisplayWords::kWordNumberOfWords; i++) {
        if(ClearWord((WordIdType) i) == kENotOk) return_value = kENotOk;
    }
    return return_value;
} /* clearAllWords */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::GetPixel(IndexType index, PixelValueType& value) const
{
    byte row, column;
    IndexToColumnAndRow(index, column, row);
    return GetPixel(column, row, value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
Display::PixelValueType Display::GetPixel(IndexType index) const
{
    byte row, column;
    IndexToColumnAndRow(index, column, row);
    return GetPixel(column, row);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
StdReturnType Display::GetPixel(byte column, byte row, PixelValueType& value)  const
{
    if(!IsColumnAndRowValid(column, row)) { return kENotOk; }

    value = GetPixel(column, row);
    return kEOk;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
Display::PixelValueType Display::GetPixel(byte column, byte row)  const
{
    if(!IsColumnAndRowValid(column, row)) { return false; }

    const Pixel pixel = pixel_stripe_.GetPixel(ToIndex(column, row));

    /* Pixel is only off when all colors are zero */
    return !(pixel.GetRed() == 0u && pixel.GetGreen() == 0u && pixel.GetBlue() == 0u);
} /* getPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::SetPixel(byte column, byte row)
{
    if(!IsColumnAndRowValid(column, row)) { return kENotOk; }

#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    return pixel_stripe_.SetPixel(ToIndex(column, row), color_);
#else
    return pixel_stripe_.SetPixel(ToIndex(column, row), color_.GetColorDimmed());
#endif
} /* setPixel */


/******************************************************************************************************************************************************
  getColorDimmed()
******************************************************************************************************************************************************/
Pixel Display::GetColorDimmed(byte brightness) const
{
    /* the display color stays the only color, the brightness just dimms it. It comes
       on top of the display brightness, which is already part of the color here. */
#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    Pixel pixel_color = color_.GetColor();
#else
    Pixel pixel_color = color_.GetColorDimmed();
#endif
    pixel_color.DimmPixel(brightness);

    return pixel_color;
} /* getColorDimmed */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::SetPixel(byte column, byte row, byte brightness)
{
    if(!IsColumnAndRowValid(column, row)) { return kENotOk; }

    return pixel_stripe_.SetPixel(ToIndex(column, row), GetColorDimmed(brightness));
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
StdReturnType Display::SetPixel(IndexType index)
{
    byte row, column;
    IndexToColumnAndRow(index, column, row);
    return SetPixel(column,  row);
} /* setPixel */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::ClearPixel(byte column, byte row)
{
    if(!IsColumnAndRowValid(column, row)) { return kENotOk; }

    return pixel_stripe_.ClearPixel(ToIndex(column, row));
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
StdReturnType Display::ClearPixel(IndexType index)
{
    byte row, column;
    IndexToColumnAndRow(index, column, row);
    return ClearPixel(column,  row);
} /* clearPixel */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
StdReturnType Display::TogglePixel(byte column, byte row)
{
    if(!IsColumnAndRowValid(column, row)) { return kENotOk; }

    /* The column and the row are what is toggled, not the index they map to: the serpentine
       branch used to transform them into an index and hand that to functions which transform
       it a second time, so on an odd row it toggled the pixel mirrored about the middle of
       the display. Nothing showed it, because nothing here is built with the serpentine
       wiring - and the same branch passed a pointer where a reference was expected, so it
       would not have compiled if anything had. */
    if(GetPixel(column, row)) { return ClearPixel(column, row); }
    else { return SetPixel(column, row); }
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
StdReturnType Display::TogglePixel(IndexType index)
{
    byte row, column;
    IndexToColumnAndRow(index, column, row);
    return TogglePixel(column,  row);
} /* togglePixel */


/******************************************************************************************************************************************************
  getPixelRow()
******************************************************************************************************************************************************/
StdReturnType Display::GetPixelRow(byte row, PixelRowType& pixel_row) const
{
    StdReturnType return_value{kEOk};
    PixelValueType pixel_value;

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(GetPixel(column, row, pixel_value) == kEOk) {
            WRITE_BIT(pixel_row, column, pixel_value);
        } else {
            return_value = kENotOk;
        }
    }
    return return_value;
} /* getPixelRow */


/******************************************************************************************************************************************************
  getPixelRow()
******************************************************************************************************************************************************/
Display::PixelRowType Display::GetPixelRow(byte row)  const
{
    PixelRowType pixel_row{0};

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        WRITE_BIT(pixel_row, column, GetPixel(column, row));
    }
    return pixel_row;
} /* getPixelRow */


/******************************************************************************************************************************************************
  getPixelColumn()
******************************************************************************************************************************************************/
StdReturnType Display::GetPixelColumn(byte column, PixelRowType& pixel_column)  const
{
    StdReturnType return_value{kEOk};
    PixelValueType pixel_value;

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(GetPixel(column, row, pixel_value) == kEOk) {
            WRITE_BIT(pixel_column, row, pixel_value);
        } else {
            return_value = kENotOk;
        }
    }
    return return_value;
} /* getPixelColumn */


/******************************************************************************************************************************************************
  getPixelColumn()
******************************************************************************************************************************************************/
Display::PixelColumnType Display::GetPixelColumn(byte column)  const
{
    Display::PixelColumnType pixel_column{0};

    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        WRITE_BIT(pixel_column, row, GetPixel(column, row));
    }
    return pixel_column;
} /* getPixelColumn */


/******************************************************************************************************************************************************
  setPixelRow()
******************************************************************************************************************************************************/
StdReturnType Display::SetPixelRow(byte row, PixelRowType pixel_row)
{
    StdReturnType return_value{kEOk};

    for(byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
        if(WritePixel(column, row, READ_BIT(pixel_row, column)) == kENotOk) return_value = kENotOk;
    }
    return return_value;
} /* setPixelRow */


/******************************************************************************************************************************************************
  setPixelColumn()
******************************************************************************************************************************************************/
StdReturnType Display::SetPixelColumn(byte column, PixelRowType pixel_column)
{
    StdReturnType return_value{kEOk};
    for(byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
        if(WritePixel(column, row, READ_BIT(pixel_column, row)) == kENotOk) return_value = kENotOk;
    }
    return return_value;
} /* setPixelColumn */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of Display
******************************************************************************************************************************************************/
Display::Display(Pixel color) : pixel_stripe_{Pixels::GetInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
: pixel_stripe_()
#endif
{
    state_ = kStateUninit;
    color_.SetColor(color);

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    brightness_.SetBrightness(255u);
#endif
} /* Display */


/******************************************************************************************************************************************************
  Constructor of Display
******************************************************************************************************************************************************/
Display::Display(ColorType red, ColorType green, ColorType blue) : pixel_stripe_{Pixels::GetInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
: pixel_stripe_()
#endif
{
    color_.SetColorRed(red);
    color_.SetColorGreen(green);
    color_.SetColorBlue(blue);
    state_ = kStateUninit;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    brightness_.SetBrightness(255u);
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
byte Display::TransformToSerpentine(byte column, byte row)
{
    byte index;

    if(IS_BIT_CLEARED(row, 0)) index = (row * DISPLAY_NUMBER_OF_COLUMNS) + column;
    else index = (row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - column - 1);

    return index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
byte Display::TransformToSerpentine(IndexType index)
{
    byte column = IndexToColumn(index);
    byte row = IndexToRow(index);

    return TransformToSerpentine(column, row);
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
