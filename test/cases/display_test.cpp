// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

// What reaches the pixels: the word-set latch, the fade, the colour channels and the letter table.

#include "Display.h"

#include "Animations.h"
#include "Clock.h"
#include "DisplayCharacters.h"
#include "DisplayColor.h"
#include "DisplayManager.h"
#include "DisplayWords.h"
#include "GammaCorrection.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "test/cases.h"
#include "test/check.h"

// Only where the colour does the dimming: where the strip does it, DisplayColor keeps no
// dimmed copy, and what this would check is the strip's master.
void TestColourDimmingAcrossByteRange() {
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
  DisplayColor color;
  bool all_match = true;
  for (unsigned channel = 0u; channel <= 255u; ++channel) {
    byte const level = static_cast<byte>(channel);
    color.SetColor(Pixel(level, level, level));
    for (unsigned brightness = 0u; brightness <= 255u; ++brightness) {
      color.DimmColors(static_cast<byte>(brightness));
      uint32_t const scaled = (static_cast<uint32_t>(channel) * brightness) / 256u;
      byte const expected =
          (channel == 0u) ? 0u : ((scaled == 0u) ? 1u : static_cast<byte>(scaled));
      all_match = all_match && color.GetColorRedDimmed() == expected &&
                  color.GetColorGreenDimmed() == expected && color.GetColorBlueDimmed() == expected;
    }
  }
  Expect(all_match,
         "dimming must scale every byte pair without overflow and retain the minimum level");
  Expect(color.GetColorRed() == 255u, "dimming must preserve the configured colour");

  /* The case the loop above walks only on the diagonal: a colour with a channel at 0. */
  color.SetColor(Pixel(255u, 0u, 0u));
  color.DimmColors(0u);
  Expect(color.GetColorRedDimmed() == 1u,
         "a pure red at the lowest brightness must keep its red at the minimum");
  Expect(color.GetColorGreenDimmed() == 0u && color.GetColorBlueDimmed() == 0u,
         "and must not gain the green and blue it does not have");
  color.DimmColors(128u);
  Expect(color.GetColorGreenDimmed() == 0u && color.GetColorBlueDimmed() == 0u,
         "at any brightness");
#endif
}

/*
 * The curve's ends, its shape and a value from each end of the shift range. The middle
 * ones are what the construction gives directly: one stored value, shifted once per
 * doubling below the top.
 */
void TestGammaCorrectionCurve() {
  Expect(GammaCorrection::CalcCorrectedValue(0u) == 0u,
         "a setting of 0 must stay 0, not the faintest light");
  Expect(GammaCorrection::CalcCorrectedValue(1u) == 1u,
         "the first step above 0 must be the faintest light there is");
  Expect(GammaCorrection::CalcCorrectedValue(255u) == 255u, "the full setting must stay full");

  bool rising = true;
  for (unsigned value = 1u; value <= 255u; ++value) {
    rising = rising && GammaCorrection::CalcCorrectedValue(static_cast<byte>(value - 1u)) <=
                           GammaCorrection::CalcCorrectedValue(static_cast<byte>(value));
  }
  Expect(rising, "the curve must never fall, not even where one doubling hands over to the next");

  Expect(GammaCorrection::CalcCorrectedValue(31u) == 1u &&
             GammaCorrection::CalcCorrectedValue(32u) == 2u,
         "the first doubling must end at setting 31 and the second begin at 32");
  Expect(GammaCorrection::CalcCorrectedValue(128u) == 16u,
         "half the setting must be 133 shifted down three doublings");
  Expect(GammaCorrection::CalcCorrectedValue(254u) == 255u,
         "the last step must be the top of the stored doubling, unshifted");
}

// How much light the whole display is putting out, which is what a fade changes and a
// redraw does not.
uint32_t TotalIntensity(PixelBufferType const& pixels) {
  uint32_t total = 0u;

  for (byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
    total += pixels[index].GetRed();
    total += pixels[index].GetGreen();
    total += pixels[index].GetBlue();
  }
  return total;
}

