/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       accessor_test.cpp
 *      \brief      What an accessor answers for an index that is not there, now that each one has a single form
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Display.h"
#include "DisplayCharacters.h"
#include "DisplayWords.h"
#include "FontChar.h"
#include "Pixels.h"

/*
 * Every reader used to come in two forms, and the second one trusted its caller: it read
 * the buffer or the table at whatever index it was handed. That is what these cases exist
 * for. There is one form of each now, it checks, and what it answers for an index that is
 * not there is written down here rather than left to whatever sits past the end.
 *
 * An unlit pixel, a null character and a word of zero length are the answers, because none
 * of them can be mistaken for something that is on the display.
 */
void TestReadingPastTheEndIsEmpty()
{
    Pixels& pixels = Pixels::GetInstance();
    constexpr byte kPastTheStrip{PIXELS_NUMBER_OF_PIXELS};

    const Pixels::PixelType unlit = pixels.GetPixel(kPastTheStrip);
    Expect(unlit.GetRed() == 0u && unlit.GetGreen() == 0u && unlit.GetBlue() == 0u,
           "a pixel past the strip must read as unlit");

    // The dimmed form goes through the same reader, so it inherits the check rather than
    // having one of its own - and a dimmed black is still black.
    const Pixels::PixelType unlit_output = pixels.GetOutputPixel(kPastTheStrip);
    Expect(unlit_output.GetRed() == 0u && unlit_output.GetGreen() == 0u && unlit_output.GetBlue() == 0u,
           "and so must the dimmed copy of it");

    Pixels::PixelType pixel;
    Expect(pixels.GetPixel(kPastTheStrip, pixel) == kENotOk,
           "the form that answers a code must still refuse it");

    Display& display = Display::GetInstance();
    Expect(display.GetPixel(DISPLAY_NUMBER_OF_PIXELS) == false,
           "a display pixel past the last one must read as dark");
    Expect(display.GetPixel(DISPLAY_NUMBER_OF_COLUMNS, 0u) == false,
           "and so must a column past the last one");

    const DisplayCharacters characters;
    Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS) == STD_NULL_CHARACTER,
           "an index past the letter table must read as no character");
    Expect(characters.GetCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u) == STD_NULL_CHARACTER,
           "and so must a column past the plate");

    const DisplayWords words;
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
void TestWritingPastTheEndIsRefused()
{
    Display& display = Display::GetInstance();

    display.Clear();
    const PixelBufferType before = ReadPixels();

    Expect(Pixels::GetInstance().SetPixel(PIXELS_NUMBER_OF_PIXELS, Pixels::PixelType(255u, 255u, 255u)) == kENotOk,
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
void TestGlyphAccessorsCheckBothWays()
{
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

    Expect(columns.GetColumn(0u, column) == kEOk && column == 0b11u, "a column of the glyph must read back");
    Expect(columns.GetColumn(0u) == 0b11u, "and the form answering the column itself must agree");
    Expect(columns.GetColumn(3u, column) == kENotOk, "a column past the glyph must be refused");
    Expect(columns.GetColumn(3u) == 0u, "and answered as empty where no code is asked for");
    Expect(columns.SetColumn(1u, 0b111u) == kEOk && columns.GetColumn(1u) == 0b111u, "a column must be writable");
    Expect(columns.SetColumn(3u, 0b111u) == kENotOk, "a column past the glyph must not be writable");
}
