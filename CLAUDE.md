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

## Screenshotting the simulator

A layout change can be checked directly instead of asking someone to look. The tools
(`imagemagick`, `xdotool`, `x11-utils`) are in the dev container. Three things have to
line up, and each one silently produces an empty window list when it does not:

1. **Disable the tool sandbox** for the call. Inside the sandbox a GUI process started
   from the shell is invisible to `xdotool`/`import`, which still connect to the real X
   server and report only the compositor's own windows — so it looks like the app never
   opened a window. A pure X11 client such as `display` behaves the same way, which is
   the quickest way to confirm the sandbox is what is in the way.
2. **`GDK_BACKEND=x11`.** The WSLg variants set `WAYLAND_DISPLAY`, so GTK renders to
   Wayland by default and no X11 tool can see the window.
3. **Start the app inside the same call**, as `(cmd &)`. A separate background call, or
   `setsid`/`nohup`/`disown`, gets killed with exit 144.

```bash
(GDK_BACKEND=x11 ./build/bin/Wordclock > run.log 2>&1 &) && sleep 6
WIN=$(xdotool search --name "Wordclock Pixels" | head -1)
import -window "$WIN" shot.png
xdotool getwindowgeometry "$WIN"      # window size, useful for layout checks
pkill -f 'bin/Wordclock'
```

Then read `shot.png`. Two limits: the window pops up on the user's real desktop, so keep
the runs short and kill the process afterwards; and menus cannot be driven synthetically
— Weston's X11 window manager sets no `_NET_ACTIVE_WINDOW`, so `windowactivate`, key
mnemonics and clicks on menu items do nothing. Anything behind a menu still has to be
checked by hand.