/*
 * The latch itself: the display is redrawn when the word set changes and left alone
 * when it does not. Pixels::IsDirty() is what makes "left alone" observable at all - the
 * buffer staying equal would not tell a skipped redraw from one that rewrote the same
 * words. Must run before any other test touches DisplayManager.
 */
void TestDisplayManagerLatch() {
  Clock::GetInstance().SetMode(Clock::ModeType::kWessi);
  // Without an animation the word change draws straight to the buffer, so what the
  // latch decided is visible in the same task.
  Animations::GetInstance().SetMode(Animations::ModeType::kFixed);
  Animations::GetInstance().SetAnimation(Animations::AnimationIdType::kNone);

  Pixels& pixels = Pixels::GetInstance();
  DisplayManager& display_manager = DisplayManager::GetInstance();

  SetTime(10u, 4u, 0u);
  pixels.ClearDirty();
  display_manager.Task();
  Expect(pixels.IsDirty(), "the first task must draw the time it finds");

  PixelBufferType const after_first_task = ReadPixels();
  Expect(IsAnyPixelLit(after_first_task), "the first task must light some letters");

  /* Same five-minute step, one task later. */
  SetTime(10u, 4u, 30u);
  pixels.ClearDirty();
  display_manager.Task();
  Expect(!pixels.IsDirty(), "an unchanged word set must not redraw");
  Expect(ArePixelsEqual(ReadPixels(), after_first_task),
         "an unchanged word set must leave the letters as they were");

  SetTime(10u, 5u, 0u);
  pixels.ClearDirty();
  display_manager.Task();
  Expect(pixels.IsDirty(), "a word change must redraw");
  Expect(!ArePixelsEqual(ReadPixels(), after_first_task), "a word change must reach the letters");

  /* And back to standing still on the new step. */
  PixelBufferType const after_change = ReadPixels();
  SetTime(10u, 9u, 59u);
  pixels.ClearDirty();
  display_manager.Task();
  Expect(!pixels.IsDirty(), "the rest of the step must not redraw either");
  Expect(ArePixelsEqual(ReadPixels(), after_change),
         "the rest of the step must leave the letters as they were");
}

/*
 * The fade's own promise, which the display it leaves behind cannot show: that it dims on
 * the way. It used to count a brightness nobody applied - the variable went down and up
 * and reached no pixel - so the animation was a pause with a hard change at the end of it.
 * The other half of this is what it must not touch: the brightness setting, which is what
 * Persistence writes to the store. A fade that dimmed by turning the setting down would
 * save its own half-way value as what the user asked for.
 */
