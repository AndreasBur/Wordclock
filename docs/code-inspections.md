# Code inspections

CLion's inspections were run over the tree once and the findings worked through. The
snapshot itself is not kept: it is a few thousand findings carrying line numbers, and a
line number is wrong as soon as the next commit lands. What is worth keeping is the part
that does not rot - which inspections are switched off, and which findings were read and
deliberately not acted on. Both are below. Re-run the inspections from the IDE when you
want the current list.

## Enforced instead of remembered

The two findings that recur often enough to be worth a gate are clang-tidy checks rather
than a document: `readability-convert-member-functions-to-static` and
`readability-make-member-function-const` are on in `.clang-tidy`, so CI reports them on
every pull request. Functions that need no instance state can be static, and functions
that only read it can be const.

A function using a mutable referenced object is still reviewed for its meaning: compiling
with const does not by itself make a setter a read-only operation.

## Switched off, with the reason

Three CLion suggestions are disabled in `.idea/editor.xml`:

- `CppParameterMayBeConst`: top-level const on a value parameter is primarily a local
  implementation convention, not an API guarantee.
- `CppPassValueParameterByConstReference`: small firmware values can be cheaper to pass by
  value on the AVR. Review large copies individually.
- `CppUseDesignatedInitializers`: the project builds as C++17.

Two clang-tidy suppressions sit in the code rather than in `.clang-tidy`, because they are
about particular classes and not about the check:

- The wxWidgets event handlers in `platform/simulator/src/PixelsFrame.cpp` stay non-static
  because the event tables require member-function pointers.
- The stand-ins in `platform/simulator/` stay non-static because they mirror the API of the
  backends the clock really runs on, where the same call reaches a peripheral through
  instance state. `BH1750`, `Pixels`, `Storage` and `System` hold these methods byte for
  byte as the avr-dx, esp32 and rp2350 headers do, and clang-tidy only ever sees the
  simulator - so following it here would have made the simulator the one backend out of
  four with a different shape.

## Read and not acted on

- `optionShortName` in `MsgParameterParser::parse` is assigned in `STATE_OPTION_CHAR`
  before the parser enters `STATE_OPTION_ARGUMENT`.
- `words == words` in `clock_test.cpp` deliberately tests reflexivity of the overloaded
  equality operator.
- Ignoring a return value does not make a call side-effect free. Transformation and clock
  methods can change the display; inspect the meaning of their status result. Tests should
  check setup failures before checking the resulting frame.
- `value.toString(16)` in the colour formatter is intentional for numeric RGB channels. The
  JavaScript argument-count finding requires checking inferred types rather than removing
  the radix.

## What the new tests cover

The parser regression tests in `platform/test/cases/serial_test.cpp` cover empty messages,
invalid command numbers and integer overflow. The simulator animation tests exercise
construction and destruction of `AnimationSnake`. The persistence test checks both data
transfer and file-close errors so a failed flush cannot be mistaken for a successfully
damaged checksum.

The colour-dimming test covers every pair of byte-valued colour and brightness inputs. The
multiplication explicitly widens one operand to `uint16_t` before multiplying: on AVR,
multiplying two bytes otherwise uses signed 16-bit `int` and can overflow. Host tests alone
cannot detect that target-specific overflow, because their `int` is wider.
