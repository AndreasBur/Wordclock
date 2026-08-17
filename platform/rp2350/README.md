# RP2350 platform

The Wordclock on a Raspberry Pi Pico 2 W. The firmware core is not copied here: this
directory holds only the headers the core reaches for by name — `Pixels.h`,
`RealTimeClock.h`, `BH1750.h`, `DS3231.h`, `Storage.h`, `System.h` and the `Arduino.h`
shim — and the sources behind them. See [`../hardware/README.md`](../hardware/README.md)
for the contract these fulfil, and [`../esp32/README.md`](../esp32/README.md) for the
backend this one was derived from.

## Building

```bash
pio run -t upload -d platform/rp2350
pio device monitor -d platform/rp2350
```

The first build is slow in a way the others are not: the platform clones
`earlephilhower/arduino-pico` recursively, which is around 1.5 GB with the Pico SDK and
its submodules. Afterwards a full build is about ten seconds.

**The registry's `raspberrypi` platform will not do.** It knows RP2040 only — with it
installed, `pio boards` lists no `rpipico2w` at all, and the failure is that the board name
is simply unknown rather than anything that points at the cause. The `platformio.ini` here
points at the community fork, which is the same arrangement the ESP32 backend needs for its
own reason.

## Verification status

**Builds for real.** `pio run` produces a firmware image with **no warnings** from any file
in this repository:

```
RAM:    14.5%  of 512 KB
Flash:  11.3%  of 4 MB
```

Rounded on purpose: the exact byte count moves with every edit to the page.

The warnings are worth a note. On the ESP32 the HTTP server comes with the framework and is
built separately, so `-Wall -Wextra` never reaches it. Here the server is a `lib_deps` entry
and would be compiled with whatever stands in `build_flags`, which buries our own
diagnostics under a few hundred unused-parameter warnings from headers we do not own. Hence
`build_src_flags` rather than `build_flags` for the two warning switches.

**Nothing here has been on hardware.** No strip, no oscilloscope, no bus. Before trusting a
first board, check these in order:

1. **The two pulse widths**, 400 ns and 800 ns against a 1250 ns period. They come from
   25 state machine cycles at 20 MHz, split 8/8/9 — see `WS2812Pio.h`, where two
   `static_assert`s hold the arithmetic but nothing holds the four encoded instructions
   against the datasheet. A wrong side-set bit would produce an inverted or shifted
   waveform that looks like a wiring fault.
2. **That the frame reaches the strip in the right order.** The three colour bytes are
   left-aligned into bits 31..8 and shifted out from the top; if the shift direction is
   wrong, the display comes up in plausible but wrong colours rather than dark.
3. **Whether the BH1750 and the DS3231 answer** on a bus opened through `setSDA`/`setSCL`
   rather than through `begin()`'s arguments. The pin setters return a value that is
   checked, so a refusal is reported — but a bus that opens and stays silent is not.
4. **Whether SNTP arrives.** Time synchronisation is split here (see below), and the half
   that starts the servers runs from the tick rather than from `setup()`.

## What differs from the ESP32 backend

The two are the same program above the peripherals. Five things underneath are not.

- **`Wire` takes its pins separately.** `Wire.begin(sda, scl, freq)` is an ESP32 extension;
  here it is `setSDA`, `setSCL`, `begin`, `setClock`. The pin setters are what can refuse,
  so they carry the check that `begin()`'s return value carries there.
- **`Storage` is a file, not a key-value pair.** LittleFS rather than the EEPROM emulation,
  because `read()` has to refuse a blob of the wrong length and a file carries its own.
  `System`'s credentials go the same way, one file per key.
- **Power saving is a mode.** `WiFi.setSleep(bool)` becomes `defaultLowPowerMode()` or
  `noLowPowerMode()`.
- **There is no `setAutoReconnect`.** `reconnectNetwork()` joins again instead, which means
  a router that reboots needs the command where the ESP32 recovers on its own. That is a
  real loss rather than a detail, and the first thing to fix if this backend is ever the
  one in the wall.
- **Time synchronisation is in two halves.** `configTzTime()` has no counterpart: the zone
  rule is POSIX and has to be applied before the first tick or the display shows UTC, while
  the servers are resolved by name and can only be started once there is a link.
  `TimeSync.h` holds the pair so that neither call site does half of it.

The console is the same page and a third less code. `AsyncWebSocket` owns its client list,
counts it and broadcasts to it, so the descriptor array, its atomics and the walk over them
at every send are gone — `textAll()` is the whole of what they did.

## The waveform

`WS2812Pio` is to this backend what the RMT channel is to the ESP32 and what the timers and
the CCL are to the AVR-Dx: the pulse widths come out of hardware and the processor only
hands over a frame, by DMA, without waiting for it.

The timing is derived rather than copied. At 20 MHz a cycle is 50 ns, and the three phases
of 8, 9 and 8 make a 1250 ns bit in which a zero is high for 400 ns and a one for 800 —
the datasheet's nominal figures. The Pico SDK's own example uses 250/875 instead, which
also works but spends the whole tolerance on one side, and it is the far end of a long
strip with a slow edge that eats into what is left. The ESP32 backend made the same
decision against the IDF's example.

The program is four encoded instructions rather than a `.pio` source. The platform package
installs a stand-in for `pioasm` rather than the assembler, so an assembled program would
build here and not on the next machine; four instructions are short enough to read either
way, and each encoding is derived in the comment beside it.

## Hardware

The Pico 2 W drives its outputs at 3.3 V, so everything the ESP32 backend's hardware notes
say about the data line applies unchanged — the level shifter, the series resistor and
where it belongs, the capacitors and the supply sizing. See
[`../esp32/README.md`](../esp32/README.md#hardware-notes) rather than a second copy here.

One thing is easier: every GPIO on this part reaches a PIO block, so there is no range
reserved for the flash chip to route around.

## Gaps

- **No host tests.** The ESP32 backend compiles itself against stand-ins for the framework
  and exercises everything above the peripherals; this one has nothing equivalent yet. Two
  of that harness's four case files are platform-independent and want sharing rather than
  copying, which is a change to a working harness and has not been made.
- **No CI job**, for the same reason plus the 1.5 GB clone, which wants a cached toolchain
  before it is run on every push.
- **No `WordclockSecrets.h` handling difference**: the credentials work as on the ESP32,
  but that path has only been compiled, not run.
