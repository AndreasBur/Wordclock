# Wordclock

## Code conventions

These are observed throughout the existing code — follow them rather than
reformatting to a different taste. The layout itself is clang-format's: `.clang-format` is
Google's style with `const` right of what it qualifies, at 100 columns.

### Prefer small functions

Split an expression into named steps instead of leaving a block of inline
arithmetic. A step whose name explains it is worth a function even when it is
called exactly once, and the name then carries the meaning that a comment would
otherwise have to.

A helper of one expression lives in the class body in the header:

```cpp
static constexpr ColorType DimmColor(ColorType color, byte brightness) {
  return static_cast<ColorType>((static_cast<uint16_t>(color) * (brightness + 1u)) >> 8u);
}
```

`static` when the helper does not touch members, `constexpr` on top of that when
the body allows it — a helper calling a non-`constexpr` function (or `std::sqrt`)
stays plain `static`. The caller then reads as a pipeline:

```cpp
byte const level = ToLevel(GetWeightedIntensity(GetIntensity(pixel)));
```

### Header and source

A header includes what its declarations need, not what its bodies need. A body that
would pull a header in only for itself is defined in the `.cpp`, and so is anything
with more to it than a line or two — a state machine, a `Task()`, an `#if` ladder.
What stays in the class body:

- a template, because every caller has to see it — unless it is private and only the
  class's own `.cpp` instantiates it, as `Overlays::ShowNow`;
- a `constexpr` function, such as the helper above;
- the singleton's `GetInstance()`;
- an accessor: one statement on what the header has to declare anyway.

Inlining is not a reason to keep a body in the header. The AVR image is linked with
`-flto` and inlines across sources as readily as within one, and what a body costs
there is decided by the inliner, not by the file it is written in. What the header
does cost is every file that includes it: `DisplayManager.h` used to drag
`Animations.h`, `Overlays.h` and the clock into every one of its includers for four
private helpers.

A header with one includer is the exception. The command parsers' headers are each
included by `MsgCmdParser.cpp` and nothing else, so their bodies cost no other file an
include, and a source per parser would only add files: such a header keeps its bodies,
and gives them up when a second file includes it.

Moving a body moves its includes, and a file that leaned on one of them breaks only
once that include is gone. `make headers` - the template's `tools/self_contained_headers.py` -
compiles every header as the only include of an empty file, by each target's compiler and the
host's.

A source may lean on what its own header includes, as Google's rule allows, and includes
everything else it names itself: an animation that walks `DisplayWords` includes
`DisplayWords.h` rather than taking it through `Animation.h` and `Display.h`. That is
also why `misc-include-cleaner` stays off; `.clang-tidy` says what it would report.

### Naming

Google's names, the same as in MaxMatrix and Utl, and checked: `.clang-tidy` runs
`readability-identifier-naming`, so a name that breaks them fails the static-analysis job.

- Types, functions and methods `CamelCase`; `Get`/`Set` for accessors, `To` for
  conversions (`ToIndex`, `ToLevel`), `Is` for predicates (`IsIndexValid`, `IsAnimationValid`).
- Variables, parameters and public members `lower_case`; a private or protected member
  ends in `_` (`brightness_`). A setter's parameter is therefore the plain name -
  `SetBrightness(byte brightness)` assigning `brightness_` - with no prefix to tell them apart.
- Constants and enumerators `k` plus `CamelCase` (`kUnlitLevel`, `WordIdType::kEs`). The
  enumerations are `enum class`, so an enumerator is named without its type's word and
  turns into a number only through `ToUnderlying()`. A count that sizes arrays or bounds
  loops stays the enumeration's last enumerator - so the compiler keeps counting, behind
  `#if`s too - and is offered beside it as a number: `static constexpr byte
  kNumberOfWords{static_cast<byte>(WordIdType::kNumberOfWords)}`.
- `StdReturnType` is the exception and still a plain `enum`, `kEOk` and `kENotOk`.
- A name that belongs to someone else keeps its spelling and says so with a
  `NOLINT(readability-identifier-naming)` where it is declared: Arduino's `Serial`,
  `millis()` and `byte`, the standard library's `std::array`, the frameworks the host
  tests stub.
