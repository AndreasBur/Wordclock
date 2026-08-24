# ESP32 platform

The on-device backend of the Wordclock firmware. It supplies every header the core reaches the
hardware through, plus the application entry point. Which headers those are, and what each owes
the core, is the contract in
[../avr-dx/README.md](../avr-dx/README.md#what-a-backend-must-provide) — named there once rather
than listed again here, where a list drifts as the seam grows.

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

The image carries it, in a virtual environment under `/opt/platformio` that is already on
the path, so `pio run -d platform/esp32` works in a fresh container with nothing installed
by hand. The package cache is a named volume at `~/.platformio`: the roughly 1.5 GB of
toolchain survives a rebuild, and only the first run pays for fetching it.

Behind a TLS-intercepting proxy there is one trap worth knowing about, because the error
names neither the proxy nor the cause: PlatformIO downloads with `requests` and passes its
own `certifi` bundle explicitly, so it ignores `REQUESTS_CA_BUNDLE` and fails with
`CERTIFICATE_VERIFY_FAILED: self-signed certificate in certificate chain` while `curl` and
`git` reach the same URL. Both halves of it are already answered, which is why nothing here
has to be done about it: the image appends its own trust store to the bundle it ships, and
`platformio-trust-store` does the same on every start for the second environment the
platform packages build inside the volume at first use. The arrangement is described in
[`../../.devcontainer/README.md`](../../.devcontainer/README.md#platformio-for-the-esp32-backend),
which is where it belongs rather than as a second copy here.

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
RAM:     16 %  of 320 KB
Flash:   33 %  of the 3.19 MB app slot
```

Percentages and not byte counts, and checked rather than remembered: `tools/documented-sizes.py
--esp32-bin --esp32-elf` measures them off the image the build just produced, with a point of
slack, which is the same shape the AVR and RP2350 figures have and for the same reason - a byte
count belongs to the compiler in front of you.

It had to become a checked number. These two read 10.8 % and 17.5 % for a long time, from a
build before the two pages and the network update existed, while the app slot table further
down the same file put the image at three times that share - the two halves of one file
disagreeing by a factor of two. Nothing caught it, because the ESP32 was the one target whose
figures no check covered.

Flash is measured as the size of `firmware.bin` rather than by adding sections up, which is a
trap here specifically: the S3's ELF carries `.ext_ram.dummy`, `.flash_rodata_dummy` and
`.dram0.dummy`, padding and aliases that the loader never writes and that a naive sum counts as
more than a megabyte that is not there. The image file is not a reconstruction of the artefact,
it *is* the artefact.

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
the handlers, driven through the same registration call the server makes. `serve` puts the real
firmware behind both pages on localhost, which is how the browser side is worked on without
flashing - node is the HTTP and web socket half of it, and the host process contributes the
core, the catalog and the broadcast, so what it proves is about the pages rather than about
this backend's server.

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

Its default of GPIO 10 is chosen for the S3, where that pin is free. It is **not** free on
a classic ESP32-WROOM — GPIO 6 to 11 are wired to the flash chip there — so a board that
needs another pin says so in its own build rather than in the core header, which every
target shares:

```ini
build_flags =
    -DDISPLAY_DATA_PIN=13u
```

GPIO 13 because it has no strapping role, is not on UART0 (1 and 3) or the I²C bus (21 and
22), and can drive an output, which GPIO 34 to 39 cannot. Nothing in this repository sets
the flag — every board it has been built for has GPIO 10.

## Which ESP32 this needs

Written down because the question comes up with every board somebody already owns, and
because the answer is shorter than it looks: **the architecture is irrelevant**. There is no
Xtensa in the core or in this backend — no inline assembly, no intrinsics, plain C++17 — so
RISC-V parts are not a different case. Two things decide it:

- **WiFi**, for SNTP, the web console and the access point a clock with no credentials opens.
- **One RMT transmit channel**, for the strip.

| Part | WiFi | RMT TX × words | Verdict |
|---|---|---|---|
| ESP32-S3 | yes | 4 × 48 | what this backend targets — see the app slot table below |
| ESP32 (classic) | yes | 8 × 64 | builds; GPIO 6 to 11 are flash, so the data pin has to move |
| ESP32-S2 | yes | 4 × 64 | never built, nothing known against it |
| ESP32-C3 / C6 | yes | 2 × 48 | builds; both transmit channels go to the strip, see `RmtMemoryBlockSymbols` |
| ESP32-C5 | yes | 2 × 48 | never built, nothing known against it |
| ESP32-C2 | — | — | the pinned platform ships no C2 libraries, so it does not build here |
| ESP32-H2 | **no** | 2 × 48 | out. 802.15.4 only |
| ESP32-P4 | **no** | 4 × 48 | out. Radio would be a companion chip |

The WiFi column is `SOC_WIFI_SUPPORTED` in the IDF's `soc_caps.h`, which H2 and P4 do not
define; the channel counts are `SOC_RMT_TX_CANDIDATES_PER_GROUP` and
`SOC_RMT_MEM_WORDS_PER_CHANNEL` from the same headers.

**Flash is the one number that separates the parts that work.** What the image has to fit in
is the *app slot*, not the chip, and the same image is a very different fraction of each - the
percentages below are one build weighed three ways, and they are what
`tools/documented-sizes.py --esp32-bin` keeps honest:

| Board | App slot | Used |
|---|---|---|
| `esp32-s3-devkitc-1` (8 MB layout) | 3.19 MB | 33 % |
| `esp32dev`, default table | 1.25 MB | 85 % |
| `esp32dev`, `min_spiffs.csv` | 1.92 MB | 57 % |

So a 4 MB board is not short of flash, its default partition table is: it reserves 1.5 MB
for a SPIFFS this firmware never mounts. The web page is compiled into the image by
`../scripts/embed_web.py` and the settings live in NVS, so there is no filesystem to keep, and
one line moves it:

```ini
board_build.partitions = min_spiffs.csv
```

**None of this has been on hardware.** "Builds" here means the image links and the pins are
free, which is the same reservation the rest of this README carries.

## What the backend does differently from the simulator

| | Simulator | ESP32 |
|---|---|---|
| `Pixels` | writes into a window | WS2812 over the RMT peripheral, refilled from its interrupt |
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

## The two pages

The panel at `/` and the console at `/console` are shared with the RP2350 backend and
described where they live, in [`../../web/README.md`](../../web/README.md) - what each page
is for, how they find their commands, what the socket carries, and how they are worked on
without a board. What belongs here is only what this platform puts underneath them.

## Updating the firmware

`POST /update` takes the `.ota.bin` from a release - not the `.factory.bin`, which carries
the bootloader and is for a bare board. Where the update sits in both pages, and why the
image travels as the raw request body rather than a form, is with the pages; what happens
to it here is not shared.

The image is written to whichever of `app0` and `app1` is not running, and the bootloader is
pointed at it only once the last byte has arrived - so an upload that stops halfway leaves
the clock running exactly what it was running, and the panel says so rather than reporting a
failure. The partition table needs no change for this: the board's default already carries
two app slots of 3264 KB each with an `otadata` beside them, and the firmware uses a third
of one. The restart is asked for, not taken, so the answer reaches the browser first - the
same deferral RPC 31 uses.

## Hardware notes

### The LED data line

The ESP32's 3.3 V output drives a 5 V WS2812 out of spec: the datasheet asks for
0.7 × VDD, so 3.5 V, and 200 mV are missing. It works on the bench anyway, because the
input in fact switches around 2.7 V — and then fails with temperature, a longer cable or
the next batch of strips. An `SN74AHCT125N` on the data line closes the gap.

The T is the whole point. The HCT family kept the old TTL threshold of 2.0 V while
running off 5 V, so it reads 3.3 V logic and emits 5 V logic. A 74**HC**125 in the same
socket has CMOS thresholds and changes nothing.

| Pin | Connect to |
|-----|------------|
| 1 `1OE`  | GND — an open enable leaves the channel silent |
| 2 `1A`   | ESP32 data pin |
| 3 `1Y`   | 330 Ω, then the strip's DIN |
| 7 `GND`  | GND, common with the ESP32 and the strip |
| 14 `VCC` | **5 V**, not 3.3 |

100 nF across pins 14 and 7, as close to the package as it will go: at 5 ns edges the
output takes its current in spikes too short for the supply wire to deliver. The three
unused channels want their inputs tied rather than left floating — 5, 9 and 12 to GND,
4, 10 and 13 to 5 V — because a floating CMOS input draws current and can oscillate into
the channel that is doing the work.

The series resistor answers two problems, which is why it belongs at the driver's end
rather than at the strip. Fast edges see even 20 cm of wire as a transmission line, and
330 Ω at the source absorbs the reflection instead of letting it ring at DIN; that one
only costs wrong colours. The other can destroy something: data arriving while the strip
has no 5 V pushes current through DIN's protection diode into the unpowered rail.
[`../../firmware/inc/Communication/MessageParser/MsgCmdRemoteProcedureCallParser.h`](../../firmware/inc/Communication/MessageParser/MsgCmdRemoteProcedureCallParser.h)
already orders `POWER_OFF` to blank the strips before cutting the supply, but a reset or
a watchdog keeps no such order, and the resistor is what holds in that case.

Rejected on the way here: the cheap bidirectional boards are the wrong family. BSS138
pulls up through 10 kΩ, so the rising edge takes some 350 ns against a 400 ns "0" pulse,
and TXS0108E drives hard for a 30 ns one-shot before letting the same weak pull-up
finish, with 70 pF of rated load that a wire to the strip already exceeds. Both are built
for bidirectional buses, while this line runs one way and wants a push-pull driver.
Dropping the strip to 4.5 V with a series diode is the one real alternative — it puts
3.3 V inside spec at the cost of brightness. The "sacrificial first LED" is not one,
since that LED still receives the marginal level itself.

### Switching the strip's supply

RPC ids 20 and 21 switch the strip's 5 V, and the circuit they were written for did not have to
be invented. The [WC MiniDev Shield
v5](https://www.mikrocontroller.net/wikifiles/e/ea/WC_MiniDev_Shield_v5_Schaltplan.png) carries
one and everything below is read off it — a board for an STM32F103, so the pin is worth
nothing here and the topology is worth all of it.

One controller port, active high. It drives the gate of an N-channel BS170 through 82 Ω, and
the BS170's drain pulls the gate of a P-channel IRF9310 — high side, source on the incoming
5 V — down through 3k3. The 100 kΩ from the BS170's gate to ground is what makes that
polarity worth having: while the port is an input, which is what it is from reset until the
firmware has configured it, the BS170 is held off, the IRF9310's gate sits at 5 V and the
strip has no supply at all. A clock therefore comes up dark rather than in whatever the
strip's registers happened to hold.

The BS170 is there because 3.3 V logic cannot switch a P-channel high side on its own. Not
turning it on — a port pulled to ground gives the full −5 V — but turning it **off**, which
needs the gate at 5 V and is 1.7 V short of it. That lands inside the part's threshold band
rather than clear of it, so the MOSFET would be neither on nor off but dissipating somewhere
between, and which of the two depends on the sample. The port therefore does not drive the
gate; it drives a stage whose own supply is the rail the gate has to reach. The 3k3 carries
about 1.5 mA, which is also why the BS170's threshold does not have to be met properly, and
it is deliberately weak: a P-MOSFET that opens slowly hands the strip's inrush to the supply
over microseconds instead of at once.

The data line then wants a diode. A BAT43 with its anode on the data line and its cathode on
the **switched** rail bounds how far DIN can sit above the supply the strip itself is on.
That is the failure the 330 Ω above only limits: with the rail down, current arriving at DIN
leaves through the WS2812's internal protection diode into a rail that cannot take it. The
Schottky's 0.3 V undercuts that diode's 0.6 V, so the BAT43 carries it and the strip's own
does not. What it buys is independence from software order — blank the strips, wait for the
frame to leave the wire, then cut the supply is what the firmware has to do, and a watchdog
or a reset keeps no such order. A second BAT43, cathode on the data line and anode on ground,
is optional on that board and answers the other direction: undershoot from a reflection on a
long cable.

The switched rail leaves on a 16-pin header, odd pins on 5 V and even pins on ground, eight
of each — the inrush note below applied to contact resistance rather than to wire.

Two things not to copy. That board level-shifts its data line with a 1k8 pull-up to 5 V
against an open-drain port, which is the mechanism already rejected above at 10 kΩ and about
five times quicker; it still leaves a weak pull-up shaping the rising edge of a line that
wants a push-pull driver, so the `SN74AHCT125N` stays the answer. And its own note says the
whole switch may be left unbuilt with the IRF9310's drain and source bridged, which is a
reason to keep it optional in the firmware rather than a reason to skip it.

The firmware side is built and `POWER_SWITCH_IS_FITTED` in
[`include/PowerSwitch.h`](include/PowerSwitch.h) is what turns it on, together with the pin
beside it — `STD_OFF` and GPIO 11 until somebody confirms both against a board. Until then ids
20 and 21 answer `Error=9` rather than pretending, and the port is never driven, which is the
one thing a wrong pin here could do damage with. What the sequence behind those ids has to do,
and why it takes two ticks, is in [`Power.h`](../../firmware/inc/Power/Power.h).

### Supply

- **Brown-out on inrush.** The classic failure is a reset when the strip's inrush sags
  the 5 V rail while WiFi is transmitting. Star wiring for the 5 V, and 470 µF at the
  module.
- **Supply sizing.** A word clock lights twenty to thirty letters at once, so it draws a
  few hundred mA in amber at a living-room brightness and some 2 A in white at full. The
  6.6 A that all 110 at full white would take is reachable all the same, and with three
  commands: `test()` sets every pixel at once and `BrightnessMaxValue` is 255. Size for the
  6.6 A rather than for the display — 5 V / 10 A puts a supply at two thirds of its rating,
  which is where it belongs once it has aged and warmed up.
- **What the firmware will not let it draw.**
  [DisplayCurrentLimit](../../firmware/inc/Display/DisplayCurrentLimit.h) holds the display to
  `DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS`, which defaults to the 2500 mA a 5 V / 3 A supply
  can be asked for. It estimates from the number of lit letters and the colour — 20 mA per
  channel at full and 1 mA idle per LED, the numbers the 6.6 A above is itself made of — and
  scales the strip's master down until the estimate fits. On this display and that budget the
  clock face costs 1190 mA in white and is left alone; `test()` in white asks 6710 mA and is
  held to 92 of 255, which is 2481 mA.

  This is a floor under a mistake, not a substitute for a supply: it cannot know what the rail
  actually does, it estimates high on purpose, and a display told to be dark still draws its
  110 mA of idle current. Set the budget from the supply that is fitted — the number belongs to
  the build, which is why no command can raise it.
- **1000 µF at the strip's supply**, beside the 470 µF at the module — the same inrush,
  answered at the end that causes it.

### The I²C devices

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

## The network

The credentials are **not compiled in any more**. They live in their own NVS namespace and
are set with command 13 over whichever console can reach the clock:

```
13 -SMyNetwork -Psecret
```

A clock with nothing stored opens an open access point named `Wordclock` and serves the web
console on it, which is where the first pair is entered — 192.168.4.1 in a browser. The
access point closes as soon as a network is stored.

`WORDCLOCK_WIFI_SSID` in [`WordclockConfiguration.h`](include/WordclockConfiguration.h) and
a local `WordclockSecrets.h` still work and are the fallback: what is stored wins over what
was compiled in, because it is the later word. A clock flashed with credentials therefore
comes up on the network as before, and can still be moved to another one without a reflash.

The namespace is separate from the settings blob on purpose, so that a settings reset
(`1 -P30`) leaves the network alone. A reset that took it away would leave a clock nobody
can reach without a cable.

## Gaps

- **A clock without a battery still comes up blank.** The DS3231 fills the gap between
  power-on and the first SNTP answer — but only if it has one to fill it with: a chip
  without a battery reports that its oscillator stopped, and its registers are then ignored
  on purpose.
- **Authentication is optional and is off until a password is set** — command 16, stored in
  NVS beside the WiFi credentials. With one set, every route asks for it: the page, the
  catalog, the panel, `POST /update` and the web socket's handshake, because a console that
  asked for a password and then took commands on an unchecked socket would be a lock on the
  wrong door. Without one, the clock answers everybody, which is what a clock already on a
  wall keeps doing after an update.

  What it is worth is worth stating exactly. **Basic authentication over plain http is not
  encryption**: the credential is base64 of `wordclock:<password>` and travels in the clear.
  It keeps out a guest who opens the page and starts sending commands or installing firmware.
  It keeps out nobody who can watch the traffic — that needs https, which a clock on a house
  network cannot offer credibly. And the access point an unconfigured clock opens *is* an
  open network, which is the window a first setup happens in either way.

  A forgotten password is cleared by sending command 16 with no option over the **serial
  line** — the one way in that does not go through the console it locks. That needs the
  cable, which is the price of the lock.
- **The time and date are not stored**, because they come from the network. Everything else
  the clock is set to survives a restart, the overlays and their text included.
