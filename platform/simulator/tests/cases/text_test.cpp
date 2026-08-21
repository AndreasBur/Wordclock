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

/* Which characters the text overlay can draw, checked through the one entry point that
   says so: setChar() refuses what it cannot map to a glyph.
   The font tables hold 103 entries - ASCII 0x20 to 0x7F, then the six umlauts and the
   degree sign - and the conversion used to check only the lower bound. Where char is unsigned, which is what AVR
   makes it, every other Latin-1 byte passed that check and indexed past the end of the
   table. The host cannot show that: char is signed here, so those bytes come out negative
   and are refused for the wrong reason. The case is checked anyway, because it is the one
   a port would meet. */
void testCharacterToGlyphMapping()
{
    Text& text = Text::getInstance();

    for(char Character = ' '; Character < '\x7F'; Character++) {
        if(text.setChar(0u, 0u, Character, Text::FONT_5X8) != E_OK) {
            expect(false, "every printable character must have a glyph");
            break;
        }
    }

    expect(text.setChar(0u, 0u, '\n', Text::FONT_5X8) == E_NOT_OK, "a control character has no glyph");
    expect(text.setChar(0u, 0u, '\x1F', Text::FONT_5X8) == E_NOT_OK, "and neither has the last one below the space");

    /* The umlauts, which sit behind the ASCII range in every table. */
    const char Umlauts[] = {'\xC4', '\xD6', '\xDC', '\xE4', '\xF6', '\xFC'};
    for(const char Umlaut : Umlauts) {
        if(text.setChar(0u, 0u, Umlaut, Text::FONT_5X8) != E_OK) {
            expect(false, "every umlaut must have a glyph");
            break;
        }
    }

    expect(text.setChar(0u, 0u, '\xE0', Text::FONT_5X8) == E_NOT_OK,
           "a Latin-1 byte that is none of the seven must be refused");

    /* The degree sign, appended past the umlauts so the temperature overlay can show a
       reading rather than spell its unit out. Checked by what reaches the display rather
       than by the width: an unmapped byte falls back to the space, and in the 5x8 table the
       space is exactly as wide as the sign, so a width test would pass on the fallback. */
    for(byte Font = 0u; Font < Text::FONT_NUMBER_OF_FONTS; Font++) {
        const Text::FontType FontType = static_cast<Text::FontType>(Font);
        char Description[64];

        Display::getInstance().clear();
        snprintf(Description, sizeof(Description), "font %u must draw the degree sign", Font);
        expect(text.setChar(0u, 0u, '\xB0', FontType) == E_OK && isAnyPixelLit(readPixels()), Description);
    }

    /* The scrolling text the overlay actually uses goes through the form of the conversion
       that answers an index rather than a code, and substitutes the space where it cannot
       map a byte. So the sign has to be checked there as well, and against the space rather
       than against an empty display - a fallback would light nothing and look like a blank
       glyph. */
    Display::getInstance().clear();
    text.setChar(0u, 0u, '\xB0', Text::FONT_5X8);
    const PixelBufferType Sign = readPixels();

    Display::getInstance().clear();
    text.setChar(0u, 0u, ' ', Text::FONT_5X8);
    const PixelBufferType Space = readPixels();

    expect(!arePixelsEqual(Sign, Space), "the substituting conversion must not fall back to the space");

    /* Every font carries the same character set, and a glyph nobody can see would be a
       table read as the wrong packing. */
    for(byte Font = 0u; Font < Text::FONT_NUMBER_OF_FONTS; Font++) {
        const Text::FontType FontType = static_cast<Text::FontType>(Font);
        char Description[64];

        snprintf(Description, sizeof(Description), "font %u must have a width for a letter", Font);
        expect(text.getFontCharWidth(FontType, 'A') > 0u, Description);

        snprintf(Description, sizeof(Description), "font %u must be able to draw a letter", Font);
        expect(text.setChar(0u, 0u, 'A', FontType) == E_OK, Description);
    }

    Display::getInstance().clear();
}


/* The scrolling path draws every character, including the ones setChar() refuses. It used
   to reach a second entry point that substituted the space silently; there is one entry
   point now, so the substitution is where the scrolling task can be read to make it -
   and it has to keep happening, because the space is the blank glyph and drawing it is
   what clears the cells the previous shift step wrote. Leaving the character out would
   smear those pixels along the display for as long as the text runs.

   Checked against the space over a lit display rather than an empty one: on a dark display
   drawing a blank glyph and drawing nothing at all are the same picture. */
void testUndrawableCharacterShiftsAsASpace()
{
    Text& text = Text::getInstance();
    Display& display = Display::getInstance();

    const auto shiftOver = [&](char Character) {
        text.stop();
        display.clear();
        /* Something to clear, in every pixel the glyph could touch. */
        display.test();
        text.setCharWithShift(Character, Text::FONT_5X8);

        for(byte Step = 0u; Step < DISPLAY_NUMBER_OF_COLUMNS && text.getState() != Text::STATE_IDLE; Step++) {
            text.task();
        }
        return readPixels();
    };

    const PixelBufferType Unmapped = shiftOver('\xE0');
    const PixelBufferType Space = shiftOver(' ');
    const PixelBufferType Letter = shiftOver('A');

    expect(arePixelsEqual(Unmapped, Space), "a character with no glyph must shift through as the space");
    /* Otherwise the case above would also pass on a display the shift left dark, which is
       the one picture that says nothing about what was drawn into it. */
    expect(!arePixelsEqual(Letter, Space), "while a letter must not leave the same picture as the space");
    expect(isAnyPixelLit(Space), "and the shift must leave something lit to have cleared around");

    text.stop();
    display.clear();
}
