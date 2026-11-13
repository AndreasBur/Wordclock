// What an accessor answers for an index that is not there, now that each one has a single form.

#include "Display.h"
#include "DisplayCharacters.h"
#include "DisplayWords.h"
#include "FontChar.h"
#include "FontSprite5x8.h"
#include "Pixels.h"
#include "cases.h"
#include "check.h"

/*
 * Every reader used to come in two forms, and the second one trusted its caller: it read
 * the buffer or the table at whatever index it was handed. That is what these cases exist
 * for. There is one form of each now, it checks, and what it answers for an index that is
 * not there is written down here rather than left to whatever sits past the end.
 *
 * An unlit pixel, a null character and a word of zero length are the answers, because none
 * of them can be mistaken for something that is on the display.
 */
void TestReadingPastTheEndIsEmpty() {
  Pixels& pixels = Pixels::GetInstance();
  constexpr byte kPastTheStrip{PIXELS_NUMBER_OF_PIXELS};

  Pixels::PixelType const unlit = pixels.GetPixel(kPastTheStrip);
  Expect(unlit.GetRed() == 0u && unlit.GetGreen() == 0u && unlit.GetBlue() == 0u,
         "a pixel past the strip must read as unlit");

  // The dimmed form goes through the same reader, so it inherits the check rather than
  // having one of its own - and a dimmed black is still black.
  Pixels::PixelType const unlit_output = pixels.GetOutputPixel(kPastTheStrip);
  Expect(
      unlit_output.GetRed() == 0u && unlit_output.GetGreen() == 0u && unlit_output.GetBlue() == 0u,
      "and so must the dimmed copy of it");

  Pixels::PixelType pixel;
  Expect(pixels.GetPixel(kPastTheStrip, pixel) == kENotOk,
         "the form that answers a code must still refuse it");

  Display& display = Display::GetInstance();
  Expect(display.GetPixel(DISPLAY_NUMBER_OF_PIXELS) == false,
         "a display pixel past the last one must read as dark");
  Expect(display.GetPixel(DISPLAY_NUMBER_OF_COLUMNS, 0u) == false,
         "and so must a column past the last one");

  DisplayCharacters const characters;
  Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS) == STD_NULL_CHARACTER,
         "an index past the letter table must read as no character");
  Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u) == STD_NULL_CHARACTER,
         "and so must a column past the plate");

  DisplayWords const words;
  Expect(words.GetDisplayWord(DisplayWords::WordIdType::kNumberOfWords).GetLength() == 0u,
         "a word that is not in the table must have no length");
  Expect(words.GetDisplayWordLength(DisplayWords::WordIdType::kNumberOfWords) == 0u,
         "which is what the length on its own answers too");
}

/*
 * The writers lost their second form entirely: what is left answers, and a caller with
 * nothing to do with the answer ignores it. So the check is no longer something a caller
 * opts into by picking a name, and an index past the end changes nothing.
 */
void TestWritingPastTheEndIsRefused() {
  Display& display = Display::GetInstance();

  display.Clear();
  PixelBufferType const before = ReadPixels();

  Expect(Pixels::GetInstance().SetPixel(PIXELS_NUMBER_OF_PIXELS,
                                        Pixels::PixelType(255u, 255u, 255u)) == kENotOk,
         "a pixel past the strip must be refused");
  Expect(Pixels::GetInstance().ClearPixel(PIXELS_NUMBER_OF_PIXELS) == kENotOk,
         "and so must clearing one");
  Expect(display.SetPixel(DISPLAY_NUMBER_OF_PIXELS) == kENotOk,
         "a display pixel past the last one must be refused");
  Expect(display.SetPixel(DISPLAY_NUMBER_OF_COLUMNS, 0u) == kENotOk,
         "and so must a column past the last one");
  Expect(display.SetWord(DisplayWords::WordIdType::kNumberOfWords) == kENotOk,
         "a word that is not in the table must be refused");

  Expect(ArePixelsEqual(before, ReadPixels()), "none of the refused writes may reach the strip");
}

