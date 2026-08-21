# RP2350 platform

The Wordclock on a Raspberry Pi Pico 2 W. The firmware core is not copied here: this
directory holds only the headers the core reaches for by name — `Pixels.h`,
`RealTimeClock.h`, `BH1750.h`, `DS3231.h`, `Storage.h`, `System.h` and the `Arduino.h`
shim — and the sources behind them. See
[`../avr-dx/README.md`](../avr-dx/README.md#what-a-backend-must-provide) for the contract
these fulfil, and [`../esp32/README.md`](../esp32/README.md) for the
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
RAM:     15 %  of 512 KB
Flash:   16 %  of the 3 MB the sketch is left
```

Percentages and not byte counts, and checked rather than remembered: `tools/documented-sizes.py
--rp2350-elf` measures them off the image the firmware job just built, with a point of slack,
which is the same shape the AVR's figure has and for the same reason - a byte count belongs to
the compiler in front of you. The flash is 3 MB and not the part's 4 because the network update
needed a megabyte of filesystem to write an image into; a percentage of the whole part would
flatter it.

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
   rather than through `begin()`'s arguments. Nothing here reports a refusal: a pin those
   setters will not take ends in the SDK's `panic()`, which is noreturn, so what the ESP32
   returns as an error halts the chip here instead. What holds is that both drivers name
   the same two pins — and a bus that opens and stays silent is not reported either.
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

The two pages are the same as the ESP32 backend's - the panel at `/`, the console at
`/console` - and a third less code here. `AsyncWebSocket` owns its client list,
counts it and broadcasts to it, so the descriptor array, its atomics and the walk over them
at every send are gone — `textAll()` is the whole of what they did.

## Updating over the network

`POST /update` takes the firmware as the request body, the same route the ESP32 answers and
the same panel in the page. What happens underneath is not the same at all, because this part
has no second app partition to write into:

1. The image is written into LittleFS as `/firmware.bin`.
2. `PicoOTA` writes a command page beside it, naming that file and carrying a checksum.
3. The clock restarts, and the **OTA loader that sits at the front of every image built here**
   — 10 028 bytes of `.ota` section ahead of the application, put there by the core's default
   linker script rather than by anything in this repository — copies the file into application
   flash, erases the command and reboots into what it just wrote.

What that buys is the property the ESP32's second slot gives for free: **the firmware being
written is never the firmware running**. So the command page is written last and only once, on
the chunk that completes the image and only if the file measures what the request announced.
No command page, no update — an interrupted upload leaves a file nobody reads and a clock that
comes back up on what it had. An upload that ends *short of its own Content-Length* is the one
case that can be cleaned up on the spot, and is: the image is removed and the answer says the
upload stopped.

**The filesystem is 1 MB for this, and it used to be 64 KB.** The image is some 490 KB, so
`board_build.filesystem_size` had to hold that with room to grow; 3 MB of the part's 4 are
left for the sketch, which uses 16 % of them. The region grows downwards from the end of
flash, which is the part worth knowing before installing this version: **its start moves, the
filesystem mounted at the old start is not found at the new one, and the settings go with
it.** So the release that introduces network updates is the one that has to be installed over
USB and resets the clock's settings once. Every update after it keeps them, and the reason
this is stated in three places — here, [`platformio.ini`](platformio.ini) and the release
notes — is that it cannot be undone by trying again.

What is *not* checked on a board: any of it. The handler and its refusals are exercised on the
host against stand-ins for LittleFS and PicoOTA, which is where the loader's own behaviour
stops being reachable — nothing here has watched a Pico boot into an image it was sent.

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

## And it is exercised

By the host tests in [`test/`](test/run.sh):

```bash
platform/rp2350/test/run.sh
```

Those compile the backend against stand-ins for the core, so they reach everything above
the peripherals: the frame `Pixels::render()` hands to DMA, word for word; that an injected
command takes the same path through `Communication` as one typed on the wire; and the
handlers, driven through the same registration call the server makes.

Two of the four case files are not here but in [`../test/`](../test/), shared with the ESP32
backend. Nothing in `ds3231_test.cpp` or `serial_test.cpp` is about either platform - they
compile whichever backend is being tested against whichever stubs it brings - so they live
once.

There is no `serve` mode here. The page is shared, so the host that puts a browser in front
of it only has to exist once: `platform/esp32/test/run.sh serve`.

## Gaps

- **The `WordclockSecrets.h` path has only been compiled, not run.** The credentials work
  as on the ESP32 as far as the tests reach, which is up to but not including a radio.
- **The first CI run of the firmware job will be slow**, because the 1.5 GB clone has to
  happen before there is a cache to restore.
- **The network update has never applied an image.** Everything up to the command page is
  checked on the host; the loader that reads it lives at the front of the image and has not
  been watched doing so. What that would take is one board, one USB install of a build with
  the 1 MB filesystem, and a second image sent to it over the network.
