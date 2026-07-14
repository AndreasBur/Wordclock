# Wordclock Simulator

A desktop simulator for the [Wordclock](../README.md) firmware, built with
[wxWidgets](https://www.wxwidgets.org/). It renders the 11×10 letter matrix in a
window so the clock, animations, overlays and communication logic can be
developed and debugged on a PC without flashing hardware.

The simulator compiles the **same firmware sources** as the real device (from
[Arduino/](Arduino/)) and swaps only the hardware layer for a wxWidgets
implementation. The Arduino core is emulated by a small shim in
[include/Arduino.h](include/Arduino.h) (`Serial`, `PROGMEM`, `pgm_read_byte`,
`itoa`, …), and the LED matrix is drawn by [src/Pixels.cpp](src/Pixels.cpp).

## Building

There are three ways to build, in order of preference.

### 1. CMake (recommended)

Requires CMake ≥ 3.16, a C++17 compiler and wxWidgets 3.x (GTK build on Linux).

```bash
# Install wxWidgets (Debian/Ubuntu)
sudo apt install cmake ninja-build libwxgtk3.2-dev

# Configure and build
cmake -B build -S . -G Ninja
cmake --build build

# Run
./build/bin/Wordclock
```

wxWidgets is located via `find_package(wxWidgets)`, so the same
[CMakeLists.txt](CMakeLists.txt) works on Linux, macOS and Windows without
per-platform edits.

### 2. Dev container (VS Code)

The [.devcontainer/](.devcontainer/) config provides a ready-made toolchain
(Ubuntu 24.04 + wxWidgets 3.2). In VS Code: **Dev Containers: Reopen in
Container** — CMake configures on open; build and run the `Wordclock` target.

On **WSL2/WSLg** the container forwards both the X11 and Wayland sockets, so the
GUI appears on the Windows desktop with no extra X server. See the comments in
[.devcontainer/devcontainer.json](.devcontainer/devcontainer.json) if you need
to switch the GTK backend to Wayland.

### 3. Code::Blocks

Per-platform projects live in [codeblocks/](codeblocks/):

| File | Toolchain |
|------|-----------|
| `Wordclock_Linux.cbp`   | system `wx-config` |
| `Wordclock_Windows.cbp` | wxWidgets 3.1 (`$(#wx31)` global variable) |
| `Wordclock_MacOs.cbp`   | wxWidgets 3.0.3 (hard-coded path — adjust to your setup) |

These were the original debugging projects and are kept for that workflow; the
CMake build is otherwise authoritative.

## Project layout

```
Wordclock_Simulator/
├── Arduino/            firmware sources shared with the hardware target
│   ├── inc/            headers (Animation, Clock, Display, Font, …)
│   └── src/            implementations
├── include/            simulator-only headers
│   ├── Arduino.h       Arduino-core shim
│   └── Pixels.h        wxWidgets LED-matrix frame
├── src/Pixels.cpp      matrix rendering + serial console
├── WordclockApp.*      wxApp entry point + 50 ms task timer
├── WordclockMain.*     wires the scheduler to the simulated real-time clock
├── CMakeLists.txt      cross-platform build
├── .devcontainer/      containerised toolchain (wxWidgets + WSLg GUI)
└── codeblocks/         legacy Code::Blocks projects
```

## How it works

`WordclockApp` starts a 50 ms `wxTimer` that calls `WordclockMain::task()`,
which feeds the current wall-clock time into the firmware's `RealTimeClock` and
runs the `Scheduler`. The scheduler drives the display, and every "LED" write
ends up recolouring a `wxStaticText` cell in the matrix — lit letters turn dark,
unlit ones stay light grey. The right-hand console mirrors the device's serial
output and lets you send commands back via the `Serial`/`Pixels` bridge.
