# Wordclock

A word clock based on Arduino: an 11×10 grid of letters that spells out the
time in German ("ES IST FÜNF NACH ZEHN"), with animations, overlays (date,
temperature, text) and a serial command interface.

## Repository layout

| Directory | Purpose |
|-----------|---------|
| [firmware/](firmware/) | **Single source of truth** for the clock logic — platform-agnostic (animations, clock, display, scheduler, overlays, communication). |
| [platform/simulator/](platform/simulator/) | wxWidgets desktop backend: renders the matrix in a window so the firmware can be developed and debugged on a PC. |
| [platform/hardware/](platform/hardware/) | On-device backend (Atmel xmega) — currently an [interface contract](platform/hardware/README.md), not yet implemented. |
| [Wordclock_xmegaForArduino/](Wordclock_xmegaForArduino/) | Existing hardware project (older firmware); source for the eventual hardware port. |
| [FontCreator/](FontCreator/) | Tool that generates the bitmap font tables (`Font*.cpp`). |

## Architecture

The firmware core reaches the hardware **only through header names**
(`Arduino.h`, `Pixels.h`, `RealTimeClock.h`, `BH1750.h`), resolved via the
include path. Each platform under `platform/` supplies those headers with its own
implementation — a compile-time swap with no runtime cost. See
[platform/hardware/README.md](platform/hardware/README.md) for the contract.

## Building

Pick a platform with the `PLATFORM` switch (default `simulator`):

```bash
cmake -B build -S . -DPLATFORM=simulator
cmake --build build
./build/bin/Wordclock
```

Requires CMake ≥ 3.16, a C++17 compiler and wxWidgets 3.x (GTK on Linux). A ready
-made toolchain and a WSLg GUI setup are provided in
[.devcontainer/](.devcontainer/) — open the repo in VS Code and *Reopen in
Container*. See the [simulator README](platform/simulator/README.md) for details
and the Code::Blocks projects.

The `hardware` platform is built with the AVR toolchain, not CMake (see its
README).

## History note

Several older, diverged copies of the firmware (`Wordclock/`,
`Wordclock_xmega_Template/`, and a `.7z` archive) were removed during cleanup.
They remain fully recoverable from the git tag `archive/legacy-forks`:

```bash
git checkout archive/legacy-forks -- <path>
```
