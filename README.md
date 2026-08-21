# Wordclock

[![CI](https://img.shields.io/github/actions/workflow/status/AndreasBur/Wordclock/ci.yml?branch=master&label=CI&logo=github)](https://github.com/AndreasBur/Wordclock/actions/workflows/ci.yml)
[![static analysis](https://img.shields.io/github/actions/workflow/status/AndreasBur/Wordclock/static-analysis.yml?branch=master&label=static%20analysis&logo=github)](https://github.com/AndreasBur/Wordclock/actions/workflows/static-analysis.yml)
[![Last commit](https://img.shields.io/github/last-commit/AndreasBur/Wordclock?logo=github)](https://github.com/AndreasBur/Wordclock/commits/master)
[![Contributors](https://img.shields.io/github/contributors/AndreasBur/Wordclock?logo=github)](https://github.com/AndreasBur/Wordclock/graphs/contributors)

[![Platforms](https://img.shields.io/badge/platform-ESP32--S3%20%7C%20RP2350%20%7C%20AVR128DA48%20%7C%20Simulator-blue)](platform/)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
[![CMake](https://img.shields.io/badge/CMake-064F8C?logo=cmake&logoColor=white)](CMakeLists.txt)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-FF7F00?logo=platformio&logoColor=white)](platform/esp32/platformio.ini)
![Code size](https://img.shields.io/github/languages/code-size/AndreasBur/Wordclock)
[![License](https://img.shields.io/github/license/AndreasBur/Wordclock)](LICENSE)

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
| [docs/](docs/) | Reference documentation: the [serial command reference](docs/serial-commands.md), the [font tables](docs/fonts.md) and the [roadmap](docs/roadmap.md). |
| [assets/](assets/) | The icon's SVG masters and the script that generates the `.ico`, the `.xpm` and `docs/images/logo.png` from them. |
| [web/](web/) | The two pages a networked clock serves: the panel at `/` for what is changed often, the console at `/console` for every command. Compiled into the firmware by [platform/scripts/embed_web.py](platform/scripts/embed_web.py) — the clock has nowhere to fetch anything from. |
| [platform/simulator/](platform/simulator/) | wxWidgets desktop backend: renders the matrix in a window so the firmware can be developed and debugged on a PC. |
| [platform/esp32/](platform/esp32/) | On-device backend: WS2812 over the RMT peripheral, time from NTP. Built with PlatformIO — see its [README](platform/esp32/README.md). |
| [platform/rp2350/](platform/rp2350/) | On-device backend for the Raspberry Pi Pico 2 W, derived from the ESP32 one. Built with PlatformIO — see its [README](platform/rp2350/README.md). |
| [platform/avr-dx/](platform/avr-dx/) | On-device backend for the AVR128DA48: WS2812 shaped by the CCL, time from a DS3231. Built with CMake and a cross toolchain file — see its [README](platform/avr-dx/README.md). |

The tool that generates the bitmap font tables lives in its own repository,
[theAndreas/FontCreator](https://github.com/theAndreas/FontCreator) — see
[docs/fonts.md](docs/fonts.md) for the table format and how to regenerate one.

## Architecture

The firmware core reaches the hardware **only through header names**
(`Arduino.h`, `Pixels.h`, `RealTimeClock.h`, `BH1750.h`, `DS3231.h`, `Storage.h`), resolved via the
include path. Each platform under `platform/` supplies those headers with its own
implementation — a compile-time swap with no runtime cost. See
[platform/avr-dx/README.md](platform/avr-dx/README.md) for the contract.

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

The on-device platforms are not built with CMake. The `esp32` and `rp2350` platforms use
PlatformIO:

```bash
pio run -t upload -d platform/esp32
pio device monitor -d platform/esp32

pio run -t upload -d platform/rp2350     # hold BOOTSEL on the first upload
```

The `avr-dx` platform uses the same CMake, but cross-compiles — so its toolchain
has to be named when the build directory is created, because a compiler cannot be
swapped afterwards:

```bash
cmake -B build-avr -S . -DPLATFORM=avr-dx \
      -DCMAKE_TOOLCHAIN_FILE=platform/avr-dx/toolchain-avr.cmake
cmake --build build-avr
cmake --build build-avr --target flash
```

### The two web pages

The pages in [web/](web/) are the one part of this project the simulator window cannot show,
and they need a clock behind them to show anything. That clock can be a host process:

```bash
platform/esp32/test/run.sh serve 8080
```

This compiles the ESP32 backend against the stand-ins in `platform/esp32/test/stubs/` and puts
a small server in front of it, so `http://localhost:8080/` is the panel and `/console` the
console — both read from `web/` on every request, so an edit is a browser reload — while
`/commands`, `/display`, `/update` and the web socket are answered by the real firmware core.
What the browser sees is therefore what a clock would send, without a clock.

The port is optional and defaults to 8080. Without the `serve` argument the same script builds
and runs the backend's host tests instead.

## Checks

Every pull request runs two workflows, and the two badges above say separately
whether the code builds and whether it reads clean.

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) is the three ways this
repository can be built, each of which breaks on its own:

| Job | What it does |
|-----|--------------|
| Simulator and core tests | Configures and builds the wxWidgets backend with `-Werror` and runs `ctest` |
| ESP32 backend on the host | [`platform/esp32/test/run.sh`](platform/esp32/test/run.sh) — the backend against the stand-ins in `test/stubs`, no board needed |
| ESP32 firmware | `pio run` for the board it actually runs on |

The firmware job looks redundant next to the host tests and is not: those compile
the same sources with the host's compiler and libc, so anything the target's
toolchain has a different opinion about passes them. `timegm`, which newlib does
not declare, broke the firmware while every host test stayed green.

[`.github/workflows/static-analysis.yml`](.github/workflows/static-analysis.yml)
is what reads the code rather than building it — two analysers, because they are
good at different things:

| Job | What it does |
|-----|--------------|
| clang-tidy | Over the firmware core and the simulator backend, seeing exactly what the compiler sees through `compile_commands.json`. Which checks and why the others are off is in [`.clang-tidy`](.clang-tidy) |
| cppcheck | Parses the tree by itself, which is what reaches the template instantiations and container bounds the clang checks do not follow. Its two suppressions and their reasons are in [`.cppcheck-suppressions`](.cppcheck-suppressions) |

cppcheck reads `firmware/` only: without wxWidgets' own headers it stops at
`DECLARE_EVENT_TABLE` in the simulator's window class, which is the half
clang-tidy already covers through the compile database.

The same commands are what to run before pushing; nothing in them needs a display,
a board, or anything the dev container does not carry. The analysers are the ones
that need a configured build directory rather than a built one:

```bash
cmake -S . -B build -G Ninja -DPLATFORM=simulator -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
run-clang-tidy -p build -quiet 'firmware/|platform/simulator/src'
cppcheck --enable=warning,portability --std=c++17 --inline-suppr \
    --suppressions-list=.cppcheck-suppressions \
    $(find firmware/inc -type d | sed 's/^/-I/') \
    -Iplatform/simulator/include/sim -Iplatform/simulator/include \
    firmware/src firmware/inc
```

## License

[MIT](LICENSE) — Copyright (c) 2017-2026 Andreas Burnickl. The dependencies the
firmware links against keep their own licences: the Arduino cores are LGPL-2.1,
ESPAsyncWebServer is LGPL-3.0-or-later and wxWidgets carries the wxWindows
Licence. None of them is redistributed here; PlatformIO and CMake fetch them at
build time.
