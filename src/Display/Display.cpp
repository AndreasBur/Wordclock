// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the display does itself: brightness, colour, current limit, words and pixels.
 */

#include "Display.h"
#include "Bit.h"
#include "DisplayWord.h"
#include "Pixel.h"

void Display::Init() {
  Clear();
  pixel_stripe_.Init(DISPLAY_DATA_PIN);
  state_ = StateType::kInit;
}

void Display::SetBrightness(byte brightness) {
  /* only the wish is stored here, what reaches the LEDs is CalcBrightness() */
  brightness_.SetBrightness(brightness);
  ApplyBrightness();
}

void Display::ApplyBrightness() {
  /*
   * Before the shortcut below and on every run, because what the cap depends on moves
   * without the setting moving: a clock face that lights more letters than the last one
   * draws more current at the same brightness. This task is the only place that notices.
   */
  ApplyCurrentLimit();

  byte const brightness = brightness_.CalcBrightness();

  /*
   * Nothing to do while the calculated value stays put, which is the normal case: only
   * the automatic makes it move on its own. Without this check the task would rewrite
   * every lit pixel on every run.
   */
  if (brightness == applied_brightness_) {
    return;
  }
  applied_brightness_ = brightness;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
  /*
   * Where the strip does all of the dimming, the master carries the setting and the cap at
   * once, so the two are combined here rather than kept apart as ApplyCurrentLimit() does
   * for the other path.
   */
  byte const limit = GetCurrentLimit();
  pixel_stripe_.SetBrightness((brightness < limit) ? brightness : limit);
  applied_current_limit_ = limit;
#else
  ApplyColor();
#endif
}

/*
 * The dimmed color is derived from the color and the brightness, and what the pixels carry
 * is the dimmed one - so a write to either invalidates both. ApplyBrightness() above has
 * always done this for its own half; the color setters did not, which left a color change
 * invisible until the brightness happened to move, redraw or no redraw.
 *
 * Where the strip does the dimming there is no dimmed copy to derive, but the pixels still
 * carry the previous color. This used to do nothing there, on the reasoning that the next
 * redraw would pick the new one up - and nothing redraws a clock face between two minutes,
 * so a color change, and every step of the color cycle, waited for the time to move.
 */
void Display::ApplyColor() {
#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
  color_.DimmColors(applied_brightness_);
#endif

  /* the pixels already on the display still carry the previous color */
  for (IndexType index = 0; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
    if (GetPixel(index)) {
      SetPixel(index);
    }
  }
}

/*
 * The colour reaches the pixels, and then the cap is recomputed for it: white at the same
 * brightness draws three times what amber does, so a colour change moves the current as much
 * as a brightness change and must not wait for the next task run to be noticed.
 */
void Display::ApplyColorAndCurrentLimit() {
  ApplyColor();
  ApplyCurrentLimit();
}

/*
 * Counted from the strip's buffer rather than kept as a running total beside it. The
 * animations and SetPixel() write there directly, so a counter would be the copy that
 * falls behind - the same argument Persistence::Gather() is built on. One pass over the
 * display per task run, next to the pass ApplyColor() already makes.
 */
byte Display::GetNumberOfLitPixels() const {
  static_assert(DISPLAY_NUMBER_OF_PIXELS <= 255u,
                "Display: a larger display needs a wider lit count");
  byte count{0u};

  for (IndexType index = 0u; index < DISPLAY_NUMBER_OF_PIXELS; index++) {
    if (GetPixel(index)) {
      count++;
    }
  }
  return count;
}

/*
 * Built from the colour and a count rather than from the pixel values, and that is not a
 * shortcut: which dimming path is compiled decides whether the buffer holds the dimmed colour
 * or the plain one, and only the plain one says what the strip would draw at full brightness.
 * Every lit pixel carries the same colour, so a count is all the buffer has to give.
 *
 * An upper bound on purpose. SetPixel() with its own brightness leaves a pixel dimmer than
 * the colour, and a limiter that guessed low would be one that does not protect.
 *
 * The colour it asks for is the one being *shown*, not the one that was set: with the colour
 * cycle running those are different, and reading the setting would let a saturated hue on the
 * strip be budgeted as whatever pale colour happens to be stored underneath it - which is a
 * guess in the one direction this must never guess.
 */
byte Display::GetCurrentLimit() const {
  Pixel const shown = color_.GetColorToShow();
  uint16_t const channel_sum =
      DisplayCurrentLimit::ToChannelSum(shown.GetRed(), shown.GetGreen(), shown.GetBlue());

  return DisplayCurrentLimit::ToBrightnessLimit(DISPLAY_NUMBER_OF_PIXELS, GetNumberOfLitPixels(),
                                                channel_sum);
}