/*
 * The glyph accessors, which are the ones this collapse found something in: their checked
 * halves were templates nobody instantiated, and the setter among them assigned a member
 * from a const function. That does not compile - and it never had to, because only the
 * unchecked half was ever called. These cases call the survivors, so the code is compiled
 * rather than merely written.
 */
void TestGlyphAccessorsCheckBothWays() {
  FontCharHorizontal<uint16_t, 3u> rows(2u, {0b11u, 0b01u, 0b10u});
  uint16_t row{0u};

  Expect(rows.GetRow(0u, row) == kEOk && row == 0b11u, "a row of the glyph must read back");
  Expect(rows.GetRow(0u) == 0b11u, "and the form answering the row itself must agree");
  Expect(rows.GetRow(3u, row) == kENotOk, "a row past the glyph must be refused");
  Expect(rows.GetRow(3u) == 0u, "and answered as empty where no code is asked for");
  Expect(rows.SetRow(1u, 0b111u) == kEOk && rows.GetRow(1u) == 0b111u, "a row must be writable");
  Expect(rows.SetRow(3u, 0b111u) == kENotOk, "a row past the glyph must not be writable");

  FontCharVertical<uint16_t, 3u> columns(2u, {0b11u, 0b01u, 0b10u});
  uint16_t column{0u};

  Expect(columns.GetColumn(0u, column) == kEOk && column == 0b11u,
         "a column of the glyph must read back");
  Expect(columns.GetColumn(0u) == 0b11u, "and the form answering the column itself must agree");
  Expect(columns.GetColumn(3u, column) == kENotOk, "a column past the glyph must be refused");
  Expect(columns.GetColumn(3u) == 0u, "and answered as empty where no code is asked for");
  Expect(columns.SetColumn(1u, 0b111u) == kEOk && columns.GetColumn(1u) == 0b111u,
         "a column must be writable");
  Expect(columns.SetColumn(3u, 0b111u) == kENotOk, "a column past the glyph must not be writable");
}

/*
 * The readers whose form answering a code nothing in the firmware calls: the four of a word,
 * the width of a glyph, and a row or a column of the grid, whose forms nothing calls at all.
 * They are kept as the rest of the pairs are, so they are compiled and run here - over the
 * whole table where there is one, so that a form reading the wrong field of one entry shows.
 */
void TestWordAccessorsCheckBothWays() {
  DisplayWords const words;

  for (byte id = 0u; id < DisplayWords::kNumberOfWords; id++) {
    auto const word_id = static_cast<DisplayWords::WordIdType>(id);
    DisplayWord const expected = words.GetDisplayWord(word_id);
    DisplayWord word;
    byte length{0xFFu};
    byte column{0xFFu};
    byte row{0xFFu};

    Expect(words.GetDisplayWord(word_id, word) == kEOk &&
               word.GetColumn() == expected.GetColumn() && word.GetRow() == expected.GetRow() &&
               word.GetLength() == expected.GetLength(),
           "both forms of a word must agree on every word of the table");
    Expect(words.GetDisplayWordLength(word_id, length) == kEOk &&
               length == words.GetDisplayWordLength(word_id),
           "and so must both forms of its length");
    Expect(words.GetDisplayWordColumn(word_id, column) == kEOk &&
               column == words.GetDisplayWordColumn(word_id),
           "of its column");
    Expect(words.GetDisplayWordRow(word_id, row) == kEOk && row == words.GetDisplayWordRow(word_id),
           "and of its row");
  }

  // One entry by hand, so that two forms reading the same wrong field cannot agree their way
  // through: ES is the first word of the plate, two letters from its corner.
  DisplayWord es;
  Expect(words.GetDisplayWord(DisplayWords::WordIdType::kEs, es) == kEOk && es.GetColumn() == 0u &&
             es.GetRow() == 0u && es.GetLength() == 2u,
         "ES must stand at the top left, two letters long");

  constexpr DisplayWords::WordIdType kNotAWord{DisplayWords::WordIdType::kNumberOfWords};
  byte value{0xA5u};
  Expect(words.GetDisplayWordLength(kNotAWord, value) == kENotOk && value == 0xA5u,
         "a word that is not in the table must be refused, the length left as it was");
  Expect(words.GetDisplayWordColumn(kNotAWord, value) == kENotOk && value == 0xA5u,
         "and the column");
  Expect(words.GetDisplayWordRow(kNotAWord, value) == kENotOk && value == 0xA5u, "and the row");
  DisplayWord word(1u, 2u, 3u);
  Expect(words.GetDisplayWord(kNotAWord, word) == kENotOk && word.GetLength() == 3u,
         "and the word");
}

