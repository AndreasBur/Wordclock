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

CI reads four builds with the same checks: the simulator's, and each hardware backend's
through `tools/cross-clang-tidy.py`, which rewrites a GCC cross build's compile database so
that clang can follow it. The firmware core is read in every one of them, as each compiler
sees it.

A function using a mutable referenced object is still reviewed for its meaning: compiling
with const does not by itself make a setter a read-only operation.

## Switched off, with the reason

Three CLion suggestions are disabled in `.idea/editor.xml`:

- `CppParameterMayBeConst`: top-level const on a value parameter is primarily a local
  implementation convention, not an API guarantee.
- `CppPassValueParameterByConstReference`: small firmware values can be cheaper to pass by
  value on the AVR. Review large copies individually.
- `CppUseDesignatedInitializers`: the project builds as C++17.

These clang-tidy suppressions sit in the code rather than in `.clang-tidy`, because they
are about particular classes and not about the check:

- The wxWidgets event handlers in `platform/simulator/src/PixelsFrame.cpp` stay non-static
  because the event tables require member-function pointers.
- The interface the firmware core calls through `getInstance()` - `Storage`, `System`,
  `Pixels`, the web interface, the clock chip - stays made of instance methods in every
  backend, although many implementations read no member. Whether one backend needs its
  members for a call is that backend's detail; answering the check would carry it into the
  interface. The simulator's stand-ins keep the same shape for the same reason. Private
  helpers are not covered by this: one that reads no member is `static` - except the AVR's
  `SerialPort::write`, which cost 72 bytes of flash as one.
- Methods that drive hardware - `Twi::write`, `WS2812::send`, `performPendingRestart()` and
  their kind - stay non-const. Nothing in them writes a member, but a const would say they
  change nothing.
- The STL headers in `platform/avr-dx/include/stl/` are the standard library on a
  toolchain that ships none, so declaring into `namespace std` is their purpose and
  `cert-dcl58-cpp` is suppressed around each namespace. In the code rather than in a
  `.clang-tidy` of that directory, because clang-tidy configures a header by the source that
  includes it: a directory's configuration reaches the editor, which opens the header on its
  own, but not a run over the build.

Two backends switch one check off in a `.clang-tidy` of their own:
`clang-analyzer-core.FixedAddressDereference` in `platform/avr-dx/` and `platform/rp2350/`.
A peripheral register is a dereferenced constant address, and there is no other way to
write to one.

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
