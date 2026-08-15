# AVR Dx platform (AVR128DA48)

The on-device backend the xmega project became. It runs the same
[`../../firmware/`](../../firmware/) core as the simulator and the ESP32, on an
AVR128DA48, and drives the strip the way the xmega did: the WS2812 pulse widths are
made in hardware, not counted in software.

This directory also carries the description of the platform seam, which is worth
reading whatever backend you are working on.

## How the platform seam works

The firmware core in [`../../firmware/`](../../firmware/) is platform-agnostic. It
reaches the hardware **only through header names**, resolved via the compiler's
include path:

```cpp
#include "Arduino.h"        // core runtime
#include "Pixels.h"         // LED matrix
#include "RealTimeClock.h"  // time source
#include "BH1750.h"         // ambient light sensor
#include "DS3231.h"         // temperature of the clock chip
#include "Storage.h"        // where the settings survive a restart
#include "System.h"         // the machine itself: uptime, network, restart
```

A platform backend is simply a directory that provides these headers (plus an
application entry point) and is placed on the include path instead of
[`../simulator/include/`](../simulator/include/). No `#include` in the core
changes between platforms — this is a compile-time swap with zero runtime cost
(no virtual dispatch, which matters on AVR).

The simulator backend in [`../simulator/`](../simulator/) is the reference
implementation to mirror.

## What a backend must provide

| Header / unit | Contract | Here |
|---------------|----------|------|
| `Arduino.h` | `byte`, `boolean`, `F()`, `PROGMEM`, `pgm_read_byte`, `memcpy_P`, `bitRead`, `itoa`, and a `Serial` object exposing `print` / `println` / `available` / `read` | avr-libc directly, plus a non-virtual `SerialPort` on USART1 |
| `Pixels.h` | `Pixels` singleton: `getInstance`, `setPixel(Fast)` / `clearPixel(Fast)` / `getPixel(Fast)`, `setBrightness`, `show`, `clearPixels`, `init(pin)` | Buffer and brightness; `render()` hands the frame to `WS2812` |
| `RealTimeClock.h` | `RealTimeClock` singleton holding a `ClockDateTime`; the core only *reads* it via `getDateTime()` | Read from the DS3231 once a second, written through on a command |
| `BH1750.h` | Ambient-light driver exposing the illuminance reading the core consumes | BH1750 over TWI1 |
| `DS3231.h` | `DS3231` with `getTaskCycle`, `task`, and `getTemperature(TemperatureType&)` in tenths of a degree Celsius. The return code is the contract: `E_NOT_OK` until a reading has arrived, which is what a board without the chip keeps answering and what keeps the temperature overlay away | DS3231 over TWI1, register handling shared with the ESP32 backend |
| `Storage.h` | `Storage` singleton with a `Capacity` and `read` / `write` / `clear` over one byte blob; the core owns the format inside it, so this only has to store what it is given and refuse a blob of another length | On-chip EEPROM, length byte in front of the blob |
| `System.h` | `System` singleton: `getFreeMemoryInKibibytes`, `getNetworkAddress`, `getLinkQuality` for the status command, and `restart` / `resynchroniseTime` / `reconnectNetwork` for the procedures. Every one of them may answer `E_NOT_OK`, and a backend that cannot do a thing has to — a clock with no network has no address, and the status command sends an empty field rather than a zero. `restart()` only asks; the application carries it out on its next tick, after the answer has gone out | Free memory is real; everything about a network answers `E_NOT_OK`. The uptime is not here: the core counts it in `Uptime`, off the scheduler's tick, because a backend dividing its own millisecond counter down is what used to wrap |
| app entry point | Equivalent of the simulator's `WordclockApp` / `WordclockMain`: initialise, restore the settings with `Persistence::load()`, then repeatedly tick `Scheduler::task()` and update the `RealTimeClock` | `main()` in [`src/main.cpp`](src/main.cpp) |

The tick has to come every `Scheduler::getTaskIntervalMs()` milliseconds: every
module's task cycle counts in that unit, so a tick at another rate silently
rescales animation speed, the sensor's sampling interval and the serial poll rate
alike. Read the value rather than repeating the number, as the simulator's timer
does; a backend whose timer cannot be set that freely changes
`SCHEDULER_TASK_INTERVAL_MS` instead. Note that the BH1750 needs roughly 120 ms
per high-resolution conversion, which its task cycle must stay above.

