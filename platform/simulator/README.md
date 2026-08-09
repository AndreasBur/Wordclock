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

## Building

There are three ways to build, in order of preference.

### 1. CMake (recommended)

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
per-platform edits.

### 2. Dev container (VS Code)

The [.devcontainer/](../../.devcontainer/) directory provides a ready-made
toolchain (Ubuntu 24.04 + wxWidgets 3.2). Open the repository in VS Code and run
**Dev Containers: Reopen in Container**. Select **Linux/X11** on native Linux or
**WSL2/WSLg** when VS Code runs in WSL2. CMake then configures on open with
`PLATFORM=simulator`; build and run the `Wordclock` target.

The WSLg configuration forwards the X11, Wayland and PulseAudio sockets, so the
GUI appears on the Windows desktop without an extra X server. Its WSL-only
mounts are isolated from the Linux configuration, where `/mnt/wslg` need not
exist.

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
│   │   └── BH1750.h      ambient-light sensor
│   ├── Arduino.h         Arduino-core shim (Serial, PROGMEM, itoa, …)
│   └── arduino/          split Arduino helper shims (types, bits, progmem, itoa)
├── src/Pixels.cpp        matrix rendering + serial console
├── WordclockApp.*        wxApp entry point + 50 ms task timer
├── WordclockMain.*       wires the scheduler to the simulated real-time clock
├── CMakeLists.txt        simulator build (pulled in by the root switch)
└── codeblocks/           legacy Code::Blocks projects
```

## How it works

`WordclockApp` starts a 50 ms `wxTimer` that calls `WordclockMain::task()`,
which feeds the current wall-clock time into the firmware's `RealTimeClock` and
runs the `Scheduler`. The scheduler drives the display, and every "LED" write
ends up recolouring a `wxStaticText` cell in the matrix — lit letters turn dark,
unlit ones stay light grey. The right-hand console mirrors the device's serial
output and lets you send commands back via the `Serial`/`Pixels` bridge.

The firmware still includes the historical header names (`Pixels.h`,
`RealTimeClock.h`, `BH1750.h`), resolved by placing `include/sim` on the
compiler include path before `include`.
