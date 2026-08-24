# Host tests for the ESP32 backend

```bash
platform/esp32/test/run.sh              # build and run the tests
```

Needs `g++` and `python3`. No ESP32 toolchain: the backend is compiled against the stand-ins
in [`stubs/`](stubs/) instead of the real framework.

```
test/
├── run.sh      builds everything once into an object cache under .pio/, links, runs
├── cases/      one binary each, named after what it pins down. Two more come from
│            ../../test/, shared with the RP2350 backend - see its README for the rule
│            that decides which of the two places a case belongs in
├── stubs/      the framework as far as the backend touches it, plus the three
│               translation units that define what nobody inspects
└── console/    not a test: the firmware as a host process, with a page in front of it
```

## Why there is a second way to build this

`pio run` proves the backend compiles for the target. It cannot say what the code *does*,
and a board only answers the questions that involve hardware. These tests take the other
half: everything above the peripherals, where the interesting mistakes were.

Both halves have caught real bugs. The toolchain found a name that clashed with the
Arduino core's `class Server` and a `Serial` redefinition; these tests found the frame
layout and the injected-command path. Neither would have found the other's.

What is out of reach here, and needs a board: the WS2812 pulse timing, whether the BH1750
answers on its bus, whether SNTP arrives.

## What each test pins down

| | |
|---|---|
[`cases/frame_test.cpp`](cases/frame_test.cpp) | the bytes `Pixels::render()` hands the peripheral — channel order, index to offset, that an unchanged frame is not retransmitted, and that the master brightness blanks without touching the buffer |
[`../../test/cases/serial_test.cpp`](../../test/cases/serial_test.cpp) | that an injected command takes the same path as one typed on the wire, that the UART is served first, and that a line reaches the sink once per `println()` |
[`../../test/cases/ds3231_test.cpp`](../../test/cases/ds3231_test.cpp) | the clock chip in both directions: which register a read is aimed at, the temperature the overlay shows, the time block as BCD, and the two states — a stopped oscillator, a chip that was never written — that count as no time rather than as data |
[`cases/web_test.cpp`](cases/web_test.cpp) | the handlers, reached through the same registration call the server makes: a frame in, an answer out, the page as a gzip stream, the catalog as valid JSON, an oversized frame refused |

`run.sh` builds with `-Wall -Wextra -Werror`, so a warning fails the run.

## Looking at the pages is not here any more

There used to be a `serve` mode in `run.sh`, and a `console/` beside this file holding a host
build of the backend plus a node server to put in front of it. Both are gone. node did the
HTTP and the web socket there, and the host binary answered a line protocol, so nothing about
*this* backend's server was ever exercised by pointing a browser at it - what was, is what
`cases/web_test.cpp` drives directly.

The pages themselves are served by the simulator now, which needs no second process and no
node: `./build/bin/Wordclock` answers `http://localhost:8080/`. See
[`../../simulator/README.md`](../../simulator/README.md).

## The stand-ins

[`stubs/`](stubs/) holds only as much of the framework as the backend touches, under the
names it includes them by. Two rules kept them honest:

- **A test that wants to see what the backend sent defines that call itself.** The RMT
  functions live in `cases/frame_test.cpp` because it keeps the frame; the HTTP functions live in
  `cases/web_test.cpp` because it captures what was served. `stubs/stubs.cpp` has only what
  nobody inspects.
- **The hardware port is defined away from the platform's `Arduino.h`.** There the name
  `Serial` is the multiplexer, so [`stubs/hardware_port.cpp`](stubs/hardware_port.cpp) includes the
  stub directly, through the same define the real build passes in.
