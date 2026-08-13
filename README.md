# Wordclock

A word clock based on Arduino: an 11×10 grid of letters that spells out the
time in German ("ES IST FÜNF NACH ZEHN"), with animations, overlays (date,
temperature, text) and a serial command interface.

![The simulator window, showing 16:20 as "ES IST ZEHN VOR HALB FÜNF"](docs/images/simulator.png)

The whole firmware runs on a PC as well: the simulator draws the matrix in a
window and routes the serial interface to the text boxes beside it, so commands
can be sent and answers read without any hardware. Above it shows 16:20, which
German says as *zehn vor halb fünf* — ten before half five, the kind of wording
the word tables have to cover.

## Repository layout

| Directory | Purpose |
|-----------|---------|
| [firmware/](firmware/) | **Single source of truth** for the clock logic — platform-agnostic (animations, clock, display, scheduler, overlays, communication). |
| [docs/](docs/) | Reference documentation: the [serial command reference](docs/serial-commands.md) and the [font tables](docs/fonts.md). |
| [platform/simulator/](platform/simulator/) | wxWidgets desktop backend: renders the matrix in a window so the firmware can be developed and debugged on a PC. |
| [platform/esp32/](platform/esp32/) | On-device backend: WS2812 over the RMT peripheral, time from NTP. Built with PlatformIO — see its [README](platform/esp32/README.md). |
| [platform/hardware/](platform/hardware/) | Atmel xmega backend — an [interface contract](platform/hardware/README.md) only, and no longer the intended target. |
| [Wordclock_xmegaForArduino/](Wordclock_xmegaForArduino/) | Existing hardware project (older firmware); source for the eventual hardware port. |

The tool that generates the bitmap font tables lives in its own repository,
[theAndreas/FontCreator](https://github.com/theAndreas/FontCreator) — see
[docs/fonts.md](docs/fonts.md) for the table format and how to regenerate one.

## Architecture

The firmware core reaches the hardware **only through header names**
(`Arduino.h`, `Pixels.h`, `RealTimeClock.h`, `BH1750.h`), resolved via the
include path. Each platform under `platform/` supplies those headers with its own
implementation — a compile-time swap with no runtime cost. See
[platform/hardware/README.md](platform/hardware/README.md) for the contract.

## Building

The quickest way needs nothing installed but VS Code: open the repository, run
**Dev Containers: Reopen in Container** and pick the configuration matching the
host — **Linux/X11** on native Linux, **WSL2/WSLg** when VS Code runs in WSL2.
The container brings the toolchain and wxWidgets, CMake configures on open, and
the WSLg configuration forwards the GUI to the Windows desktop without an extra
X server. See [.devcontainer/README.md](.devcontainer/README.md) for what the
configurations share, how to add a site-specific one on an internal base image,
and why changing the shared settings needs a container rebuild.

Without a container, pick a platform with the `PLATFORM` switch (default
`simulator`):

```bash
cmake -B build -S . -DPLATFORM=simulator
cmake --build build
ctest --test-dir build --output-on-failure
./build/bin/Wordclock
```

That way requires CMake ≥ 3.16, a C++17 compiler and wxWidgets 3.x (GTK on
Linux). See the [simulator README](platform/simulator/README.md) for details and
the Code::Blocks projects.

The on-device platforms are not built with CMake. The `esp32` platform uses
PlatformIO:

```bash
pio run -t upload -d platform/esp32
pio device monitor -d platform/esp32
```

The `hardware` (xmega) platform would use the AVR toolchain; see its README.

## History note

Several older, diverged copies of the firmware (`Wordclock/`,
`Wordclock_xmega_Template/`, and a `.7z` archive) were removed during cleanup.
They remain fully recoverable from the git tag `archive/legacy-forks`:

```bash
git checkout archive/legacy-forks -- <path>
```
