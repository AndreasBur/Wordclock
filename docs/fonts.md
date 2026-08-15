# Fonts

The firmware carries five bitmap fonts as generated tables under
[firmware/src/Font/](../firmware/src/Font/), with the matching class per font in
[firmware/inc/Font/](../firmware/inc/Font/). They are used by the text overlay,
not by the clock itself — the time is spelled out by the fixed letter grid.

| Font | Glyph size | Characters | Packing | Word |
|------|-----------|------------|---------|------|
| [FontSprite5x8](../firmware/inc/Font/FontSprite5x8.h) | 5×8 | 103 | vertical | `byte` |
| [FontCourierNew7x9](../firmware/inc/Font/FontCourierNew7x9.h) | 7×9 | 103 | horizontal | `byte` |
| [FontCourierNew7x10](../firmware/inc/Font/FontCourierNew7x10.h) | 7×10 | 103 | horizontal | `byte` |
| [FontLucidaSans9x10](../firmware/inc/Font/FontLucidaSans9x10.h) | 9×10 | 103 | vertical | `uint16_t` |
| [FontTahoma10x10](../firmware/inc/Font/FontTahoma10x10.h) | 10×10 | 103 | horizontal | `uint16_t` |

## How a table is packed

A glyph is stored as one word per line of pixels, and the two packings differ in
which direction a line runs:

- **vertical** ([`FontCharVertical`](../firmware/inc/Font/FontChar.h)) — one word
  per pixel **column**, so a word holds as many bits as the glyph is high.
- **horizontal** ([`FontCharHorizontal`](../firmware/inc/Font/FontChar.h)) — one
  word per pixel **row**, so a word holds as many bits as the glyph is wide.

The word type follows from that: it is a `byte` while the packed direction fits in
8 bits and a `uint16_t` beyond. Both packings are in use, so a table only makes
sense together with the class that declares it — a vertical table read as a
horizontal one yields transposed glyphs, not garbage, which makes the mistake easy
to miss.

## Regenerating a table

The generator is a separate tool: **https://github.com/theAndreas/FontCreator**

It has no file input or output. The character set lives in its `main.cpp` and the
packed words go to standard output, so a table is produced by pasting the set,
setting the size constants, choosing the packing that matches the target font
class, and copying the output into the `Font*.cpp` here. Its README describes the
steps and the two switches that select ASCII-art preview instead of hex, and row
instead of column packing.

The tool used to live in this repository under `FontCreator/` and was moved out
because it is a build-time utility, not part of the firmware.

## Adding a single glyph

Regenerating is the wrong tool for one character. FontCreator rasterises the faces
these tables came from — Courier New, Lucida Sans and Tahoma are Windows fonts — so
producing one new glyph with it reproduces the other 102 as whatever the host
renders them, and the diff covers the whole file instead of one line.

A glyph appended by hand costs three edits, which is the path the degree sign took:

1. One `FontTableElementType` line per table, after the last entry. The literals are
   readable: `0b` bits for the 5×8 table, hex for the rest, LSB first in both
   packings — bit 0 is the top row of a column, or the leftmost pixel of a row.
2. `*_FONT_TABLE_SIZE` in the matching header, raised by one.
3. A branch in both `convertCharToFontIndex` and `convertCharToFontIndexFast`
   ([Text.cpp](../firmware/src/Text/Text.cpp)), mapping the Latin-1 byte to the new
   index. Past the ASCII run rather than inside it, the same way the six umlauts sit
   at 96 to 101 — a character inserted into the run would shift every index after it.

The fast conversion answers with the space rather than a return code for a byte it
cannot map, so a missing branch there draws a blank instead of failing. That is what
the degree sign's check in
[WordclockTests.cpp](../platform/simulator/tests/WordclockTests.cpp) compares
against: the space, not an empty display.