/*
 * The cap rides on the strip's own master rather than on the colour written into the buffer,
 * and that placement is the whole reason this is cheap. The buffer keeps the colour somebody
 * asked for at the brightness they asked for; the master says how much of it the supply lets
 * out. So a cap that moves needs no redraw, and a clock face restored after a full display
 * comes back at the brightness it had rather than at the one the full display was held to.
 *
 * Writing the master marks the frame dirty, which is why the remembered value is compared
 * first: without that, every task run would queue a transmission that changes nothing.
 */
void Display::ApplyCurrentLimit() {
  // Off stays off - the master is also what Disable() uses. Enable() asks for the cap again
  // on its way back, so nothing is lost by leaving it alone here.
  if (!enabled_) {
    return;
  }

  byte const limit = GetCurrentLimit();

  if (limit == applied_current_limit_) {
    return;
  }
  applied_current_limit_ = limit;
  pixel_stripe_.SetBrightness(limit);
}

/*
 * What Enable() uses instead of EnablePixels(). The remembered value is set from the same
 * computation that is written, so the guard in ApplyCurrentLimit() agrees with the strip
 * afterwards.
 */
void Display::ResumeAtCurrentLimit() {
  applied_current_limit_ = GetCurrentLimit();
  pixel_stripe_.SetBrightness(applied_current_limit_);
}

StdReturnType Display::SetWord(WordIdType word_id, byte max_length) {
  StdReturnType return_value{kENotOk};
  byte length;

  if (word_id < WordIdType::kNumberOfWords) {
    return_value = kEOk;
    DisplayWord word = words_.GetDisplayWord(word_id);
    byte word_length = word.GetLength();

    if (max_length == kWordLengthUnlimited || max_length >= word_length) {
      length = word_length;
    } else {
      length = max_length;
    }

    for (IndexType index = 0; index < length; index++) {
      if (SetPixel(word.GetColumn() + index, word.GetRow()) == kENotOk) return_value = kENotOk;
    }
  } else {
    return_value = kENotOk;
  }
  return return_value;
}

StdReturnType Display::ClearWord(WordIdType word_id) {
  StdReturnType return_value{kENotOk};

  if (word_id < WordIdType::kNumberOfWords) {
    return_value = kEOk;
    DisplayWord word = words_.GetDisplayWord(word_id);

    for (IndexType index = 0; index < word.GetLength(); index++) {
      if (ClearPixel(word.GetColumn() + index, word.GetRow()) == kENotOk) return_value = kENotOk;
    }
  } else {
    return_value = kENotOk;
  }
  return return_value;
}

StdReturnType Display::ClearWords() {
  StdReturnType return_value{kEOk};

  for (byte i = ToUnderlying(WordIdType::kEs); i < DisplayWords::kNumberOfWords; i++) {
    if (ClearWord(static_cast<WordIdType>(i)) == kENotOk) return_value = kENotOk;
  }
  return return_value;
}

StdReturnType Display::GetPixel(IndexType index, PixelValueType& value) const {
  byte row, column;
  IndexToColumnAndRow(index, column, row);
  return GetPixel(column, row, value);
}

Display::PixelValueType Display::GetPixel(IndexType index) const {
  byte row, column;
  IndexToColumnAndRow(index, column, row);
  return GetPixel(column, row);
}

StdReturnType Display::GetPixel(byte column, byte row, PixelValueType& value) const {
  if (!IsColumnAndRowValid(column, row)) {
    return kENotOk;
  }

  value = GetPixel(column, row);
  return kEOk;
}

Display::PixelValueType Display::GetPixel(byte column, byte row) const {
  if (!IsColumnAndRowValid(column, row)) {
    return false;
  }

  Pixel const pixel = pixel_stripe_.GetPixel(ToIndex(column, row));

  /* Pixel is only off when all colors are zero */
  return !(pixel.GetRed() == 0u && pixel.GetGreen() == 0u && pixel.GetBlue() == 0u);
}

StdReturnType Display::SetPixel(byte column, byte row) {
  if (!IsColumnAndRowValid(column, row)) {
    return kENotOk;
  }

  return pixel_stripe_.SetPixel(ToIndex(column, row), GetColorToWrite());
}

Pixel Display::GetColorDimmed(byte brightness) const {
  // the display color stays the only color, the brightness just dimms it. It comes
  // on top of the display brightness, which is already part of the color here.
  Pixel pixel_color = GetColorToWrite();
  pixel_color.DimmPixel(brightness);

  return pixel_color;
}

StdReturnType Display::SetPixel(byte column, byte row, byte brightness) {
  if (!IsColumnAndRowValid(column, row)) {
    return kENotOk;
  }

  return pixel_stripe_.SetPixel(ToIndex(column, row), GetColorDimmed(brightness));
}

StdReturnType Display::SetPixel(IndexType index) {
  byte row, column;
  IndexToColumnAndRow(index, column, row);
  return SetPixel(column, row);
}

