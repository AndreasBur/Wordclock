# Wordclock — a German word clock (Wortuhr)

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

A word clock based on Arduino — a *Wortuhr*, which is what the German builds of
this idea are called: an 11×10 grid of letters that spells out the time in German
("ES IST FÜNF NACH ZEHN"), with animations, overlays (date, temperature, text)
and a serial command interface.

The same firmware drives WS2812 LEDs from an ESP32-S3, a Raspberry Pi Pico 2 W
(RP2350) or an AVR128DA48, and on the first two a phone configures it over WiFi:
two pages the clock serves itself, NTP time with a time zone, automatic
brightness from a light sensor, and firmware updates over the air.

![The simulator window, showing 16:20 as "ES IST ZEHN VOR HALB FÜNF"](docs/images/simulator.png)

The whole firmware runs on a PC as well: the simulator draws the matrix in a
window and routes the serial interface to the text boxes beside it, so commands
can be sent and answers read without any hardware. Above it shows 16:20, which
German says as *zehn vor halb fünf* — ten before half five, the kind of wording
the word tables have to cover.

## What it does

- **Time in words** — an 11×10 German letter matrix, driven from a font table the
  clock reads at runtime.
- **Boards** — ESP32-S3, Raspberry Pi Pico 2 W (RP2350), AVR128DA48: one firmware,
  one core, three backends.
- **LEDs** — WS2812 / NeoPixel, shaped by the RMT peripheral on the ESP32 and by
  the CCL on the AVR.
- **Configuration** — a web panel and a command console the clock serves itself
  over WiFi, and the same commands over the serial port.
- **Time source** — NTP with a time zone on the networked boards, a DS3231 on the AVR.
- **Brightness** — automatic, from a BH1750 light sensor, with a night switch.
- **Updates** — over the air, from the panel, on the networked boards.
- **No hardware needed** — the simulator runs the whole firmware on a PC and serves
  the same pages a clock does.

## Auf Deutsch

Eine Wortuhr, die die Zeit in deutschen Worten anzeigt — 11×10 Buchstaben,
WS2812-LEDs, Konfiguration per Handy über WLAN, Update over the air. Die
Firmware läuft auf ESP32-S3, Raspberry Pi Pico 2 W und AVR128DA48, und ohne
Hardware im Simulator auf dem PC. Die Dokumentation ist englisch, die Uhr
spricht deutsch.

## Repository layout

| Directory | Purpose |
|-----------|---------|
| [firmware/](firmware/) | **Single source of truth** for the clock logic — platform-agnostic (animations, clock, display, scheduler, overlays, communication). |
| [docs/](docs/) | Reference documentation: the [serial command reference](docs/serial-commands.md), the [font tables](docs/fonts.md), the [roadmap](docs/roadmap.md) of what is still open, the [comparison](docs/comparison.md) with the other published word clocks and the [measured decisions](docs/decisions.md). |
| [assets/](assets/) | The icon's SVG masters and the script that generates the `.ico`, the `.xpm` and `docs/images/logo.png` from them. |
| [web/](web/) | The two pages a networked clock serves: the panel at `/` for what is changed often, the console at `/console` for every command. Compiled into the firmware by [platform/scripts/embed_web.py](platform/scripts/embed_web.py) — the clock has nowhere to fetch anything from. See its [README](web/README.md). |
| [platform/simulator/](platform/simulator/) | wxWidgets desktop backend: renders the matrix in a window so the firmware can be developed and debugged on a PC. |
| [platform/esp32/](platform/esp32/) | On-device backend: WS2812 over the RMT peripheral, time from NTP. Built with PlatformIO — see its [README](platform/esp32/README.md). |
| [platform/rp2350/](platform/rp2350/) | On-device backend for the Raspberry Pi Pico 2 W, derived from the ESP32 one. Built with PlatformIO — see its [README](platform/rp2350/README.md). |
| [platform/avr-dx/](platform/avr-dx/) | On-device backend for the AVR128DA48: WS2812 shaped by the CCL, time from a DS3231. Built with CMake and a cross toolchain file — see its [README](platform/avr-dx/README.md). |
| [tools/](tools/) | [documented-sizes.py](tools/documented-sizes.py), which compares every size this repository claims against a build and fails the pull request that made one stale. |

The tool that generates the bitmap font tables lives in its own repository,
[theAndreas/FontCreator](https://github.com/theAndreas/FontCreator) — see
[docs/fonts.md](docs/fonts.md) for the table format and how to regenerate one.

## Architecture

The firmware core reaches the hardware **only through header names**
(`Arduino.h`, `Pixels.h`, `RealTimeClock.h`, `BH1750.h`, `DS3231.h`, `Storage.h`, `System.h`,
`PowerSwitch.h`), resolved via the include path. Each platform under `platform/` supplies those headers with its own
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
Linux). See the [simulator README](platform/simulator/README.md) for details.

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

The pages in [web/](web/) need a clock behind them to show anything, and **the simulator is
one**: it serves them itself, on localhost, from the binary that draws the window.

```bash
cmake --build build && ./build/bin/Wordclock     # http://localhost:8080/
```

The panel is at `/` and the console at `/console`, answered out of the same
[`WebFrontend`](firmware/inc/Communication/WebFrontend/WebFrontend.h) a clock answers them
from — so what the browser sees is what a clock would send, with no board and no second
process. The window and the browser then show the same clock, which is worth more than either
alone: the window is colour-accurate, the pages are what somebody actually uses.

`/update` is answered too, and honestly: nothing is installed, because a desktop has no second
partition — what it stands in for is the panel's progress and its two outcomes, which is the
part of that card nothing else can reach without a board.

There used to be a second way, `platform/esp32/test/run.sh serve`, which put node in front of
a host build of the ESP32 backend. It is gone: node did all the HTTP and the web socket there,
so what it exercised was the pages and the firmware core — exactly what the simulator now does
with one binary and a real server.

## Checks

Every pull request runs two workflows, and the two badges above say separately
whether the code builds and whether it reads clean.

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) is the six ways this
repository can be built, each of which breaks on its own:

| Job | What it does |
|-----|--------------|
| Simulator and core tests | Configures and builds the wxWidgets backend with `-Werror` and runs `ctest` |
| ESP32 backend on the host | [`platform/esp32/test/run.sh`](platform/esp32/test/run.sh) — the backend against the stand-ins in `test/stubs`, no board needed |
| RP2350 backend on the host | [`platform/rp2350/test/run.sh`](platform/rp2350/test/run.sh) — the same arrangement for the Pico's backend |
| ESP32 firmware | `pio run` for the board it actually runs on |
| RP2350 firmware | `pio run` for the Pico 2 W |
| AVR Dx firmware | The cross build, plus [`tools/documented-sizes.py`](tools/documented-sizes.py) against the image it just produced |

A firmware job looks redundant next to its host tests and is not: those compile
the same sources with the host's compiler and libc, so anything the target's
toolchain has a different opinion about passes them. `timegm`, which newlib does
not declare, broke the firmware while every host test stayed green. That argument
carries furthest for the AVR, an 8-bit freestanding target with more opinions than
either of the others.

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
