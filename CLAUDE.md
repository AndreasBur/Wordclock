# Wordclock

## Code conventions

These are observed throughout the existing code — follow them rather than
reformatting to a different taste.

### Prefer small functions

Split an expression into named steps instead of leaving a block of inline
arithmetic. A step whose name explains it is worth a function even when it is
called exactly once, and the name then carries the meaning that a comment would
otherwise have to.

Such helpers live in the class body in the header, so they inline:

```cpp
static constexpr ColorType dimmColor(ColorType Color, byte Brightness) {
    return static_cast<ColorType>((static_cast<uint16_t>(Color) * (Brightness + 1u)) >> 8u);
}
```

`static` when the helper does not touch members, `constexpr` on top of that when
the body allows it — a helper calling a non-`constexpr` function (or `std::sqrt`)
stays plain `static`. The caller then reads as a pipeline:

```cpp
const byte Level = toLevel(getWeightedIntensity(getIntensity(Pixel)));
```

### Naming

- Functions `lowerCamelCase`; `get`/`set` for accessors, `to` for conversions
  (`toColour`, `toLevel`), `is` for predicates (`isIndexValid`, `isAnimationValid`).
- Variables, parameters and members `PascalCase`.
- A setter parameter that collides with its member is prefixed `s`
  (`setBrightness(byte sBrightness)` assigning `Brightness`).

### Constants

Class-internal constants are `static constexpr`, not `#define`:

```cpp
static constexpr byte UnlitLevel{192u};
```

`#define` is reserved for the configuration macros at the top of a header, which
are named after the module (`PIXELS_NUMBER_OF_LEDS`, `PIXELS_SUPPORT_DIMMING`).

### Comments

Comments say *why*, not what — the reasoning that is not recoverable from the
code, such as which alternative was rejected and what went wrong with it. Keep a
comment attached to the function whose behaviour it explains.

## Layout

- `firmware/` — the platform-independent clock, shared by both targets.
- `platform/simulator/` — wxWidgets stand-ins for the hardware (pixel matrix,
  light sensor, Arduino `Serial`), mirroring the hardware classes' public API.
- `.devcontainer/` — one `Dockerfile` plus a `devcontainer.json` per host
  (`linux` for X11, `wslg` for WSL2); shared editor settings live in the local
  `shared/` feature.

## Build

```bash
cmake -S . -B build -G Ninja -DPLATFORM=simulator
cmake --build build          # -> build/bin/Wordclock
```

## Delivering a change

Anything more than a trivial fix goes on a branch and arrives as a pull request, rather
than as a commit on `master`. `gh` is in the dev container for that, so opening one costs
a command:

```bash
git switch -c <branch>
# commit there, then
git push -u origin <branch>
gh pr create --fill
```

A stale line in a document, a typo, a comment that no longer matches its code — those may
still go straight to `master`. Anything that changes behaviour does not.

What the pull request body has to carry is what the commits already argue: what moved,
why that way rather than the obvious alternative, and what was measured. A body that
repeats the diff is worth nothing to a reviewer; the numbers - flash on the AVR, which
targets were built, which cases were made to fail on purpose - are the part nobody can
reconstruct from the code.

## Sizes in prose

Every size written in this repository has been wrong at least once, always the same way: the
code grew and the sentence did not. So a number that describes **the artefact as it is now**
is enforced rather than remembered — `tools/documented-sizes.py` compares the documentation
against a build, the AVR CI job runs it, and `--fix` writes the measured values in:

```bash
tools/documented-sizes.py --elf build-avr/platform/avr-dx/Wordclock.elf        # check
tools/documented-sizes.py --elf build-avr/platform/avr-dx/Wordclock.elf --fix  # write
```

What may be enforced is decided by **what the number is a property of**. The console page's
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
`/console` — are the one part of this project the simulator cannot show, and unlike the
wx window below they can be **driven**: clicks, typing and file pickers all work. The dev
container ships Playwright with its own Chromium for that — `apt` on this base offers only a
snap stub, which does not run in a container. It is headless, so none of the display
plumbing the next section works around applies here.

```bash
# the browser the image provides; PLAYWRIGHT_BROWSERS_PATH is already set by the image
/opt/playwright/bin/python shot.py
```

They need the firmware behind them, which `platform/esp32/test/run.sh serve <port>`
provides: it serves both pages from disk — the panel at `/`, the console at `/console`, the
same way a clock does — and answers `/commands`, `/display` and the web socket from a host
build of the real backend, so what the browser sees is what a clock would send. Five things
have to line up, and four of them fail as a timeout that says nothing:

1. **Start the server as a tracked background call**, not as `( ... &)`. `run.sh` keeps its
   binaries in a temporary directory and deletes them in a trap when its own shell exits, so
   a detached server still answers `/` from disk and then hangs forever on `/display` and
   `/commands` — which is where the panel and every settings row come from.
2. **Never wait for `networkidle`.** Both pages hold a web socket open, so neither ever goes
   idle and `page.goto` dies at its 30 s timeout. Use `wait_until="load"`, then wait for
   `#plate:not([hidden])` on the panel or `#panel:not([hidden])` on the console: that selector
   appearing *is* `/display` having answered.
3. **The header reads "connected"**, so a wait for the absence of "connecting" matches it and
   never fires. Wait on the plate, or on `.lit` inside it for the first frame off the socket.
4. **Waiting for the plate is not waiting for the values.** The letters come from `/display`
   and every field from the catalog and then a round of answers over the socket, so a script
   that reads a slider a second after the plate appeared reads its unfilled default — 128 for
   a range, black for a colour. Two seconds is enough here; better still, wait for the field
   to hold something.
5. **Give it a real width.** The panel is a two-column desktop layout above 960 px and one
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
(GDK_BACKEND=x11 ./build/bin/Wordclock > run.log 2>&1 &) && sleep 6
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
