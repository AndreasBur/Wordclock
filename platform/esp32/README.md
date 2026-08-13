# ESP32 platform

The on-device backend of the Wordclock firmware. It supplies the headers the core
reaches the hardware through — `Pixels.h`, `RealTimeClock.h`, `BH1750.h`, `Storage.h`
and `Arduino.h` — plus the application entry point. See
[../hardware/README.md](../hardware/README.md) for the contract these fulfil.

Its `Arduino.h` is not a reimplementation: it includes the Arduino core's own and
replaces exactly one thing, `Serial`. See *A second front end reaches the same port*
below for why.

The clock logic itself is not copied in here. `platformio.ini` compiles
[`../../firmware/src`](../../firmware/src) directly and
[`scripts/firmware_includes.py`](scripts/firmware_includes.py) puts its header
directories on the include path, so there is one copy of the firmware, shared with the
simulator.

## Building

```bash
pio run -d platform/esp32                 # build
pio run -t upload -d platform/esp32       # build and flash
pio device monitor -d platform/esp32      # serial console, 115200 baud
```

CMake refuses this platform on purpose — `-DPLATFORM=esp32` prints the PlatformIO
command instead.

**Arduino core 3.x (ESP-IDF ≥ 5.1) is required**, because `Pixels.cpp` uses the IDF 5
RMT driver `<driver/rmt_tx.h>`. Core 2.x ships IDF 4.4, which has only the older
`<driver/rmt.h>` with different types, so the build fails on that include rather than
misbehaving later. `platformio.ini` notes where a core 3.x platform comes from. The web socket handler additionally
needs `CONFIG_HTTPD_WS_SUPPORT` in the core's sdkconfig, which it enables by default - but
that, like the RMT names, is unconfirmed until the real toolchain has run.

### Verification status

This backend was written and checked without an ESP32 toolchain in reach. What has been
verified against stand-ins for the framework: every translation unit here plus the whole
firmware core compiles clean under `-Wall -Wextra` and links with no undefined symbols;
the frame `Pixels::render()` produces was checked byte for byte against a captured
transmission (channel order, index-to-offset mapping, dirty-flag suppression, master
brightness); and an injected command was driven through `Communication` to its answer,
`3 B=255 A=0 G=0`, which is what proves a second front end takes the same path as the
wire. The web socket handler was driven the same way, through the registration call
the server makes: a frame carrying `3 -B200` reached the parser and `3 B=200 A=0 G=0` came
back out over the socket, the page is served as a real gzip stream, and an oversized frame
is refused rather than truncated into the parser. What has **not** been run is the real toolchain, so the ESP-IDF API names and the
timing on an actual strip are still unconfirmed.

## Configuration

Everything site-specific lives in
[`include/WordclockConfiguration.h`](include/WordclockConfiguration.h): the network, the
NTP servers, the time zone. Each setting can be overridden with a `-D` build flag or
from a `WordclockSecrets.h` next to it, which is where the credentials belong — that
file is git-ignored:

```cpp
#define WORDCLOCK_WIFI_SSID     "..."
#define WORDCLOCK_WIFI_PASSWORD "..."
```

Without an SSID the clock still boots, says so on the console and runs without a
network. The time then stays unset, because there is no other time source (see below).

The LED data pin is **not** configured here. It comes from `DISPLAY_DATA_PIN` in
[`../../firmware/inc/Display/Display.h`](../../firmware/inc/Display/Display.h), like
every other display parameter, and the backend takes what `Display::init()` hands it.

## What the backend does differently from the simulator

| | Simulator | ESP32 |
|---|---|---|
| `Pixels` | writes into a window | WS2812 over the RMT peripheral, DMA-fed |
| `RealTimeClock` | counts a host clock forward | reads the system clock, which SNTP sets |
| `BH1750` | returns what a slider dialled in | reads the sensor over I²C |
| `Storage` | a file in the working directory | one blob in the NVS partition |
| `Serial` | routed into two text controls | UART0, plus characters a second front end injects |
| tick | wxTimer | `vTaskDelayUntil` in `loop()` |

Four details are worth knowing before changing anything here.

