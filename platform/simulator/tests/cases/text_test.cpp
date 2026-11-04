/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       text_test.cpp
 *      \brief      Which characters the text overlay can draw, through the entry point that refuses the rest
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Display.h"
#include "Pixels.h"
#include "Text.h"

/*
 * Which characters the text overlay can draw, checked through the one entry point that
 * says so: SetChar() refuses what it cannot map to a glyph.
 * The font tables hold 103 entries - ASCII 0x20 to 0x7F, then the six umlauts and the
 * degree sign - and the conversion used to check only the lower bound. Where char is unsigned, which is what AVR
 * makes it, every other Latin-1 byte passed that check and indexed past the end of the
 * table. The host cannot show that: char is signed here, so those bytes come out negative
 * and are refused for the wrong reason. The case is checked anyway, because it is the one
 * a port would meet.
 */
void TestCharacterToGlyphMapping()
{
    Text& text = Text::GetInstance();

    for(char character = ' '; character < '\x7F'; character++) {
        if(text.SetChar(0u, 0u, character, Text::FontType::kFont5x8) != kEOk) {
            Expect(false, "every printable character must have a glyph");
            break;
        }
    }

    Expect(text.SetChar(0u, 0u, '\n', Text::FontType::kFont5x8) == kENotOk, "a control character has no glyph");
    Expect(text.SetChar(0u, 0u, '\x1F', Text::FontType::kFont5x8) == kENotOk, "and neither has the last one below the space");

    /* The umlauts, which sit behind the ASCII range in every table. */
    const char umlauts[] = {'\xC4', '\xD6', '\xDC', '\xE4', '\xF6', '\xFC'};
    for(const char umlaut : umlauts) {
        if(text.SetChar(0u, 0u, umlaut, Text::FontType::kFont5x8) != kEOk) {
            Expect(false, "every umlaut must have a glyph");
            break;
        }
    }

    Expect(text.SetChar(0u, 0u, '\xE0', Text::FontType::kFont5x8) == kENotOk,
           "a Latin-1 byte that is none of the seven must be refused");

    /*
     * The degree sign, appended past the umlauts so the temperature overlay can show a
     * reading rather than spell its unit out. Checked by what reaches the display rather
     * than by the width: an unmapped byte falls back to the space, and in the 5x8 table the
     * space is exactly as wide as the sign, so a width test would pass on the fallback.
     */
    for(byte font = 0u; font < Text::kNumberOfFonts; font++) {
        const Text::FontType font_type = static_cast<Text::FontType>(font);
        char description[64];

        Display::GetInstance().Clear();
        {
            const int length = snprintf(description, sizeof(description), "font %u must draw the degree sign", font);
            Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
                   "test description must fit its buffer");
        }
        Expect(text.SetChar(0u, 0u, '\xB0', font_type) == kEOk && IsAnyPixelLit(ReadPixels()), description);
    }

    /*
     * The scrolling text the overlay actually uses goes through the form of the conversion
     * that answers an index rather than a code, and substitutes the space where it cannot
     * map a byte. So the sign has to be checked there as well, and against the space rather
     * than against an empty display - a fallback would light nothing and look like a blank
     * glyph.
     */
    Display::GetInstance().Clear();
    text.SetChar(0u, 0u, '\xB0', Text::FontType::kFont5x8);
    const PixelBufferType sign = ReadPixels();

    Display::GetInstance().Clear();
    text.SetChar(0u, 0u, ' ', Text::FontType::kFont5x8);
    const PixelBufferType space = ReadPixels();

    Expect(!ArePixelsEqual(sign, space), "the substituting conversion must not fall back to the space");

    // Every font carries the same character set, and a glyph nobody can see would be a
    // table read as the wrong packing.
    for(byte font = 0u; font < Text::kNumberOfFonts; font++) {
        const Text::FontType font_type = static_cast<Text::FontType>(font);
        char description[64];

        {
            const int length = snprintf(description, sizeof(description), "font %u must have a width for a letter", font);
            Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
                   "test description must fit its buffer");
        }
        Expect(text.GetFontCharWidth(font_type, 'A') > 0u, description);

        {
            const int length = snprintf(description, sizeof(description), "font %u must be able to draw a letter", font);
            Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
                   "test description must fit its buffer");
        }
        Expect(text.SetChar(0u, 0u, 'A', font_type) == kEOk, description);
    }

    Display::GetInstance().Clear();
}


/*
 * The scrolling path draws every character, including the ones SetChar() refuses. It used
 * to reach a second entry point that substituted the space silently; there is one entry
 * point now, so the substitution is where the scrolling task can be read to make it -
 * and it has to keep happening, because the space is the blank glyph and drawing it is
 * what clears the cells the previous shift step wrote. Leaving the character out would
 * smear those pixels along the display for as long as the text runs.
 *
 * Checked against the space over a lit display rather than an empty one: on a dark display
 * drawing a blank glyph and drawing nothing at all are the same picture.
 */
void TestUndrawableCharacterShiftsAsASpace()
{
    Text& text = Text::GetInstance();
    Display& display = Display::GetInstance();

    const auto shift_over = [&](char character) {
        text.Stop();
        display.Clear();
        /* Something to clear, in every pixel the glyph could touch. */
        display.Test();
        text.SetCharWithShift(character, Text::FontType::kFont5x8);

        for(byte step = 0u; step < DISPLAY_NUMBER_OF_COLUMNS && text.GetState() != Text::StateType::kIdle; step++) {
            text.Task();
        }
        return ReadPixels();
    };

    const PixelBufferType unmapped = shift_over('\xE0');
    const PixelBufferType space = shift_over(' ');
    const PixelBufferType letter = shift_over('A');

    Expect(ArePixelsEqual(unmapped, space), "a character with no glyph must shift through as the space");
    // Otherwise the case above would also pass on a display the shift left dark, which is
    // the one picture that says nothing about what was drawn into it.
    Expect(!ArePixelsEqual(letter, space), "while a letter must not leave the same picture as the space");
    Expect(IsAnyPixelLit(space), "and the shift must leave something lit to have cleared around");

    text.Stop();
    display.Clear();
}
