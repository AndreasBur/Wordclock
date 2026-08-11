# Wordclock Simulator

The wxWidgets desktop backend for the [Wordclock](../../README.md) firmware. It
renders the 11×10 letter matrix in a window so the clock, animations, overlays
and communication logic can be developed and debugged on a PC without flashing
hardware.

The simulator compiles the **shared firmware core** from
[firmware/](../../firmware/) and only supplies the hardware layer: the Arduino
core is emulated by a small shim in [include/Arduino.h](include/Arduino.h)
(`Serial`, `PROGMEM`, `pgm_read_byte`, `itoa`, …), and the LED matrix is drawn by
[src/Pixels.cpp](src/Pixels.cpp). See
[platform/hardware/README.md](../hardware/README.md) for the platform contract.

## What the window shows, and what it cannot

A lit letter is drawn as a grey between the unlit letter colour and black, so its
**brightness** is visible — that is what makes the brightness automatic, the `Fade`
animation and the trail of `Matrix` and `Collapse` readable here. The curve is
deliberately not proportional: unlit letters sit at light grey rather than at black,
which squeezes the dark end, so the low intensities are spread out to stay visible.

The **display color cannot be shown** on a light background: a white display color
at full brightness would be invisible. Only brightness is rendered, the hue is
dropped.

During a **text overlay** the grid is not a word clock at all but a 10×11 dot
matrix: the date or the temperature is pushed through it as a scrolled string, so
the letters that light up are only the cells the font's pixels fall on. Eleven
columns are narrow for that, and a long string needs many steps to pass through:
`-F` picks a smaller font, `-S` how fast it steps — the higher the faster, and `0`
stops it altogether. Give the overlay enough `-E` seconds for a full pass.

## Windows

**File → Settings** holds what stands in for hardware the PC does not have. So far
that is the **Illuminance** slider for the light sensor, which has nothing to
measure here; it only takes effect while the brightness automatic is on (`3 -A1`).

**File → Message**, or the **Create** button, puts a command together instead of
leaving it to be typed: pick it by name, tick the options it should carry, and
values that are an enumeration — the clock modes, the animations, the fonts — are
offered by name rather than by number. **Insert** writes the result into the input
field, where it can still be corrected before **Send** hands it over.

What comes back is read into a readable form underneath the raw line, in both
directions of the protocol:

```
3 B=255 A=1 G=0
   Display brightness
      Brightness = 255
      Automatic = on (1)
      Gamma correction = off (0)
Error=4:M
   Error = Value out of bounds (4)
   Option = M
```

The raw line stays, because that is what actually went over the wire — what
matters when the protocol itself is in doubt. Anything that is neither an answer
nor an error passes through untouched.

## Building

There are three ways to build, in order of preference.

### 1. Dev container (VS Code, recommended)

The [.devcontainer/](../../.devcontainer/) directory provides a ready-made
toolchain (Ubuntu 24.04 + wxWidgets 3.2), so nothing has to be installed on the
host. Open the repository in VS Code and run **Dev Containers: Reopen in
Container**. Select **Linux/X11** on native Linux or **WSL2/WSLg** when VS Code
runs in WSL2. CMake then configures on open with `PLATFORM=simulator`; build and
run the `Wordclock` target.

The WSLg configuration forwards the X11, Wayland and PulseAudio sockets, so the
GUI appears on the Windows desktop without an extra X server. Its WSL-only
mounts are isolated from the Linux configuration, where `/mnt/wslg` need not
exist.

A site-specific configuration on an internal base image can be added next to
them without being committed, and the settings the configurations share live in a
local feature that a rebuild has to pick up — see
[.devcontainer/README.md](../../.devcontainer/README.md) for both.

### 2. CMake on the host

Requires CMake ≥ 3.16, a C++17 compiler and wxWidgets 3.x (GTK build on Linux).

```bash
# Install wxWidgets (Debian/Ubuntu)
sudo apt install cmake ninja-build libwxgtk3.2-dev

# From the repository root — the PLATFORM switch selects this backend:
cmake -B build -S . -DPLATFORM=simulator -G Ninja
cmake --build build
./build/bin/Wordclock
```

wxWidgets is located via `find_package(wxWidgets)`, so the same
[CMakeLists.txt](CMakeLists.txt) works on Linux, macOS and Windows without
per-platform edits. This is also the build that runs inside the container.

### 3. Code::Blocks

Per-platform projects live in [codeblocks/](codeblocks/):

| File | Toolchain |
|------|-----------|
| `Wordclock_Linux.cbp`   | system `wx-config` |
| `Wordclock_Windows.cbp` | wxWidgets 3.1 (`$(#wx31)` global variable) |
| `Wordclock_MacOs.cbp`   | wxWidgets 3.0.3 (hard-coded path — adjust to your setup) |

These were the original debugging projects and are kept for that workflow; the
CMake build is otherwise authoritative.

## Layout

```
firmware/                 shared, platform-agnostic core (../../firmware)
platform/simulator/
├── include/              simulator-only HAL headers
│   ├── sim/              simulator implementations
│   │   ├── Pixels.h      wxWidgets LED-matrix frame
│   │   ├── RealTimeClock.h   time source
│   │   ├── BH1750.h      ambient-light sensor
│   │   ├── SerialShim.h  the port Serial is bound to
│   │   ├── Settings.h    stand-ins for absent hardware
│   │   ├── MessageBuilder.h  puts a command together
│   │   ├── MessageCatalog.h  what the commands are called
│   │   └── MessageDecoder.h  reads an answer back into names
│   ├── Arduino.h         Arduino-core shim (Serial, PROGMEM, itoa, …)
│   └── arduino/          split Arduino helper shims (types, bits, progmem, itoa)
├── src/                  their implementations
├── WordclockApp.*        wxApp entry point + 50 ms task timer
├── WordclockMain.*       wires the scheduler to the simulated real-time clock
├── CMakeLists.txt        simulator build (pulled in by the root switch)
└── codeblocks/           legacy Code::Blocks projects
```

## How it works

`WordclockApp` starts a 50 ms `wxTimer` that calls `WordclockMain::task()`,
which feeds the current wall-clock time into the firmware's `RealTimeClock` and
runs the `Scheduler`. That is all this layer does: what reaches the display is
decided by the firmware's `DisplayManager`, so the hardware backend will behave
the same without repeating any of it.

Every "LED" write ends up recolouring a `wxStaticText` cell in the matrix — lit
letters turn dark, unlit ones stay light grey. The right-hand console mirrors the
device's serial output and lets you send commands back: `Serial` is bound to
`SerialShim`, which writes into the two text controls the matrix window lays out
for it.

The firmware still includes the historical header names (`Pixels.h`,
`RealTimeClock.h`, `BH1750.h`), resolved by placing `include/sim` on the
compiler include path before `include`.