- The check does not see a name used inside a macro body: it neither renames nor
  reports it. Grep for the old spelling after renaming anything a macro reaches.

### Constants

Class-internal constants are `static constexpr`, not `#define`:

```cpp
static constexpr byte kUnlitLevel{192u};
```

`#define` is reserved for the configuration macros at the top of a header, which
are named after the module (`PIXELS_NUMBER_OF_LEDS`, `PIXELS_SUPPORT_DIMMING`).

### Comments

Comments say *why*, not what — the reasoning that is not recoverable from the
code, such as which alternative was rejected and what went wrong with it. Keep a
comment attached to the function whose behaviour it explains.

Two kinds, as in Utl and MaxMatrix: a Doxygen block `/*! ... */` above a declaration
in the header, for whoever calls it, and a comment on the implementation, `//` for one
or two lines and `/* ... */` with a ` * ` on every line from three. Every file starts
with the copyright, an SPDX line and a `\file` block. What a block holds, and how
`make docs` and `-Wdocumentation` check it, is
[docs/documentation.md](docs/documentation.md).

## Layout

- `src/` — the platform-independent clock, a folder per module, shared by every target.
- `test/` — the core's regression tests and fuzzers, built with the simulator's backend.
- `platform/simulator/` — wxWidgets stand-ins for the hardware (pixel matrix,
  light sensor, Arduino `Serial`), mirroring the hardware classes' public API.
- `.devcontainer/` — one `Dockerfile` plus a `devcontainer.json` per host
  (`linux` for X11, `wslg` for WSL2); shared editor settings live in the local
  `shared/` feature.

## Build

```bash
cmake --preset default
cmake --build --preset default       # -> build/host/bin/Wordclock
cmake --workflow --preset default  # configure, build and run the tests in one
```

`cmake --list-presets=all` names the others - `avr`, `clang`, `sanitize`, `coverage`, `fuzz`
and the `ci-` variants, which are the same with warnings as errors.

Everything else - the analysers, the host suites, the PlatformIO builds, the web smoke test -
is a target of the `Makefile`, which is the template's, or of `tools/project.mk`, which holds
Wordclock's own and which the Makefile includes; `make` lists them all. Every workflow step
that builds or checks something is `make <target>` and nothing else, so a flag belongs in
`CMakePresets.json` and a command in `tools/project.mk`, never in a workflow; the reason for a
command is written beside it there. Before pushing, run the targets of what changed - `make
lint` takes seconds, `make check` is all of CI and takes minutes, most of them PlatformIO.

## Delivering a change

Anything more than a trivial fix goes on a branch and arrives as a pull request, rather
than as a commit on `main`. `gh` is in the dev container for that, so opening one costs
a command:

```bash
git switch -c <branch>
# commit there, then
git push -u origin <branch>
gh pr create --fill
```

A stale line in a document, a typo, a comment that no longer matches its code — those may
still go straight to `main`. Anything that changes behaviour does not.

What the pull request body has to carry is what the commits already argue: what moved,
why that way rather than the obvious alternative, and what was measured. A body that
repeats the diff is worth nothing to a reviewer; the numbers - flash on the AVR, which
targets were built, which cases were made to fail on purpose - are the part nobody can
reconstruct from the code.

## Sizes in prose

Every size written in this repository has been wrong at least once, always the same way: the
code grew and the sentence did not. So a number that describes **the artefact as it is now**
is enforced rather than remembered — `tools/documented-sizes.py` compares the documentation
against a build, each firmware CI job runs it for its own target, and `--fix` writes the
measured values in. One flag per target, and a target left out is simply skipped:

```bash
tools/documented-sizes.py --elf build-avr/programs/Wordclock.elf        # check
tools/documented-sizes.py --elf build-avr/programs/Wordclock.elf --fix  # write
tools/documented-sizes.py --rp2350-elf platform/rp2350/.pio/build/wordclock/firmware.elf
tools/documented-sizes.py --esp32-bin  platform/esp32/.pio/build/wordclock/firmware.bin \
                          --esp32-elf  platform/esp32/.pio/build/wordclock/firmware.elf
```