## Building

The same CMake as the simulator, with one difference: this backend cross-compiles,
so its toolchain has to be named when the build directory is created. A compiler
cannot be swapped afterwards, which is why `-DPLATFORM=avr-dx` on its own is not
enough — and says so rather than failing later on a host compiler that does not
know `-mmcu`.

```bash
cmake -B build-avr -S . -DPLATFORM=avr-dx \
      -DCMAKE_TOOLCHAIN_FILE=platform/avr-dx/toolchain-avr.cmake
cmake --build build-avr
cmake --build build-avr --target flash     # over UPDI, via pymcuprog
```

**This needs `avr-gcc` 12 or newer.** Not a preference — an older one fails twice,
and the second failure is the one that cannot be worked around:

- It has no device-specs for the part: `avr-gcc: error: device-specs/specs-avr128da48:
  No such file or directory`. A Microchip device pack fixes that, see below.
- It rejects the shared core: `temporary of non-literal type 'ClockWords' in a
  constant expression`, from the `constexpr` constructors the firmware uses
  throughout. GCC 12 accepts them. Nothing short of making the core worse fixes
  that, so the answer is a newer compiler.

Ubuntu 24.04 ships 7.3, so its `gcc-avr` will not do. Debian trixie and Ubuntu 26.04
carry 14.x, which is what CI uses and what this was developed against — and why the
dev container's base image is `ubuntu26.04` rather than the `ubuntu-24.04` it started
on. That container carries the toolchain, so nothing further is needed inside it. One
line says which one is in front of you:

```bash
avr-gcc --version | head -1
avr-gcc -mmcu=avr128da48 -E -x c /dev/null -o /dev/null
```

From 12 onwards no device pack is needed. For a Dx part even a current `avr-libc`
does not know, point `AVR_DFP` at an unpacked Microchip `.atpack` — it is also read
from the environment, so a container or a CI job can carry one without every command
naming it:

```bash
cmake -B build-avr -S . -DPLATFORM=avr-dx \
      -DCMAKE_TOOLCHAIN_FILE=platform/avr-dx/toolchain-avr.cmake \
      -DAVR_MCU=avr64dd32 -DAVR_DFP=/path/to/Microchip.AVR-Dx_DFP
```

`AVR_F_CPU` and `AVR_PROGRAMMER` are cache variables too. The size is printed after
every link rather than asked for, because the flash budget is the reason this part
was chosen at all — currently, of 128 KiB flash and 16 KiB RAM:

```
Program:   44016 bytes (34%)
Data:       1549 bytes (9%)
```

`-flto` is on by default and worth 9 KiB of that: the same build without it is
53 272 bytes. It fits either way here, but that margin is the difference between
fitting and not on a smaller part.

## Debugging

`-DCMAKE_BUILD_TYPE=Debug` builds with `-Og -g3` and without LTO — whole-program
inlining leaves breakpoints on lines that no longer exist and locals the target
cannot show, which is the opposite of what a debugger is for. `-g3` rather than
`-g` so the configuration macros are visible too: most of the timing here is a
macro derived from `F_CPU`.

That image is 67 030 bytes, so it still fits with room to spare — 52 % of the
flash rather than 34 %.

The part debugs over UPDI. On an AVR128DA48 Curiosity Nano the on-board nEDBG is
the debugger as well as the programmer, which is what `AVR_PROGRAMMER` defaults
to; an Atmel-ICE or a PICkit 4 does the same job on a board of your own.

Microchip Studio can debug the result without a project file of its own —
**File → Open → Open Object File For Debugging**, pointed at
`build-avr-dbg/platform/avr-dx/Wordclock.elf`. That keeps CMake as the single
place the sources and include paths are listed. Importing the tree as a Studio
project instead would mean maintaining those paths a second time, by hand, which
is exactly what the old `.cproj` next door drifted on.

*(Not verified here: Microchip Studio is Windows-only and no part of this
container. The build and the debug information it needs are.)*

## Everything in `PROGMEM` has to stay under 64 KiB

This is an 8-bit core with 128 KiB of flash — more than a pointer can address. On
this part `pgm_read_byte` compiles to a plain `lpm` through the 16-bit Z register:

```asm
ldi r30, 0xE8      ; Z = &Table
ldi r31, 0x00
lpm r24, Z         ; 16-bit Z, RAMPZ not involved
```

