# Host tests for the ESP32 backend

```bash
platform/esp32/test/run.sh              # build and run the tests
platform/esp32/test/run.sh serve 8080   # serve the console on localhost, firmware behind it
```

Needs `g++`, `python3` and — for `serve` — `node`. No ESP32 toolchain: the backend is
compiled against the stand-ins in [`stubs/`](stubs/) instead of the real framework.

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
[`frame_test.cpp`](frame_test.cpp) | the bytes `Pixels::render()` hands the peripheral — channel order, index to offset, that an unchanged frame is not retransmitted, and that the master brightness blanks without touching the buffer |
[`serial_test.cpp`](serial_test.cpp) | that an injected command takes the same path as one typed on the wire, that the UART is served first, and that a line reaches the sink once per `println()` |
[`web_test.cpp`](web_test.cpp) | the handlers, reached through the same registration call the server makes: a frame in, an answer out, the page as a gzip stream, the catalog as valid JSON, an oversized frame refused |

`run.sh` builds with `-Wall -Wextra -Werror`, so a warning fails the run.

## The local console

`run.sh serve` puts the real firmware behind the page:

- `/` serves [`../web/index.html`](../web/index.html) **from disk**, so editing it is a
  browser reload rather than a flash
- `/commands` is answered by the platform's own handler, so the browser gets what the
  device would send
- the web socket goes into `WordclockSerial::inject()`, and the answers come back out of
  the firmware's own line sink

[`webhost.cpp`](webhost.cpp) is the clock as a host process: it runs the whole firmware
core at the scheduler's real interval and speaks a line protocol on stdio.
[`serve.js`](serve.js) does HTTP and the web socket, framing included, so nothing has to
be installed for it.

## The stand-ins

[`stubs/`](stubs/) holds only as much of the framework as the backend touches, under the
names it includes them by. Two rules kept them honest:

- **A test that wants to see what the backend sent defines that call itself.** The RMT
  functions live in `frame_test.cpp` because it keeps the frame; the HTTP functions live in
  `web_test.cpp` and `webhost.cpp` because they capture what was served. `stubs.cpp` has
  only what nobody inspects.
- **The hardware port is defined away from the platform's `Arduino.h`.** There the name
  `Serial` is the multiplexer, so [`hardware_port.cpp`](hardware_port.cpp) includes the
  stub directly, through the same define the real build passes in.