The ESP32 takes two paths because its flash figure is the size of `firmware.bin` while its
RAM figure comes off the ELF. That is not an inconsistency to tidy away: adding the S3's
sections up counts `.ext_ram.dummy` and two other padding sections as real, which is more
than a megabyte that is not in the image.

What may be enforced is decided by **what the number is a property of**. The two pages'
sizes are exact, because nothing but this repository's own bytes decides them. The AVR image
is a percentage with a point of slack, because the byte count belongs to the compiler in front
of you: 48 346 here, 48 152 in CI, for the same source - the first version of the script
pinned the byte count and duly failed on CI.

Adding a sentence with a size in it means adding a check to that script, or writing the
sentence so it needs none. Two kinds of number deliberately have none:

- **A size that describes a past change** — "82 bytes of flash for the night switch" — is
  history. It was true when it was measured and updating it would make it a lie.
- **A comparative claim** is written as the difference and not as both absolutes: "`-flto` is
  worth 10 KiB" needs no second build to stay true, where "58 778 bytes without it" would.

## Looking at the two web pages

`web/app.html` — the **panel**, served at `/` — and `web/index.html` — the **console**, at
`/console` — are what a clock hands out to a browser, and unlike the wx window below they can
be **driven**: clicks, typing and file pickers all work. The dev
container ships Playwright with its own Chromium for that — `apt` on this base offers only a
snap stub, which does not run in a container. It is headless, so none of the display
plumbing the next section works around applies here.

```bash
# Playwright from the tool environment, at uv.lock's version; its browser is the image's
uv run python shot.py
```

They need the firmware behind them, and **the simulator is it**: `./build/host/bin/Wordclock`
answers `http://localhost:8080/` — the panel at `/`, the console at `/console`, `/commands`,
`/display`, the web socket and an honest `/update` that installs nothing. It is started the
way the section below starts the window, `( ... &)` inside the call, and killed with
`pkill -x Wordclock`; the same X11 rules apply, because it is the same binary.

Four things have to line up, and each fails as a timeout that says nothing:

1. **Never wait for `networkidle`.** Both pages hold a web socket open, so neither ever goes
   idle and `page.goto` dies at its 30 s timeout. Use `wait_until="load"`, then wait for
   `#plate:not([hidden])` on the panel or `#panel:not([hidden])` on the console: that selector
   appearing *is* `/display` having answered. The very first load after the server starts can
   miss even a 20 s wait on that selector and every load after it be instant — retry once
   before reading anything into it, the way the MIT-SHM error below is retried.
2. **The header reads "connected"**, so a wait for the absence of "connecting" matches it and
   never fires. Wait on the plate, or on `.lit` inside it for the first frame off the socket.
3. **Waiting for the plate is not waiting for the values.** The letters come from `/display`
   and every field from the catalog and then a round of answers over the socket, so a script
   that reads a slider a second after the plate appeared reads its unfilled default — 128 for
   a range, black for a colour. Two seconds is enough here; better still, wait for the field
   to hold something.
4. **Give it a real width.** The panel is a two-column desktop layout above 960 px and one
   column below, the console above 800, and the phone case is the one they exist for — 390 px
   is worth checking before 1280.

```python
page = browser.new_page(viewport={"width": 390, "height": 900}, color_scheme="dark")
page.goto(URL, wait_until="load")
page.wait_for_selector("#plate:not([hidden])", timeout=20000)   # "#panel" on the console
page.wait_for_timeout(2000)                                     # the answers, see 4 above
page.screenshot(path="shot.png", full_page=True)
```

Two things it is good for beyond a screenshot. **Measuring**, which is how the layout bugs
were found rather than by looking: `scrollWidth > clientWidth` per element finds what
overflows its card, and a page that scrolls sideways at 390 px is a bug however good it
looks. And **behaviour that needs input** — `set_input_files` on the update panel's picker
plus a click on Install exercises the whole upload, both answers included.

