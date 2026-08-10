# Fonts

The firmware carries five bitmap fonts as generated tables under
[firmware/src/Font/](../firmware/src/Font/), with the matching class per font in
[firmware/inc/Font/](../firmware/inc/Font/). They are used by the text overlay,
not by the clock itself — the time is spelled out by the fixed letter grid.

| Font | Glyph size | Characters | Packing | Word |
|------|-----------|------------|---------|------|
| [FontSprite5x8](../firmware/inc/Font/FontSprite5x8.h) | 5×8 | 102 | vertical | `byte` |
| [FontCourierNew7x9](../firmware/inc/Font/FontCourierNew7x9.h) | 7×9 | 102 | horizontal | `byte` |
| [FontCourierNew7x10](../firmware/inc/Font/FontCourierNew7x10.h) | 7×10 | 102 | horizontal | `byte` |
| [FontLucidaSans9x10](../firmware/inc/Font/FontLucidaSans9x10.h) | 9×10 | 102 | vertical | `uint16_t` |
| [FontTahoma10x10](../firmware/inc/Font/FontTahoma10x10.h) | 10×10 | 102 | horizontal | `uint16_t` |

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
