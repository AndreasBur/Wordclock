# Measured decisions

What is written down here was settled by building the image both ways rather than by
argument, and has no header that owns it: it is about a pattern spread over the whole
tree, not about one class. The numbers describe the change that was made and are
therefore history — they were true when measured, and updating them would make them a
lie. The [platform contract](../platform/avr-dx/README.md) states the rule that came out
of it; this is the measurement behind the rule.

## The `Fast` accessor pair

Every platform accessor used to come in two forms: one that validated its index and
answered `StdReturnType`, one that trusted the caller and answered the value. The
justification was real for this target — a freestanding 8-bit part built with `-Os` and a
loop over 110 pixels per frame, where a bounds check per pixel is time rather than a
formality.

What put it in doubt is that `GetOutputPixel` existed for its whole life with only the
`Fast` half, on all three backends, and nothing noticed. A pattern in two parts whose
second part nothing enforces loses its second part.

**The justification did not survive the measurement.** The bounds check was put inside
the platform seam's `Fast` forms, so that all three hundred call sites paid for it without
one of them being rewritten, and the AVR image was built either way:

| | text |
|---|---|
| as it was | 48 210 |
| with the seam checking every access | **48 114** |

The checked build was **96 bytes smaller**, and the reason is worth more than the number.
Per symbol, `Pixels::setPixelFast` grew by 66 bytes and then stopped being inlined, which
took 48 bytes off `Display::SetPixel`, 46 off `Display::setPixelFast`, 42 off
`AnimationMatrix::SetTimeTask` and 48 off the RPC dispatcher. So what the difference
measured is an inlining threshold, not the cost of a comparison — **and a flash argument
that turns on the inliner's mood is not an argument.**

Two things the measurement turned up that nobody had anticipated.

The pair was invisible to the linker almost everywhere. Of 45 declared pairs, **35
appeared in the image as neither half** — fully inlined or dropped — and only three
(`GetPixel`, `SetPixel`, `WritePixel`) existed as both. For most of the pattern there was
no runtime object to be cheap or expensive about; it was upkeep and nothing else.

And in the core, `Fast` did not mean unchecked. `Display::SetPixel(column, row)`
validated nothing itself: it computed an index and handed it to `Pixels::SetPixel`, which
is where the only check in the chain lived. The two halves in `Display` differed in
whether the caller was *told*, not in whether anything was verified — so half the pattern
was not a safety mechanism at all.

**The time argument, which no host can measure, comes out negligible by arithmetic.** The
frame path is 110 accesses, and `Render()` returns early unless the buffer is dirty, so
the worst case is a display changing on every 10 ms tick: 11 000 checks a second. At a
generous four cycles each that is 44 000 of 24 000 000, **0.18 % of the part**, and a
clock showing a settled face pays none of it.

### What replaced it

The check moved into the single implementation and the name went with the pattern. There
is one accessor per operation now, and where a reader has two forms the argument list says
which — `GetPixel(index, pixel)` answers a code, `GetPixel(index)` answers the pixel, and
**both check**. Writers have one form, which answers; a caller with nothing to do with
that answer ignores it. 440 mentions across the core and all four backends, and the image
came out **586 bytes smaller** — 48 664 before, 48 078 after, RAM unchanged to the byte.

Six things it settled, and they are the reason the shape is what it is:

- **The asymmetry decided itself, and not by taste.** Two writers cannot differ in their
  return type alone, so a writer's unchecked half had nowhere to go but out. Readers
  differ in their argument list, so both of theirs live under one name — and the one
  answering a value needs something to answer for an index that is not there. That is a
  default in every case: an unlit pixel, a null character, a word of zero length, an empty
  glyph row. None of them can be mistaken for something that is on the display, which is
  what makes the answer honest rather than a zero that reads like data.
- **The unchecked read did not disappear, it went private.**
  `GetDisplayCharactersTableElement`, `GetDisplayWordsTableElement`, `GetFontTableElement`
  and the pixel buffer are the primitives both public forms go through, so each path
  checks exactly once and the class keeps one place that touches the table.
- **A column past the last one used to light the next row.**
  `Display::SetPixel(column, row)` computed an index and handed it to `Pixels`, where 11
  on an 11-wide display is a perfectly valid index — the first letter of the row below. So
  the write was neither refused nor put where it was asked for. `IsColumnAndRowValid` is
  asked by every entry point taking a column now, and the case that found it is in the
  tests; reading the code had not.
- **Two branches that had never compiled fell out.** `FontChar`'s checked `SetRow` and
  `SetColumn` assigned a member from a `const` function, and `Display::TogglePixel`'s
  serpentine branch passed a pointer where a reference was expected. A template nobody
  instantiates and a `#if` branch nobody selects are not code — the same thesis arriving
  from the other side. The glyph accessors are instantiated by a test now.
- **The duplication was worth more than the checks were.** That is where the 586 bytes
  came from, and not from the bounds tests: `Text::SetChar` existed as two 900-byte bodies
  inlined into a caller each and is one function now, and six `#if` ladders in `Display`
  mapping a column and a row onto an index became one `ToIndex`. Of the 45 declared pairs,
  nine had both halves in the image at 486 bytes; `Display::SetPixel(column, row)` alone
  was 258 bytes of pair and is 78 as one.
- **One substitution had to be moved rather than dropped.** The scrolling text relied on
  the unchecked `SetChar` drawing a space for a character it could not map, which is what
  clears the cells the previous shift step wrote; the surviving `SetChar` refuses, because
  a test says it must. So the shift task substitutes the space itself, where it can be
  read — and a case fails if it stops. The `[[nodiscard]]` that used to police the four
  `Text` twins went with them, and so did the `assert` in three backends' unchecked
  writers: what they caught is what two new cases check, in CI rather than under a
  debugger.