void TestGlyphWidthChecksBothWays() {
  FontSprite5x8 const font;
  byte width{0xFFu};

  for (byte index = 0u; index < FONT_SPRITE_5X8_FONT_TABLE_SIZE; index++) {
    Expect(font.GetCharWidth(index, width) == kEOk && width == font.GetCharWidth(index),
           "both forms of a glyph's width must agree on every glyph of the font");
  }

  width = 0xA5u;
  Expect(font.GetCharWidth(FONT_SPRITE_5X8_FONT_TABLE_SIZE, width) == kENotOk && width == 0xA5u,
         "a glyph past the table must be refused, the width left as it was");
  Expect(font.GetCharWidth(FONT_SPRITE_5X8_FONT_TABLE_SIZE) == 0u,
         "and answered as no width at all");
}

void TestGridRowsAndColumnsReadTheLitPixels() {
  Display& display = Display::GetInstance();
  constexpr byte kLastColumn{DISPLAY_NUMBER_OF_COLUMNS - 1u};
  constexpr byte kLastRow{DISPLAY_NUMBER_OF_ROWS - 1u};

  // Three pixels of the top row, the corners among them, and the bottom of column 3: what
  // both bit orders have to come out of.
  display.Clear();
  display.SetPixel(0u, 0u);
  display.SetPixel(3u, 0u);
  display.SetPixel(kLastColumn, 0u);
  display.SetPixel(3u, kLastRow);

  Display::PixelRowType const top_row = (1u << 0u) | (1u << 3u) | (1u << kLastColumn);
  Display::PixelColumnType const column_3 = (1u << 0u) | (1u << kLastRow);

  Expect(display.GetPixelRow(0u) == top_row, "a row must read bit n for column n");
  Expect(display.GetPixelRow(1u) == 0u, "and an empty row as none");
  Expect(display.GetPixelColumn(3u) == column_3, "a column must read bit n for row n");
  Expect(display.GetPixelColumn(kLastColumn) == 1u, "and the last column its one pixel");

  // The bits above the grid belong to the caller and are left as they were; the ones of the
  // grid are written, the dark ones too.
  Display::PixelRowType row = 0x8000u | (1u << 1u);
  Expect(display.GetPixelRow(0u, row) == kEOk && row == (0x8000u | top_row),
         "the form answering a code must write the row and nothing above it");
  Display::PixelColumnType column = 0x8000u | (1u << 1u);
  Expect(display.GetPixelColumn(3u, column) == kEOk && column == (0x8000u | column_3),
         "and the column the same way");

  row = 0xA5A5u;
  Expect(display.GetPixelRow(DISPLAY_NUMBER_OF_ROWS, row) == kENotOk && row == 0xA5A5u,
         "a row off the grid must be refused, the row left as it was");
  Expect(display.GetPixelRow(DISPLAY_NUMBER_OF_ROWS) == 0u, "and answered as dark");
  column = 0xA5A5u;
  Expect(display.GetPixelColumn(DISPLAY_NUMBER_OF_COLUMNS, column) == kENotOk && column == 0xA5A5u,
         "a column off the grid must be refused, the column left as it was");
  Expect(display.GetPixelColumn(DISPLAY_NUMBER_OF_COLUMNS) == 0u, "and answered as dark");

  display.Clear();
}
