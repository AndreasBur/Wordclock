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

/* Every reader used to come in two forms, and the second one trusted its caller: it read
   the buffer or the table at whatever index it was handed. That is what these cases exist
   for. There is one form of each now, it checks, and what it answers for an index that is
   not there is written down here rather than left to whatever sits past the end.

   An unlit pixel, a null character and a word of zero length are the answers, because none
   of them can be mistaken for something that is on the display. */
void testReadingPastTheEndIsEmpty()
{
    Pixels& pixels = Pixels::getInstance();
    constexpr byte PastTheStrip{PIXELS_NUMBER_OF_PIXELS};

    const Pixels::PixelType unlit = pixels.getPixel(PastTheStrip);
    expect(unlit.getRed() == 0u && unlit.getGreen() == 0u && unlit.getBlue() == 0u,
           "a pixel past the strip must read as unlit");

    /* The dimmed form goes through the same reader, so it inherits the check rather than
       having one of its own - and a dimmed black is still black. */
    const Pixels::PixelType unlitOutput = pixels.getOutputPixel(PastTheStrip);
    expect(unlitOutput.getRed() == 0u && unlitOutput.getGreen() == 0u && unlitOutput.getBlue() == 0u,
           "and so must the dimmed copy of it");

    Pixels::PixelType pixel;
    expect(pixels.getPixel(PastTheStrip, pixel) == E_NOT_OK,
           "the form that answers a code must still refuse it");

    Display& display = Display::getInstance();
    expect(display.getPixel(DISPLAY_NUMBER_OF_PIXELS) == false,
           "a display pixel past the last one must read as dark");
    expect(display.getPixel(DISPLAY_NUMBER_OF_COLUMNS, 0u) == false,
           "and so must a column past the last one");

    const DisplayCharacters characters;
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS) == STD_NULL_CHARACTER,
           "an index past the letter table must read as no character");
    expect(characters.getCharacter(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS, 0u) == STD_NULL_CHARACTER,
           "and so must a column past the plate");

    const DisplayWords words;
    expect(words.getDisplayWord(DisplayWords::WORD_NUMBER_OF_WORDS).getLength() == 0u,
           "a word that is not in the table must have no length");
    expect(words.getDisplayWordLength(DisplayWords::WORD_NUMBER_OF_WORDS) == 0u,
           "which is what the length on its own answers too");
}


/* The writers lost their second form entirely: what is left answers, and a caller with
   nothing to do with the answer ignores it. So the check is no longer something a caller
   opts into by picking a name, and an index past the end changes nothing. */
void testWritingPastTheEndIsRefused()
{
    Display& display = Display::getInstance();

    display.clear();
    const PixelBufferType before = readPixels();

    expect(Pixels::getInstance().setPixel(PIXELS_NUMBER_OF_PIXELS, Pixels::PixelType(255u, 255u, 255u)) == E_NOT_OK,
           "a pixel past the strip must be refused");
    expect(Pixels::getInstance().clearPixel(PIXELS_NUMBER_OF_PIXELS) == E_NOT_OK,
           "and so must clearing one");
    expect(display.setPixel(DISPLAY_NUMBER_OF_PIXELS) == E_NOT_OK,
           "a display pixel past the last one must be refused");
    expect(display.setPixel(DISPLAY_NUMBER_OF_COLUMNS, 0u) == E_NOT_OK,
           "and so must a column past the last one");
    expect(display.setWord(DisplayWords::WORD_NUMBER_OF_WORDS) == E_NOT_OK,
           "a word that is not in the table must be refused");

    expect(arePixelsEqual(before, readPixels()), "none of the refused writes may reach the strip");
}


/* The glyph accessors, which are the ones this collapse found something in: their checked
   halves were templates nobody instantiated, and the setter among them assigned a member
   from a const function. That does not compile - and it never had to, because only the
   unchecked half was ever called. These cases call the survivors, so the code is compiled
   rather than merely written. */
void testGlyphAccessorsCheckBothWays()
{
    FontCharHorizontal<uint16_t, 3u> rows(2u, {0b11u, 0b01u, 0b10u});
    uint16_t row{0u};

    expect(rows.getRow(0u, row) == E_OK && row == 0b11u, "a row of the glyph must read back");
    expect(rows.getRow(0u) == 0b11u, "and the form answering the row itself must agree");
    expect(rows.getRow(3u, row) == E_NOT_OK, "a row past the glyph must be refused");
    expect(rows.getRow(3u) == 0u, "and answered as empty where no code is asked for");
    expect(rows.setRow(1u, 0b111u) == E_OK && rows.getRow(1u) == 0b111u, "a row must be writable");
    expect(rows.setRow(3u, 0b111u) == E_NOT_OK, "a row past the glyph must not be writable");

    FontCharVertical<uint16_t, 3u> columns(2u, {0b11u, 0b01u, 0b10u});
    uint16_t column{0u};

    expect(columns.getColumn(0u, column) == E_OK && column == 0b11u, "a column of the glyph must read back");
    expect(columns.getColumn(0u) == 0b11u, "and the form answering the column itself must agree");
    expect(columns.getColumn(3u, column) == E_NOT_OK, "a column past the glyph must be refused");
    expect(columns.getColumn(3u) == 0u, "and answered as empty where no code is asked for");
    expect(columns.setColumn(1u, 0b111u) == E_OK && columns.getColumn(1u) == 0b111u, "a column must be writable");
    expect(columns.setColumn(3u, 0b111u) == E_NOT_OK, "a column past the glyph must not be writable");
}