void TestFadeDimsAndComesBack() {
  Animations& animations = Animations::GetInstance();
  Display& display = Display::GetInstance();

  PixelBufferType const target = DrawClockFace(10u, 35u);
  byte const brightness_before = display.GetBrightness();

  DrawClockFace(10u, 4u);
  uint32_t const intensity_before = TotalIntensity(ReadOutputPixels());

  animations.SetAnimation(Animations::AnimationIdType::kFade);
  animations.SetTime(10u, 35u);

  // Far enough in to be measurable, far short of the swap: the level counts down one per
  // task from 255.
  for (int tick = 0; tick < 64; tick++) {
    animations.Task(true);
  }
  /*
   * At the output, because that is where the dimming is in both configurations: in the
   * colour's dimmed copy, or in the strip's master where the strip does the dimming and the
   * buffer keeps every colour at full strength.
   */
  Expect(TotalIntensity(ReadOutputPixels()) < intensity_before,
         "the fade must dim what is on the display");

  int ticks = 0;
  while ((animations.GetState() == Animations::StateType::kPending) && (ticks < 5000)) {
    animations.Task(true);
    ticks++;
  }

  Expect(ArePixelsEqual(ReadPixels(), target), "the fade must come back to full brightness");
  Expect(display.GetBrightness() == brightness_before,
         "the fade must leave the brightness setting alone");

  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

/*
 * Switching the display off and on again, seen where it happens: the output rather than
 * the buffer. disable() leaves every colour in the buffer where it was and takes the
 * brightness to zero, so a check on the buffer alone would call a dark clock lit.
 */
void TestDisplayOffAndOnAgain() {
  Display& display = Display::GetInstance();

  // Switched on first rather than assumed on: the flicker animation leaves it off, and
  // until this test there was nothing that could notice.
  display.Enable();
  display.SetBrightness(120u);
  Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
  Expect(display.Show() == kEOk, "the clock face must reach the strip");
  Expect(IsAnyOutputPixelLit(), "a display that is on must put something out");

  PixelBufferType const lit = ReadPixels();

  display.Disable();
  Expect(!IsAnyOutputPixelLit(), "a display that is off must put nothing out");
  Expect(ArePixelsEqual(ReadPixels(), lit),
         "and must leave the buffer alone, so switching back on needs no redraw");

  display.Enable();
  Expect(IsAnyOutputPixelLit(), "switching back on must put something out again");
  Expect(display.GetBrightness() == 120u,
         "and must come back on the brightness that was set, not on full");
}

/*
 * What "one further" means for a switch: the other one. The toggle is the whole of what a
 * button on the case can send, so it has to agree with the enable and disable a phone
 * sends - a toggle that only tracked its own calls would drift apart from ids 3 and 4 the
 * first time both were used.
 */
void TestDisplayToggleFollowsTheState() {
  Display& display = Display::GetInstance();
  byte const brightness_before = display.GetBrightness();

  display.Enable();
  display.SetBrightness(120u);
  Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
  display.Show();
  Expect(display.IsEnabled(), "a display that was enabled must say so");

  display.Toggle();
  Expect(!display.IsEnabled(), "a toggle must switch an enabled display off");
  Expect(!IsAnyOutputPixelLit(), "and must reach the strip, not only the flag");

  display.Toggle();
  Expect(display.IsEnabled(), "a second toggle must switch it back on");
  Expect(IsAnyOutputPixelLit(), "and must put something out again");

  /* The other direction: a phone switching it off, a knob switching it back on. */
  display.Disable();
  Expect(!display.IsEnabled(), "disable must be visible to the toggle too");
  display.Toggle();
  Expect(display.IsEnabled() && IsAnyOutputPixelLit(),
         "a toggle after a disable must switch the display on");

  display.SetBrightness(brightness_before);
}

/*
 * The two settings that are a yes or a no, turned round by a control that cannot read
 * them. Both go through the setters that apply what they set, so this also says that a
 * toggle does not need a task to take effect.
 */
void TestBrightnessTogglesInvertTheSetting() {
  Display& display = Display::GetInstance();

  display.SetBrightnessUseAutomatic(false);
  display.ToggleBrightnessAutomatic();
  Expect(display.GetBrightnessUseAutomatic(), "a toggle must switch the automatic on");
  display.ToggleBrightnessAutomatic();
  Expect(!display.GetBrightnessUseAutomatic(), "and the next one must switch it off again");

  display.SetBrightnessUseGammaCorrection(false);
  display.ToggleBrightnessGammaCorrection();
  Expect(display.GetBrightnessUseGammaCorrection(), "a toggle must switch the gamma correction on");
  display.ToggleBrightnessGammaCorrection();
  Expect(!display.GetBrightnessUseGammaCorrection(), "and the next one must switch it off again");
}

/*
 * What the colour reset needs to be worth having, and what it did not have: a colour that
 * reaches the strip. The pixels carry the colour already dimmed by the brightness, and that
 * dimmed copy used to be recomputed only when the brightness moved - so every colour change,
 * the reset and the six increment procedures alike, stayed invisible until something else
 * happened to touch the brightness. A redraw did not help: it wrote the same stale copy.
 */
void TestColourChangesReachTheStrip() {
  Display& display = Display::GetInstance();
  byte const brightness_before = display.GetBrightness();

  display.Enable();
  display.SetBrightness(200u);
  display.ResetColor();
  PixelBufferType const white = DrawClockFace(10u, 5u);
  Expect(IsAnyPixelLit(white), "a clock face must light some letters to compare");

  /* No brightness change anywhere between here and the check, which is the whole point. */
  display.SetColor(255u, 0u, 0u);
  Expect(!ArePixelsEqual(ReadPixels(), white),
         "a colour change must reach the letters that are already lit");

  // Red, and the other two channels as far down as they go: DisplayColor's dimming
  // rounds a channel up to one rather than to nothing, so "off" is a one here.
  PixelBufferType const red = ReadPixels();
  bool any_lit_pixel_is_red = false;
  bool every_lit_pixel_is_red = true;
  for (byte index = 0u; index < PIXELS_NUMBER_OF_PIXELS; index++) {
    if (red[index].GetRed() <= 1u) {
      continue;
    }

    any_lit_pixel_is_red = true;
    if (red[index].GetGreen() > 1u || red[index].GetBlue() > 1u) {
      every_lit_pixel_is_red = false;
    }
  }
  Expect(any_lit_pixel_is_red && every_lit_pixel_is_red,
         "a red display must have nothing but the rounding left in the other two channels");

  display.ResetColor();
  Expect(ArePixelsEqual(ReadPixels(), white),
         "the colour reset must put white back, on the strip and not only in the setting");
  Expect(display.GetColorRed() == 255u && display.GetColorGreen() == 255u &&
             display.GetColorBlue() == 255u,
         "and must reach all three channels");

  display.SetBrightness(brightness_before);
}

/*
 * Guards a swap that this simulator cannot show: Pixel keeps its channels in the WS2812
 * wire order rather than in argument order, so a constructor that filled the raw array
 * positionally put red where GetRed() does not look. Every colour built that way so far
 * was grey or black, which hides it, and the window renders brightness rather than hue,
 * which hides it again - it would first have appeared on the strip.
 */
void TestPixelColorChannels() {
  Pixel const color(10u, 20u, 30u);

  Expect(color.GetRed() == 10u, "the first constructor argument must be red");
  Expect(color.GetGreen() == 20u, "the second constructor argument must be green");
  Expect(color.GetBlue() == 30u, "the third constructor argument must be blue");

  Pixel assigned;
  assigned.SetPixel(10u, 20u, 30u);
  Expect(assigned.GetRed() == color.GetRed() && assigned.GetGreen() == color.GetGreen() &&
             assigned.GetBlue() == color.GetBlue(),
         "setPixel and the constructor must place the channels alike");

  Pixel individual;
  individual.SetRed(10u);
  individual.SetGreen(20u);
  individual.SetBlue(30u);
  Expect(individual.GetRed() == color.GetRed() && individual.GetGreen() == color.GetGreen() &&
             individual.GetBlue() == color.GetBlue(),
         "the per-channel setters must place the channels alike");
}

/*
 * The letter table has four entry points that all have to agree, and until now they did
 * not: two of them indexed the table transposed, which for the last column read past its
 * end. Nothing called any of them, so the simulator kept a second copy of the letters
 * instead - which is the duplication this pins down as unnecessary.
 */
void TestDisplayCharacterLookup() {
  DisplayCharacters const characters;

  /* First row is "ESKISTLF" then U with an umlaut, "NF". */
  Expect(characters.GetCharacter(0u) == 'E', "index 0 must be the first letter of the first row");
  Expect(characters.GetCharacter(2u) == 'K', "index 2 must be the third letter of the first row");
  Expect(characters.GetCharacter(0u, 0u) == 'E', "column 0, row 0 must be the first letter");

  /* Column 10 is the one a transposed lookup read past the end of the table for. */
  Expect(characters.GetCharacter(10u, 0u) == 'F',
         "the last column of the first row must be reachable");
  Expect(characters.GetCharacter(10u) == 'F', "index 10 must be the last letter of the first row");

  /* Last row is "BSECHSFMUHR", so the very last letter is its R. */
  constexpr byte kLastIndex{DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS - 1u};
  Expect(characters.GetCharacter(kLastIndex) == 'R', "the last index must be the last letter");
  Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS - 1u,
                                 DISPLAY_CHARACTERS_NUMBER_OF_ROWS - 1u) == 'R',
         "the last column of the last row must be the last letter");

  // Every position has to read the same whether it is asked for by index or by column
  // and row - that is what the two broken entry points disagreed on.
  bool all_agree = true;
  for (byte index = 0u; index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; index++) {
    byte column, row;
    characters.IndexToColumnAndRow(index, column, row);

    if (characters.GetCharacter(index) != characters.GetCharacter(column, row)) {
      all_agree = false;
    }
    if (characters.ColumnAndRowToIndex(column, row) != index) {
      all_agree = false;
    }
  }
  Expect(all_agree, "index and column/row lookups must agree on every position");

  char character{'\0'};
  Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u, character) == kENotOk,
         "a column past the last one must be rejected");
  Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS, character) == kENotOk,
         "an index past the last one must be rejected");
}

