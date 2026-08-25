# Roadmap

What is still open, why it is worth doing, and what settles it. Ordered by what a
running clock gains, not by effort.

**Finished work is not here.** Its reasoning is at the code it explains — `Power.h`
carries the supply sequence, `ColorCycle.h` why the hue is not stored, `NightSwitch.h`
why the night acts on the crossing — which is where somebody changing that code will
read it, and where this file could only repeat it out of date. What is done is indexed
at the end, one line and a link each. The one measurement that no header owns is in
[decisions.md](decisions.md), and the projects this was measured against are in
[comparison.md](comparison.md).

## Next

### 1. A Bluetooth transport for the command set

A fourth consumer of a command set that already has three, so the parser and the
catalog would not move at all. ednieuw's clock does this over the Nordic UART service
with NimBLE, reachable from a phone and from a browser terminal, and the same
single-letter commands arrive over it as over the serial line.

What it is *not* is first-boot provisioning — that is settled with an access point, and
a clock already serving its console over WiFi gains nothing from a second way to do the
same thing. Its value is the clock **already on the wall**: no cable reaches it, and a
WiFi it can no longer join is exactly the fault that leaves no way in. Bluetooth answers
when the network does not.

It belongs on the platform seam rather than in the core, with the same honesty as the
power switch: the ESP32-S3 and the Pico 2 W have a radio, the AVR128DA48 has none, and
the simulator would have to fake one — so a backend that cannot do it says so rather
than being compiled out.

### 2. MQTT and Home Assistant discovery

The one feature of the comparison projects a user would notice missing: a clock that
dims with the rest of the house rather than on its own schedule. Both have it,
ednieuw's over the JSON light schema.

It fits the architecture better than its size suggests — another transport onto the
existing command set, like the Bluetooth item — but it is not free the way that one is:
it needs a broker to talk to, a reconnect policy for when the broker is the thing that
is down, and a discovery payload that has to keep agreeing with what Home Assistant
expects across its releases. That last part is the real cost, because it is upkeep
rather than work. Wanted, but after the Bluetooth transport, and only if the house it
joins actually runs a broker.

### 3. Ambilight

A second strip with its own colour and timer.

## Waiting on a board

Neither of these is settled by a build, and writing them before the hardware exists
would be guessing at what the measurement says.

- **Feed the ESP32's RMT channel by DMA.** Today the frame is refilled from the driver's
  interrupt, so the strip's timing depends on that interrupt being served: a block holds
  48 symbols, about 58 µs of output, and the part is running WiFi at the same time. A
  missed refill stalls the data line, and a WS2812 reads a long enough gap as the end of
  a frame — which shows up as a display latching a picture shifted by a pixel, the
  failure that reads like a wiring fault. `flags.with_dma` in
  [Pixels.cpp](../platform/esp32/src/Pixels.cpp) is the whole change, and it takes the
  deadline away rather than widening it. Two reasons it is not simply done: **only the S3
  can do it** — `SOC_RMT_SUPPORT_DMA` is absent on the classic ESP32, the S2, the C3 and
  the C6 — so it needs a case distinction rather than a flag, and `mem_block_symbols`
  stops meaning blocks and starts meaning a DMA buffer, so the constant beside it has to
  be reconsidered at the same time. Nothing in this backend has ever driven a strip, so
  the interrupt this would remove has never been observed missing a deadline: the honest
  order is a first board and an oscilloscope, then this.
- **Watch an OTA image being applied.** The upload handler and its five refusals are
  exercised on the host against stand-ins for LittleFS and PicoOTA, on both networked
  backends. What has not happened is a loader booting into something it was sent. Until
  it does, the feature is tested and not proven.

## Waiting on something outside the repository

**The install prompt needs `https`.** Chromium installs a page only from `https` or
`localhost`, and the clock is reached over plain `http` on a house's own network — so on
Android and on the desktop the manifest buys nothing today. iOS is not gated on the
scheme and installs; the development server answers on `localhost` and is the one place
the install can be tried. That also settles what not to add: a service worker needs the
same secure context, and offline is not what this page wants anyway, since a console
whose clock is unreachable has nothing to show.

## An idea, not a plan