So **every table read that way has to live below 65 536** — the five fonts, the
word tables, the gamma table, the message catalog. Past that boundary the reads do
not fail, they wrap: the wrong bytes come back, and on a display that means wrong
pixels rather than an error.

There is room, and the linker helps by putting the read-only tables at the very
front — the fonts occupy `0xe8` to `0x1b9a`, 6.8 KiB of the total:

| | |
|---|---|
| highest flash symbol | 43 684 |
| ceiling for `lpm` | 65 536 |

Worth knowing before adding a sixth font, because nothing in the code says it. If
the image ever does grow past the boundary, the way out is `PROGMEM_FAR` and
`pgm_read_byte_far`, which carry the extra address bits in RAMPZ — at the cost of
a slower read on every access that uses them.

## The strip

The interesting part, and the reason this is an AVR Dx rather than a bigger xmega.

The xmega E5 shaped the WS2812 pulses with its XCL: a one-shot timer carrying both
pulse widths in `PERCAPTL` and `CMPL`, and a LUT muxing them against the data line,
fed by EDMA so the processor did nothing at all. XCL exists only in the E series,
which stops at 32 KiB of flash — and today's core alone needs more than that.

The AVR Dx has the LUT back, as CCL, so the idea survives:

```
USART0 (SPI host)   clocks the pixel bytes out at 800 kbit/s
XCK edge            starts both one-shots through the event system
TCB1                the short pulse, a zero bit
TCB2                the long pulse, a one bit
CCL LUT0            out = TXD ? TCB2 : TCB1, truth table 0xE4, output on PA3
```

Two timers rather than the E5's one, because a TCB carries a single compare value
where the XCL carried two.

What no AVR Dx has is DMA, so the bytes are fed by an interrupt — one per 10 µs,
about 3.3 ms for a full frame. That is affordable precisely because the pulse
shaping is in hardware: a late byte stretches the gap between bytes, it does not
deform a pulse. The gap is the budget instead, and it has to stay under the WS2812's
50 µs reset time, which is why the strip's interrupt is the one raised to
`CPUINT.LVL1VEC` and why every other handler in this backend is kept short.

### Pins

| | |
|---|---|
| `PA0` | USART0 TXD, into the CCL — *not* the strip |
| `PA2` | USART0 XCK, into the event system |
| `PA3` | **CCL LUT0 output — the strip's data line** |
| `PC0` / `PC1` | USART1 TXD / RXD, the console at 115200 baud |
| `PF2` / `PF3` | TWI1 SDA / SCL, the DS3231 and the BH1750 |

TWI1 rather than TWI0 is forced: TWI0 can only reach PA2 and PA3, which the strip
already uses.

## Status

Everything here builds and links. **The timing has not been on an oscilloscope.**
Before trusting a first board, check three things:

1. the two pulse widths, 333 ns and 917 ns at 24 MHz;
2. that the data bit is stable when the one-shots fire — `UCPHA` in
   [`src/WS2812.cpp`](src/WS2812.cpp);
3. whether the CCL sees the TCB waveforms without their pin outputs enabled. The
   data sheet does not say, and it is the one register decision here that was not
   derivable from it.

## Notes for anyone reading the old xmega project

The original lived under `Wordclock_xmegaForArduino/` until this backend replaced it.
It was removed once the port was done rather than kept as a reference: git keeps it,
and a dead Atmel Studio solution in the tree is one more thing a reader has to be told
to ignore. `git log -- Wordclock_xmegaForArduino` finds it, and

```bash
git ls-tree -r --name-only d6e356d -- Wordclock_xmegaForArduino
git show d6e356d:Wordclock_xmegaForArduino/Wordclock/ArduinoCore/library.cpp
```

reads a file out of it without checking anything out.

Two things in it no longer apply, which is most of why it is not needed:

- It builds with a GCC plugin, `avr-flash-vtbl`, to move C++ vtables out of RAM.
  Not needed here: the core uses CRTP and has no virtual function anywhere, and
  `SerialPort` deliberately has none either, so no vtable exists to move.
- Its STL comes from the vendored copy under `ArduinoCore/include/util/STL`. This
  backend brings its own minimal [`include/stl/`](include/stl/) instead — the
  vendored `numeric_limits::max()` is not `constexpr`, which the core now requires,
  and it has no `is_base_of` for the CRTP assertions.