StdReturnType Display::ClearPixel(byte column, byte row) {
  if (!IsColumnAndRowValid(column, row)) {
    return kENotOk;
  }

  return pixel_stripe_.ClearPixel(ToIndex(column, row));
}

StdReturnType Display::ClearPixel(IndexType index) {
  byte row, column;
  IndexToColumnAndRow(index, column, row);
  return ClearPixel(column, row);
}

StdReturnType Display::TogglePixel(byte column, byte row) {
  if (!IsColumnAndRowValid(column, row)) {
    return kENotOk;
  }

  /*
   * The column and the row are what is toggled, not the index they map to: the serpentine
   * branch used to transform them into an index and hand that to functions which transform
   * it a second time, so on an odd row it toggled the pixel mirrored about the middle of
   * the display. Nothing showed it, because nothing here is built with the serpentine
   * wiring - and the same branch passed a pointer where a reference was expected, so it
   * would not have compiled if anything had.
   */
  if (GetPixel(column, row)) {
    return ClearPixel(column, row);
  } else {
    return SetPixel(column, row);
  }
}

StdReturnType Display::TogglePixel(IndexType index) {
  byte row, column;
  IndexToColumnAndRow(index, column, row);
  return TogglePixel(column, row);
}

StdReturnType Display::GetPixelRow(byte row, PixelRowType& pixel_row) const {
  StdReturnType return_value{kEOk};
  PixelValueType pixel_value;

  for (byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
    if (GetPixel(column, row, pixel_value) == kEOk) {
      pixel_row = Bit<PixelRowType>::WriteBit(pixel_row, column, pixel_value);
    } else {
      return_value = kENotOk;
    }
  }
  return return_value;
}

Display::PixelRowType Display::GetPixelRow(byte row) const {
  Bit<PixelRowType> pixel_row;

  for (byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
    pixel_row[column] = GetPixel(column, row);
  }
  return pixel_row.GetValue();
}

StdReturnType Display::GetPixelColumn(byte column, PixelColumnType& pixel_column) const {
  StdReturnType return_value{kEOk};
  PixelValueType pixel_value;

  for (byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
    if (GetPixel(column, row, pixel_value) == kEOk) {
      pixel_column = Bit<PixelColumnType>::WriteBit(pixel_column, row, pixel_value);
    } else {
      return_value = kENotOk;
    }
  }
  return return_value;
}

Display::PixelColumnType Display::GetPixelColumn(byte column) const {
  Bit<PixelColumnType> pixel_column;

  for (byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
    pixel_column[row] = GetPixel(column, row);
  }
  return pixel_column.GetValue();
}

StdReturnType Display::SetPixelRow(byte row, PixelRowType pixel_row) {
  StdReturnType return_value{kEOk};

  for (byte column = 0; column < DISPLAY_NUMBER_OF_COLUMNS; column++) {
    if (WritePixel(column, row, Bit<PixelRowType>::ReadBit(pixel_row, column)) == kENotOk)
      return_value = kENotOk;
  }
  return return_value;
}

StdReturnType Display::SetPixelColumn(byte column, PixelRowType pixel_column) {
  StdReturnType return_value{kEOk};
  for (byte row = 0; row < DISPLAY_NUMBER_OF_ROWS; row++) {
    if (WritePixel(column, row, Bit<PixelRowType>::ReadBit(pixel_column, row)) == kENotOk)
      return_value = kENotOk;
  }
  return return_value;
}

Display::Display(Pixel color)
    : pixel_stripe_{Pixels::GetInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
    : pixel_stripe_()
#endif
{
  state_ = StateType::kUninit;
  color_.SetColor(color);

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
  brightness_.SetBrightness(255u);
#endif
}

Display::Display(ColorType red, ColorType green, ColorType blue)
    : pixel_stripe_{Pixels::GetInstance()}
#if (PIXELS_IS_SINGLETON == STD_OFF)
    : pixel_stripe_()
#endif
{
  color_.SetColorRed(red);
  color_.SetColorGreen(green);
  color_.SetColorBlue(blue);
  state_ = StateType::kUninit;

#if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
  brightness_.SetBrightness(255u);
#endif
}

byte Display::TransformToSerpentine(byte column, byte row) {
  byte index;

  if (!Bit<byte>::ReadBit(row, 0u))
    index = static_cast<byte>((row * DISPLAY_NUMBER_OF_COLUMNS) + column);
  else
    index = static_cast<byte>((row * DISPLAY_NUMBER_OF_COLUMNS) +
                              (DISPLAY_NUMBER_OF_COLUMNS - column - 1u));

  return index;
}

byte Display::TransformToSerpentine(IndexType index) {
  byte column = IndexToColumn(index);
  byte row = IndexToRow(index);

  return TransformToSerpentine(column, row);
}