**A second language in the word tables.** The mechanism is half here already: four
German wordings switch at runtime through `Clock::ModeType`
([Clock.h](../firmware/inc/Clock/Clock.h)), so "several ways to say the time, chosen
while running" is a solved problem. ESPWortuhr's shape is the one to copy — one header
per layout with a generated file collecting them, chosen from a dropdown.

What is German is deeper than the tables, though, and that is why this is an idea.
`DisplayWords::WordIdType` names its entries `WORD_FUENF`, which is cosmetic; the letter
grid in `DisplayCharacters` is not, because it *is* the front plate. A second language is
therefore a second front plate, and its size need not be 11×10 — which turns this from a
table into a layout abstraction, with the column and row counts becoming values instead
of the compile-time constants they are today. Worth doing when a second plate exists to
justify it; guessing at the abstraction beforehand would fix the wrong things. The
AVR128DA48 also has a say, since runtime-switchable layouts cost flash it may not have
to spare.

## Deliberately not planned

Weather reports, MP3 playback and alarms, and games on the display: the four things
wordclock24h grew over years, and none of them is a word clock.

An **IR receiver** is a different kind of no. A handset is a thing to lose, with a flat
battery when it is found and a line of sight to keep, in front of a clock that is on the
network anyway. What it was wanted for — control without a phone — is answered by a knob
on the case, which the increment and decrement procedures already serve.

## What is done

One line each, pointing at the code that carries the reasoning. The order is the order
it happened in.

| | Where the reasoning lives |
|---|---|
| **RPCs 22 to 30** — clock refresh, animation start and abort, the three overlays on demand, overlay abort, save and reset the settings | the RPC table in [serial-commands.md](serial-commands.md); the per-module `resetToDefaults()` in [Animations.h](../firmware/inc/Animation/Animations.h) and its siblings |
| **The temperature overlay**, reading a DS3231 on the bus the light sensor already runs | [OverlayTemperature.h](../firmware/inc/Overlay/OverlayTemperature.h), [Temperature.h](../firmware/inc/Temperature/Temperature.h) |
| **The degree sign**, appended by hand as glyph 102 and sent to the display only | *Adding a single glyph* in [fonts.md](fonts.md) |
| **Status command 12** — version, uptime, illuminance, temperature, address, link quality, free memory | the `ReadOnly` flag in [MessageCatalog.h](../firmware/inc/Communication/MessageCatalog.h), [Version.h](../firmware/inc/Version/Version.h) |
| **RPCs 34 to 40** for a control with no display — animation next and previous, clock mode next, three toggles, colour reset | [Animations.h](../firmware/inc/Animation/Animations.h), [DisplayManager.h](../firmware/inc/DisplayManager/DisplayManager.h) |
| **Platform hooks** — `System.h`, procedures 31 to 33, and the four fields only a backend knows | the [platform contract](../platform/avr-dx/README.md) |
| **Cutting the strip's supply**, ids 20 and 21 against the WC MiniDev Shield v5 | [Power.h](../firmware/inc/Power/Power.h), and the hardware end in the [ESP32 notes](../platform/esp32/README.md#switching-the-strips-supply) |
| **The night switch**, which is what turns that into something a clock uses on its own | [NightSwitch.h](../firmware/inc/NightSwitch/NightSwitch.h) |
| **WiFi provisioning** — credentials in NVS, command 13, an access point when there are none | [web/README.md](../web/README.md) |
| **The panel and the console** — six task pages at `/`, the generated command groups at `/console` | [web/README.md](../web/README.md) |
| **OTA update** on both networked backends, `POST /update`, and command 16's password in front of every route | the two backends' READMEs |
| **An RTC with a battery**, keeping UTC and never overwriting a clock SNTP has set | [RealTimeClock.cpp](../platform/esp32/src/RealTimeClock.cpp) |
| **Colour animations** — a hue cycle that is a level of its own rather than a write to the colour | [ColorCycle.h](../firmware/inc/ColorCycle/ColorCycle.h) |
| **The overlay configuration is stored**, all three slots, the text as a fixed field | [Persistence.h](../firmware/inc/Persistence/Persistence.h) |
| **An installable console**, with the limit above as the more interesting half of the answer | [manifest.webmanifest](../web/manifest.webmanifest), [web/README.md](../web/README.md) |
| **One accessor per operation, and both check** — the `*Fast` twins are gone | the [platform contract](../platform/avr-dx/README.md), measured in [decisions.md](decisions.md#the-fast-accessor-pair) |