namespace {
// Whether the pixel at `column` and `row` is a letter of some word, which is what ClearWords()
// turns off and what it has to leave alone otherwise.
bool IsPixelOfAWord(byte column, byte row) {
  DisplayWords const words;

  for (byte id = ToUnderlying(DisplayWords::WordIdType::kEs); id < DisplayWords::kNumberOfWords;
       id++) {
    DisplayWord const word = words.GetDisplayWord(static_cast<DisplayWords::WordIdType>(id));
    if (word.GetRow() == row && column >= word.GetColumn() &&
        column < word.GetColumn() + word.GetLength()) {
      return true;
    }
  }
  return false;
}

// How many letters of the word are lit: all of them for a word that is shown, none for one
// that is cleared, and anything in between for a clear that stopped half-way.
byte LitLettersOf(DisplayWords::WordIdType word_id) {
  DisplayWord const word = DisplayWords::GetDisplayWord(word_id);
  byte lit = 0u;

  for (byte letter = 0u; letter < word.GetLength(); letter++) {
    if (Display::GetInstance().GetPixel(static_cast<byte>(word.GetColumn() + letter),
                                        word.GetRow())) {
      lit++;
    }
  }
  return lit;
}
}  // namespace

/*
 * The writers and readers of the display that nothing in the firmware calls: they are kept, as
 * the rest of its interface is, so they are run here.
 */