**The frame goes out once per tick, not from `show()`.** Several modules call
`Display::show()` within a single tick — DisplayManager, Animations, Text, Clock and two
command parsers all do — so transmitting from `show()` would put frames back to back on
the wire. WS2812 needs the line low for longer than 280 µs between frames, and losing
that gap does not blank the display; it shifts the next frame by a pixel, which reads as
a wiring fault. `show()` therefore only marks the buffer dirty, exactly as on the
simulator, and `Pixels::render()` transmits from the application's tick.

**`WordclockMain::init()` exists because nothing in the core calls `Display::init()`.**
The simulator gets away with that because its stand-ins need no setting up. On hardware
that call is what claims the RMT channel, and without it every `show()` is dropped in
silence.

**The light sensor sets itself up from its first `task()`.** `Illuminance` keeps its
`BH1750` private and its own `init()` is declared but never defined, so there is no
place in the core to initialise the sensor from. Doing it lazily also retries, which is
what a sensor that is not answering yet after power-on needs.

**A second front end reaches the same port.** `Communication` reads its commands one
character at a time out of `Serial`, and every answer goes back through it, so a web
socket needs no protocol of its own - it needs to reach that object. This platform's
`Arduino.h` therefore includes the core's and then binds `Serial` to `WordclockSerial`,
which reads the UART first and injected characters second, and hands each finished line
to a sink. Two consequences: the framework's own headers must be included **before** this
`Arduino.h` in any platform source, or the macro reaches into them; and the core's header
is reached through a path the build script passes in rather than through `include_next`,
which re-finds this file and lets its include guard swallow the real one.

## Web console

The clock serves a page at `http://wordclock.local/` - a console that speaks the same
commands as the wire, because its web socket is wired straight to the port `Communication`
reads from. Nothing about the protocol is repeated in the browser.

The page is [`web/index.html`](web/index.html), one self-contained file with its CSS and
script inline: the clock has nowhere to fetch anything from. It is **not** uploaded
separately. [`scripts/embed_web.py`](scripts/embed_web.py) gzips it at build time and emits
it as an array **into the build directory**, so `pio run -t upload` ships page and firmware
together and their versions cannot drift apart - the failure a second partition invites.
The generated header is a build product on purpose; a checked-in one rots the moment
someone edits the HTML and forgets to regenerate it. At the moment that is 4.8 KB of
source, 2.0 KB compressed, and the default partition table is untouched.

While the layout is being worked on there is no need to flash: open `web/index.html`
straight from disk and it asks for the clock's address instead of using its own host. The
edit cycle is then a browser reload.

## Hardware notes

- **Level shifting.** The ESP32's 3.3 V data line drives a 5 V WS2812 out of spec. It
  often works and then fails with temperature or a longer cable; a 74AHCT125 on the data
  line is the fix. 330–470 Ω in series and 1000 µF at the strip's supply belong there too.
- **Brown-out on inrush.** The classic failure is a reset when the strip's inrush sags
  the 5 V rail while WiFi is transmitting. Star wiring for the 5 V, and 470 µF at the
  module.
- **Supply sizing.** 110 LEDs at full white would draw 6.6 A, which a word clock never
  does. With a brightness cap 5 V / 3 A is comfortable.
- **I²C address.** `BH1750_I2C_ADDR` is 0x23 here, the datasheet's address for ADDR tied
  low (0x5C when high). Both the simulator stub and the xmega driver carry 0x76, which is
  neither — harmless in a stub that never opens a bus, but a sensor addressed that way
  stays silent.

## Known gaps

- **The overlays are not persisted.** Colour, brightness, clock mode, animation selection
  and speeds and the sensor calibration survive a restart; the overlay configuration and
  its text do not, because the stored format has no variable-length field yet. The time
  and date are not stored either — they come from the network.
- **No time source without the network.** No RTC chip is read, so between power-on and the
  first SNTP answer the display holds its default date. A DS3231 on the same I²C bus is
  the fix for the stromless case.
- **The web console has no command form yet.** Commands are typed as text; a form built
  from `MessageCatalog`, and a view of the letter grid, are the next step.
- **No authentication.** Anyone on the network can send commands. Fine behind a home
  router, not on an open network.