The first of them runs on every pull request: `make web`, which is `tools/web-smoke.py
build/host/bin/Wordclock`, starts the simulator under `xvfb-run` — no window on anybody's desktop, no
sandbox to disable — and checks both pages at 390 and 1280 px for console errors, sideways scrolling and a command
answered. Run it before reaching for a script of your own; write one for what it does not
check.

What none of them shows is **the colour that went to the strip**. Both pages take only
*whether* a pixel is lit from the frame — the bytes arrive already dimmed, and a word at low
brightness would be a dark grey on a dark ground — and the wx window renders brightness as a
grey level and drops the hue. The panel does paint its plate in a hue, but that hue is the
*setting* it read back from command 2 and not the frame, so it says what the clock was told
rather than what it sent. A colour is therefore still checked by reading the bytes in a test.

Two screenshot artefacts worth knowing: `full_page=True` renders a `position: sticky`
element at its scrolled position, so the clock panel appears halfway down the image on a
desktop width — it is not misplaced. And an enormous full-page image is downscaled to
illegibility on the way into a reply; shoot the viewport, or a single element with
`locator.screenshot()`, when something has to be read.

## Screenshotting the simulator

A layout change can be checked directly instead of asking someone to look. The tools
(`imagemagick`, `xdotool`, `x11-utils`) are in the dev container. Four things have to
line up, and each one leaves an empty window list or an empty call behind when it does
not — none of them says what went wrong:

1. **Disable the tool sandbox** for the call. Inside the sandbox a GUI process started
   from the shell is invisible to `xdotool`/`import`, which still connect to the real X
   server and report only the compositor's own windows — so it looks like the app never
   opened a window. A pure X11 client such as `display` behaves the same way, which is
   the quickest way to confirm the sandbox is what is in the way.
2. **`GDK_BACKEND=x11`.** The WSLg variants set `WAYLAND_DISPLAY`, so GTK renders to
   Wayland by default and no X11 tool can see the window.
3. **Start the app inside the same call**, as `(cmd &)`. A separate background call, or
   `setsid`/`nohup`/`disown`, gets killed with exit 144.
4. **Kill it with `pkill -x Wordclock`, never `pkill -f 'bin/Wordclock'`.** `-f` matches
   whole command lines, and the calling shell's own command line contains the pattern —
   so the shell kills itself, the call ends at exit 144, and everything after the `pkill`
   silently never runs. That also looks like the app never opened a window when the
   `pkill` sits at the start of the call.

```bash
(GDK_BACKEND=x11 ./build/host/bin/Wordclock > run.log 2>&1 &) && sleep 6
WIN=$(xdotool search --name "Wordclock Pixels" | head -1)
# Guard the empty case: `import -window ""` waits for the user to click a window and
# takes the call into its timeout instead of failing.
if [ -n "$WIN" ]; then
    import -window "$WIN" shot.png
    xdotool getwindowgeometry "$WIN"  # window size, useful for layout checks
else
    cat run.log                       # an X error here means the app died, not that it is hidden
fi
pkill -x Wordclock
```

Then read `shot.png`. Two limits: the window pops up on the user's real desktop, so keep
the runs short and kill the process afterwards; and **nothing in the window can be driven
synthetically** — Weston's X11 window manager sets no `_NET_ACTIVE_WINDOW`, so
`windowactivate`, key mnemonics and `xdotool click` all do nothing. Not only menus:
clicking a plain `wxButton` at its computed screen position does not open the dialog it
owns either, which is worth knowing before spending a run finding out. Anything that is
not on screen the moment the window opens has to be checked by hand.

An X error in `run.log` — `BadAccess ... MIT-SHM` in particular — is worth a plain retry
before it is taken as evidence against the change under test. It also comes out of a
wedged X connection, for instance after an earlier `import` was left waiting for a click,
and then disappears on the next run. Build the previous commit into a second directory and
run that to tell the two apart.