void TestWordsAreClearedOneOrAll() {
  Display& display = Display::GetInstance();

  // A letter that belongs to no word, the first one there is.
  byte stray_column = 0u;
  byte stray_row = 0u;
  while (IsPixelOfAWord(stray_column, stray_row)) {
    if (++stray_column == DISPLAY_NUMBER_OF_COLUMNS) {
      stray_column = 0u;
      stray_row++;
    }
  }
  Expect(stray_row < DISPLAY_NUMBER_OF_ROWS, "the plate must have a letter that is in no word");

  display.Clear();
  display.SetWord(DisplayWords::WordIdType::kEs);
  display.SetWord(DisplayWords::WordIdType::kIst);
  display.SetPixel(stray_column, stray_row);

  Expect(display.ClearWord(DisplayWords::WordIdType::kEs) == kEOk, "a word must be cleared");
  Expect(LitLettersOf(DisplayWords::WordIdType::kEs) == 0u, "every letter of it");
  Expect(LitLettersOf(DisplayWords::WordIdType::kIst) ==
             DisplayWords::GetDisplayWordLength(DisplayWords::WordIdType::kIst),
         "and nothing of the word beside it");

  PixelBufferType const before = ReadPixels();
  Expect(display.ClearWord(DisplayWords::WordIdType::kNumberOfWords) == kENotOk,
         "a word not in the table must be refused");
  Expect(display.ClearWord(DisplayWords::WordIdType::kNone) == kEOk,
         "and no word cleared as nothing");
  Expect(ArePixelsEqual(before, ReadPixels()), "neither of them turning anything off");

  Expect(display.ClearWords() == kEOk, "every word must be cleared at once");
  Expect(LitLettersOf(DisplayWords::WordIdType::kIst) == 0u, "the one left among them");
  Expect(display.GetPixel(stray_column, stray_row), "while a letter of no word stays lit");

  display.Clear();
}

