# Roadmap

What is planned, why, and what each item touches. Ordered by what a running clock
gains from it, not by effort. Items that are only ideas are kept in the backlog at
the end rather than dropped, so a rejected alternative stays visible next to the
thing that replaced it.

The comparison that shaped the backlog is with
[wordclock24h](https://github.com/ukw100/wordclock24h) (Frank Meyer, documented at
[mikrocontroller.net](https://www.mikrocontroller.net/articles/WordClock_mit_WS2812)),
the most complete of the published word clocks and the one this project overlaps
with most.

## Adding an RPC

Three places have to move together, and nothing checks that they did:

1. `RpcIdType` and the `switch` in
   [MsgCmdRemoteProcedureCallParser.h](../firmware/inc/Communication/MessageParser/MsgCmdRemoteProcedureCallParser.h)
2. `RemoteProcedureValueNames` in
   [MessageCatalog.cpp](../firmware/src/Communication/MessageCatalog.cpp) — mirrored
   by hand, and read positionally, so the order is the contract
3. The RPC table in [serial-commands.md](serial-commands.md)

New ids go at the **end** of the enum. They are not stored anywhere, so a shift
would not corrupt a saved configuration, but it would silently change what every
written-down `-P<id>` does, including the ones in this repository's own docs.

An RPC is a fire-and-forget action. Its answer is `RpcId=<id> Error=<code>` and has
no payload field, so anything with a value to report is a command, not an RPC — see
*Status command* below.

## 1. RPCs for what the clock can already do

The largest group of things the firmware does but no interface reaches. Ids
continue at 22.

| Id | RPC | What it calls | Why |
|----|-----|---------------|-----|
| 22 | Clock refresh | invalidate the DisplayManager's word latch | The clock face is redrawn on a word change only ([DisplayManager.h](../firmware/inc/DisplayManager/DisplayManager.h)). After `1 -P7` or a few `4 -I..` the display keeps showing that for up to five minutes. Must clear `ClockWordsInitialized` rather than call `Clock::refresh()` directly, or the latch and the display disagree on what is drawn. |
| 23 | Animation now | `Animations::setTime()` with the current time | Replays the selected animation on demand. Today an animation can only be watched by waiting for a word change or by re-selecting it with `9 -A<id>`, which is a different thing to mean. |
| 24 | Animation abort | stop the running animation, draw the clock | At a low `-S` an animation runs for a long time, and both a mode change and a word change are ignored while it does. |
| 25/26/27 | Overlay date / temperature / text now | `Overlays` starts the overlay | Overlays fire in their period raster only ([Overlay.h](../firmware/inc/Overlay/Overlay.h)). There is no way to say "show me the date now", which is also what makes an overlay hard to develop against. |
| 28 | Overlay abort | state → idle, show timer 0 | The counterpart of the three above; matters most for a text overlay with a long endurance. |
| 29 | Save settings now | force a `Persistence` write | Writes are rationed to one per two seconds ([Persistence.h](../firmware/inc/Persistence/Persistence.h)) — enough of a window to lose the last change when the plug is pulled. |
| 30 | Reset settings | `Storage::clear()`, then apply defaults | Nothing currently gets a clock back to defaults short of erasing NVS over USB. |

The overlay triggers cannot call the overlay directly: the show timer lives in
`Overlays`, not in the overlay ([Overlays.cpp](../firmware/src/Overlay/Overlays.cpp)),
so the entry point belongs there and takes the endurance the overlay's
`setStateToShow()` returns.

## 2. Finish the temperature overlay

[OverlayTemperature](../firmware/inc/Overlay/OverlayTemperature.h) is an empty shell:
command 6 configures period, endurance, date and font, the overlay switches state,
and nothing is ever drawn. It is the only part of the command set that answers as if
it worked.

The overlay body itself is small — [OverlayDate](../firmware/inc/Overlay/OverlayDate.h)
is the template to follow, with a formatted string handed to `Text::setTextWithShift()`.
What has to be decided first is everything around it:

- **Where the value comes from.** The core reaches hardware through header names
  only, so this is a new one next to `BH1750.h` and `Storage.h` — `Temperature.h`,
  with a reading and a "no sensor" answer, listed in the
  [platform contract](../platform/hardware/README.md).
- **Which sensor.** The ESP32 already runs an I²C bus for the BH1750, so a second
  device on it (SHT31, BME280) costs no pin; a DS18B20 costs a pin and a one-wire
  driver. A DS3231 would bring a temperature reading *and* close the RTC gap in the
  backlog below, which is why it is worth deciding these two together rather than
  in sequence.
- **The simulator's stand-in.** A slider next to the illuminance one in
  [Settings](../platform/simulator/include/sim/Settings.h), for the same reason: the
  overlay has to be developable without hardware.
- **The degree sign.** The fonts carry ASCII 0x20–0x7F plus six umlauts
  ([Text::convertCharToFontIndex](../firmware/src/Text/Text.cpp)); `°` is not in them.
  Either all five tables are regenerated with it appended as index 102 (the generator
  is [FontCreator](https://github.com/theAndreas/FontCreator), see
  [fonts.md](fonts.md)), or the overlay writes `23C`. The first is the better display
  and touches every font table; the second is free.
- **What happens without a sensor.** A build with no `Temperature.h` implementation,
  or a sensor that does not answer, must leave the overlay silent rather than show a
  placeholder reading — a wrong temperature on the wall is worse than none.

## 3. Status command

Everything an RPC cannot answer: illuminance raw value, IP address and link quality,
uptime, firmware version, free heap. As a new command (12) with the query semantics
the other commands already have, so it appears in the
[MessageCatalog](../firmware/inc/Communication/MessageCatalog.h) and therefore in
the simulator dialog and any future web UI without further work.

## 4. RPCs for a remote control

The eight increment/decrement ids are shaped for a button that has no display to
show a value. That set is incomplete for the same use:

| Id | RPC |
|----|-----|
| 31/32 | Animation next / previous |
| 33 | Clock mode next |
| 34/35/36 | Display toggle / brightness automatic toggle / gamma correction toggle |
| 37 | Colour reset (white) |

Worth doing before the IR receiver in the backlog, not after: the receiver then maps
keys onto an interface that already exists and can be tested over the serial line.

## 5. Platform hooks

| Id | RPC | Needs |
|----|-----|-------|
| 38 | System restart | a `System.h` in the platform contract; `esp_restart()` on the ESP32 |
| 39 | Resynchronise time | `sntp_restart()`; today only a reset helps when the NTP server was unreachable at boot |
| 40 | Reconnect WiFi | `WiFi.reconnect()` |

## Backlog

From the comparison with wordclock24h, in the order they would change daily use.

1. **WiFi provisioning and a configuration UI.** The SSID is compiled in
   ([WordclockConfiguration.h](../platform/esp32/include/WordclockConfiguration.h)) and
   the web interface is deliberately a console
   ([WebInterface.h](../platform/esp32/include/WebInterface.h)), so a new network means
   a new flash. An access point on first boot plus credentials in NVS is the single
   biggest difference to a finished product.
2. **Night switch-off / timer.** No equivalent at all today; the clock lights around
   the clock. A firmware module with its own command, and the first setting that needs
   a time of day rather than a value.
3. **OTA update.** Cheap on the ESP32, and the natural companion to 1.
4. **RTC with battery.** After a power cut the display holds its default date until
   SNTP answers ([RealTimeClock.h](../platform/esp32/include/RealTimeClock.h)). See the
   sensor decision in section 2 — a DS3231 answers both.
5. **Colour animations.** All fifteen animations are transitions; a slow colour cycle
   while the display stands still is a different mechanism and does not exist.
6. **IR receiver**, after section 4.
7. **Ambilight**, a second stripe with its own colour and timer.

Deliberately not planned: weather reports, MP3 playback and alarms, games on the
display. They are what wordclock24h grew over years, and none of them is a word
clock.

## Where this project is ahead

Worth writing down, because it is what the items above must not break:

- Fifteen transition animations with three selection modes, a speed per animation and
  a favourite flag ([Animations.h](../firmware/inc/Animation/Animations.h)).
- Four German regional wordings switchable at runtime
  ([Clock.h](../firmware/inc/Clock/Clock.h)).
- A platform abstraction with a full desktop simulator, so the firmware is developed
  and tested without hardware.
- A BH1750 over I²C with min/max calibration and gamma correction, rather than an LDR
  on an ADC pin.
- One command set for the serial line, the browser console and the simulator dialog,
  machine-readable in the [MessageCatalog](../firmware/inc/Communication/MessageCatalog.h) —
  a command added once appears in all three.
