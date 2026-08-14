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

### PlatformIO in the dev container

The container has no PlatformIO of its own yet. Installing it by hand:

```bash
sudo apt-get install -y --no-install-recommends python3 python3-venv
python3 -m venv ~/.pio-venv && ~/.pio-venv/bin/pip install platformio
export PATH="$HOME/.pio-venv/bin:$PATH"
```

Behind a TLS-intercepting proxy there is one trap worth knowing, because the error names
neither the proxy nor the cause: PlatformIO downloads with `requests` and passes its own
`certifi` bundle explicitly, so it ignores `REQUESTS_CA_BUNDLE` and fails with
`CERTIFICATE_VERIFY_FAILED: self-signed certificate in certificate chain` while `curl`
and `git` reach the same URL. Appending the container's trust store to that bundle fixes
it without turning verification off:

```bash
cat /etc/ssl/certs/ca-certificates.crt | \
    sudo tee -a "$(~/.pio-venv/bin/python -c 'import certifi;print(certifi.where())')" > /dev/null
```

The pioarduino platform builds a second virtual environment under
`~/.platformio/penv` with its own copy, which needs the same treatment.

CMake refuses this platform on purpose — `-DPLATFORM=esp32` prints the PlatformIO
command instead.

**Arduino core 3.x (ESP-IDF ≥ 5.1) is required**, because `Pixels.cpp` uses the IDF 5
RMT driver `<driver/rmt_tx.h>`. The registry's `espressif32` does **not** provide it —
version 7.0.1 still ships `framework-arduinoespressif32@3.20017`, which is core 2.0.17 on
IDF 4.4, and the build fails on exactly that include. `platformio.ini` therefore pins the
pioarduino fork of the platform, which carries core 3.3.11. Both halves of that were
measured, not assumed.

### Verification status

**Builds for real.** `pio run` produces a firmware image for the ESP32-S3 with **no
warnings** from any file in this repository:

```
RAM:    10.8%  of 320 KB
Flash:  17.5%  of 3.2 MB
```

Rounded on purpose: the exact byte count moves with every edit to the page, and a figure
that rots on each commit is worse than none.

A clean build takes about two minutes because it compiles the Arduino core alongside;
changing one file of ours is about seven seconds.

**And it is exercised**, by the host tests in [`test/`](test/README.md):

```bash
platform/esp32/test/run.sh              # build and run them
platform/esp32/test/run.sh serve 8080   # the console on localhost, firmware behind it
```

Those compile the backend against stand-ins for the framework, so they reach everything
above the peripherals: the frame `Pixels::render()` hands over, byte for byte; that an
injected command takes the same path through `Communication` as one typed on the wire; and
the handlers, driven through the same registration call the server makes. `serve` puts the
real firmware behind the page on localhost, which is how the browser side is worked on
without flashing.

**What no test here can reach is the hardware itself**: the pulse timing on a real strip,
whether the BH1750 answers on its bus, and whether SNTP arrives. Those need a board.

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
| `RealTimeClock` | counts a host clock forward | reads the system clock, which SNTP sets and the DS3231 fills in for until it does |
| `BH1750` | returns what a slider dialled in | reads the sensor over I²C |
| `DS3231` | returns what a slider dialled in, and "no chip" until a box is ticked | reads the clock chip's temperature registers over I²C |
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

Above the log it shows the panel itself: the letters come from `GET /display`, which the
clock generates from `DisplayCharacters`, and the colours arrive as binary frames on the
same socket - 330 bytes in the strip's own byte order, at most every 50 ms and only when
they changed. A client that connects to a standing display is sent the current frame at
once, or it would wait for the next change; on a word clock that can be five minutes.

Unlike the wx window this shows the **real colour**. That one renders a pixel's brightness
as a grey level and drops the hue, which is why the colour swap in `Pixel` could hide there
for as long as it did.

It also carries a command builder, and that form is not written down in the page either:
`GET /commands` serves `MessageCatalog` as JSON, and the page generates the dropdown, the
option rows, the ranges and the named values from it. The same table the simulator's
message builder derives its dialog from - so a command added there appears in both front
ends and on the wire at once, with nothing to keep in step by hand. The built command goes
into the input field rather than straight onto the wire, so it can still be corrected,
exactly as the wx builder's Insert does.

The page is [`web/index.html`](web/index.html), one self-contained file with its CSS and
script inline: the clock has nowhere to fetch anything from. It is **not** uploaded
separately. [`scripts/embed_web.py`](scripts/embed_web.py) gzips it at build time and emits
it as an array **into the build directory**, so `pio run -t upload` ships page and firmware
together and their versions cannot drift apart - the failure a second partition invites.
The generated header is a build product on purpose; a checked-in one rots the moment
someone edits the HTML and forgets to regenerate it. At the moment that is 18.5 KB of
source, 6.0 KB compressed, and the default partition table is untouched.

It follows the system's light or dark preference, with a button in the header that
overrides it and remembers the choice. Light is the base: the clock's own look is amber on
near-black, but the console is usually read next to other light windows.

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
- **The clock chip shares that bus.** `DS3231_I2C_ADDR` is 0x68 and cannot be changed —
  the chip has no address pins. It keeps the time over a power cut and measures a
  temperature, and both need the backup cell in its holder: without one it comes up
  reporting that its oscillator stopped, which is the firmware's signal to ignore what it
  counted. Note that what it measures is its own die temperature: in a closed case next to
  the supply it reads a degree or two above the room.
- **The chip stores UTC**, not the local time on the display. That is what makes the hour
  that occurs twice at the end of summer time unambiguous, and it means a chip read with
  another tool shows an offset rather than the wall clock.

## Known gaps

- **The overlays are not persisted.** Colour, brightness, clock mode, animation selection
  and speeds and the sensor calibration survive a restart; the overlay configuration and
  its text do not, because the stored format has no variable-length field yet. The time
  and date are not stored either — they come from the network.
- **A clock without a battery still comes up blank.** The DS3231 fills the gap between
  power-on and the first SNTP answer — but only if it has one to fill it with: a chip
  without a battery reports that its oscillator stopped, and its registers are then ignored
  on purpose.
- **No view of the letter grid yet.** The console shows the answers, not the display. The
  pixel buffer over the same socket is the next step.
- **No authentication.** Anyone on the network can send commands. Fine behind a home
  router, not on an open network.