void TestPixelRowsAndColumnsAreWritten() {
  Display& display = Display::GetInstance();
  Display::PixelRowType const row_bits =
      (1u << 0u) | (1u << 5u) | (1u << (DISPLAY_NUMBER_OF_COLUMNS - 1u));
  Display::PixelColumnType const column_bits = (1u << 1u) | (1u << (DISPLAY_NUMBER_OF_ROWS - 1u));

  // A pixel lit in the row that the bits leave dark, and one in the row above that is no
  // business of the write.
  display.Clear();
  display.SetPixel(2u, 3u);
  display.SetPixel(2u, 2u);

  Expect(display.SetPixelRow(3u, row_bits | 0xF800u) == kEOk, "a row must be written");
  Expect(display.GetPixelRow(3u) == row_bits,
         "lit where its bits are set and dark elsewhere, the bits above the grid unread");
  Expect(display.GetPixel(2u, 2u), "and the row above must be as it was");

  display.Clear();
  display.SetPixel(4u, 0u);
  Expect(display.SetPixelColumn(4u, column_bits | 0xFC00u) == kEOk, "a column must be written");
  Expect(display.GetPixelColumn(4u) == column_bits,
         "lit where its bits are set and dark elsewhere");
  Expect(display.GetPixelColumn(3u) == 0u && display.GetPixelColumn(5u) == 0u,
         "and nothing beside it");

  PixelBufferType const before = ReadPixels();
  Expect(display.SetPixelRow(DISPLAY_NUMBER_OF_ROWS, 0xFFFFu) == kENotOk,
         "a row off the grid must be refused");
  Expect(display.SetPixelColumn(DISPLAY_NUMBER_OF_COLUMNS, 0xFFFFu) == kENotOk,
         "and so must a column");
  Expect(ArePixelsEqual(before, ReadPixels()), "with nothing written");

  display.Clear();
}

void TestPixelByIndexTogglesAndReads() {
  Display& display = Display::GetInstance();
  // Row 2, column 1: the index counts row by row.
  constexpr Display::IndexType kIndex{2u * DISPLAY_NUMBER_OF_COLUMNS + 1u};
  Display::PixelValueType value{false};

  display.Clear();
  Expect(display.TogglePixel(kIndex) == kEOk && display.GetPixel(1u, 2u),
         "a dark pixel must toggle to lit");
  Expect(display.GetPixel(kIndex, value) == kEOk && value, "and read back as lit by its index");
  Expect(display.TogglePixel(kIndex) == kEOk && display.IsCleared(), "and toggle back to dark");
  Expect(display.GetPixel(kIndex, value) == kEOk && !value, "reading back as dark");

  value = true;
  Expect(display.TogglePixel(DISPLAY_NUMBER_OF_PIXELS) == kENotOk && display.IsCleared(),
         "a pixel past the last one must be refused, nothing toggled");
  Expect(display.GetPixel(DISPLAY_NUMBER_OF_PIXELS, value) == kENotOk && value,
         "and refused when read, the value left as it was");
}

void TestInitTurnsEveryPixelOff() {
  Display& display = Display::GetInstance();

  display.SetWord(DisplayWords::WordIdType::kEs);
  display.Init();
  Expect(display.IsCleared(), "the display must be dark after Init()");
  Expect(display.GetState() == Display::StateType::kInit, "and initialised");
}
