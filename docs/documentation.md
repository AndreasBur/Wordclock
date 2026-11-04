# Documenting the code

How the code of Wordclock is documented: which comments are Doxygen blocks, what each block says,
and how the style is checked. The style is the one of Utl and MaxMatrix, so that every project
documents alike. It holds for `firmware/` and `platform/`; the tests keep a plain comment at the
top and one above each test.

The blocks are read in the code and checked in CI; no reference is published from them. Wordclock
is a firmware, and the reader of a block is the next person who calls or changes the code.

## Two kinds of comment

- **A Doxygen block `/*! ... */`** documents an entity for whoever uses it: what it is, how to use
  it, what it costs. It stands right above the declaration, in the header.
- **A comment on the implementation** explains it to the next person who changes it: why it is
  written this way, which alternative was rejected and why.
  - One or two lines, it is a `//` comment.
  - Three lines or more, or more than one paragraph, it is a block `/* ... */`, each line
    starting with ` * `, without the `!` that would make it Doxygen's.

A comment says what the code cannot say for itself. It does not repeat the code.

### Where a comment on the implementation stands

Where the code it explains is, never between a Doxygen block and its declaration, where it would
read as a second comment of no one:

- **In the body** of the function it explains.
- **Above the private member** whose meaning or layout it explains, as `enabled_` in `Display`.
- **At the definition in the `.cpp`**, when it explains that.
- **In the Doxygen block** instead, when the caller should know it after all, as a `\note`.

Only a `NOLINTNEXTLINE` stands between a block and its declaration, as it has to stand right above
the line it is for.

## The block

```cpp
    /*!
     * \brief Lights the pixel at `column` and `row` in the colour of the display.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left
     *         as they were.
     */
    StdReturnType SetPixel(byte column, byte row);
```

- `/*!` on a line of its own, each further line starting with ` * `, and ` */` on a line of its
  own. A block that holds only a brief is one line: `/*! \brief The number of rows. */`.
- An enum value, a member or a struct field is documented behind it, with `/*!< ... */`, when the
  brief is all it needs and fits on the line. If it does not, a one-line block
  `/*! \brief ... */` stands above it.
- Commands start with a backslash, `\brief`, never with `@`.
- One space between a command, its direction and its name, `\param[in] column`; nothing is aligned
  in columns.
- A line that continues a command starts where the text after the command starts, as the
  `\return` above does. `tools/doc-indent.py --fix` indents it so; without `--fix`, it fails where
  a line is not indented so.

### The parts, in this order

| Part | Says |
|---|---|
| `\brief` | One sentence, ending in a full stop. Always written out. |
| Description | After an empty line: what the caller has to know beyond the brief, in a few short paragraphs. |
| `\tparam` | Each template parameter that the caller chooses. |
| `\param[in]`, `[out]`, `[in,out]` | Each parameter, with its direction. |
| `\return` | What comes back. For a `StdReturnType`, when `kENotOk` comes back and what is left as it was. A one-line block whose brief already says it, "Returns the number of rows.", leaves it out. |
| `\pre` | What the caller must ensure, such as `Init()` first. |
| `\note`, `\warning` | What is easy to miss. `\warning` only where a mistake is costly. |
| `\see` | A related entity, where the link helps. |

A function that returns nothing has no `\return`. Every parameter has its `\param`, even the only
one. A part that has nothing to say is left out. `\details` is not written: the description is
the paragraphs after the brief.

### The brief

- A function: a verb in the third person, saying what it does. "Lights the pixel at `column` and
  `row`.", "Shows the time in words.", "Returns the brightness the display is set to."
- A type: a noun phrase, saying what it is. "The display: the pixels of the letter grid, with
  brightness, colour and current limit.", "A time of day: hour, minute and second."
- A value or a member: what it holds or means. "The word ES of ES IST."

### The description

- Plain, complete sentences. Present tense, active voice, no "we".
- What the caller needs to use the entity correctly: its cost on the AVR in flash and RAM where it
  matters, its limits, what it does at the edges, what it writes to the strip and when.
- An example in `\code{.cpp} ... \endcode` where a call is clearer than a sentence. Keep it short
  and make it compile.
- Names in backticks: parameters as `column`, entities as `DisplayWords`. Doxygen links an entity
  by its name; write `Display::SetPixel()` to link a member.
- A size that describes the code as it is now needs a check in `tools/documented-sizes.py`, as any
  other size in this repository does; see CLAUDE.md.

### What is documented

- **Everything a header offers**, documented whole: every class, every public and protected
  function, member and type, every enum value and every macro, the configuration macros
  included.
- **A private member or a function local to a `.cpp`** has no Doxygen block. Where there is
  something to say, a `//` or `/* */` comment says it at the definition.
- **A function that only passes a call on** takes the block of the function it calls with
  `\copydoc`, rather than a second text that would drift from it. **Overloads** that differ only
  in their parameters share one text the same way.
- **A class that every backend implements**, such as `Pixels` or `System`, is documented in each
  backend, as each says how it does it on its hardware.
- **Tests and the stubs of the host tests** keep plain comments.

### A file

Every file of `firmware/` and `platform/` outside the tests starts with the licence, as `//` lines
that tools such as REUSE read, and then a block for the file:

```cpp
// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The display: the pixels of the letter grid, with brightness, colour and current limit.
 */
```

The year is the one of the file's first commit. The brief of a header says what it declares; the
brief of a source with a header of its own says what the source adds, "The steps of the fade
animation."

## Checking

- **`tools/docs.py`** has Doxygen read the blocks, once per backend, and fails on every warning: a
  block Doxygen cannot read, a parameter without its `\param`, an entity without a block. It
  writes what Doxygen made of the blocks into `build/doxygen/<backend>/html`.
- **clang's `-Wdocumentation`**, through clang-tidy in `static-analysis.yml` and through clangd,
  checks each block against the declaration it stands at: a `\param` whose name the function does
  not have, a `\return` on a function that returns nothing, an empty command, an unknown command.
  clangd shows these in the editor as you type. A backslash meant as a char is written `\\`, or
  clang takes it for a command.

- **`tools/doc-indent.py`** checks that the lines continuing a command are indented under its
  text.

All three run in `static-analysis.yml` on every pull request.

### Until every module is written

The modules are written in this style one at a time, each in its own pull request. Until then a
module keeps the blocks it had, at the definitions in the `.cpp`, and its members may lack one:
`PENDING` in `tools/docs.py` lists those modules, and a module comes off the list in the pull
request that documents it. Every other warning fails already. `tools/doc-indent.py` leaves those
modules out, as their old lists of values in a `\return` would run together.

## Writing a block

The dev container has the extension "Doxygen Documentation Generator". Type `/*!` above a
declaration and press Enter: it writes the skeleton of this style, with a `\param` for each
parameter and a `\return` for a function that returns a value. The text is yours to write.
