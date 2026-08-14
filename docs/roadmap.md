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

Three places have to move together:

1. `RpcIdType` and the `switch` in
   [MsgCmdRemoteProcedureCallParser.h](../firmware/inc/Communication/MessageParser/MsgCmdRemoteProcedureCallParser.h)
2. `RemoteProcedureValueNames` in
   [MessageCatalog.cpp](../firmware/src/Communication/MessageCatalog.cpp) — written
   beside the firmware rather than derived from it, and read positionally, so the order
   is the contract. A `static_assert` counts the names against `RpcIdType`, so a missing
   one does not compile; a *misordered* one still does.
3. The RPC table in [serial-commands.md](serial-commands.md)

New ids go at the **end** of the enum. They are not stored anywhere, so a shift
would not corrupt a saved configuration, but it would silently change what every
written-down `-P<id>` does, including the ones in this repository's own docs.

An RPC is a fire-and-forget action. Its answer is `RpcId=<id> Error=<code>` and has
no payload field, so anything with a value to report is a command, not an RPC — see
*Status command* below.

## 1. RPCs for what the clock can already do

The largest group of things the firmware does but no interface reaches. Ids continue
at 22.

**Done — ids 22 to 30**, see the RPC table in [serial-commands.md](serial-commands.md):
clock refresh, animation start and abort, the three overlays shown on demand, the overlay
abort, and saving and resetting the settings. Three things they ran into are worth keeping
in mind for what follows:

- The show timer lives in `Overlays`, not in the overlay
  ([Overlays.cpp](../firmware/src/Overlay/Overlays.cpp)), so the entry point belongs
  there and takes the endurance the overlay's `setStateToShow()` returns.
- The clock refresh clears `ClockWordsInitialized` rather than calling `Clock::refresh()`
  directly ([DisplayManager.h](../firmware/inc/DisplayManager/DisplayManager.h)), or the
  latch and the display disagree on what is drawn.
- The reset asks each module for its own defaults through a `resetToDefaults()`, the same
  way `Persistence::gather()` reads the current values from each of them. A list of
  defaults kept in `Persistence` would be the copy that falls behind, and the values it
  would copy were literals in four different headers before this.

## 2. The temperature overlay

**Done.** [OverlayTemperature](../firmware/inc/Overlay/OverlayTemperature.h) was an empty
shell — command 6 configured it, the overlay switched state, and nothing was ever drawn.
It now shows the reading, and the decisions it needed came out as follows:

- **The sensor is a DS3231** ([DS3231.cpp](../platform/esp32/src/DS3231.cpp)). It costs no
  pin on the I²C bus the light sensor already runs, and it is the same chip that closes the
  RTC gap — which is why it beat an SHT31 or a one-wire DS18B20. What it measures is
  its own die, so a closed case reads above the room by whatever that case adds.
- **The core reaches it as `Temperature`**
  ([Temperature.h](../firmware/inc/Temperature/Temperature.h)), the same arrangement as
  `Illuminance` and its `BH1750`, with `DS3231.h` added to the
  [platform contract](../platform/hardware/README.md).
- **No reading, no overlay.** The reading comes with a return code rather than as a
  number, because a build without the chip has to be told apart from a reading of zero
  degrees. `Overlay::canShow()` asks before every start, so the overlay neither fires in
  its period nor through `1 -P26` until the chip has answered once.
- **The degree sign is still not in the fonts**, so the overlay writes `23.4C`. Adding it
  means regenerating all five tables with [FontCreator](https://github.com/theAndreas/FontCreator)
  (see [fonts.md](fonts.md)) — worth doing, but it is a font change rather than an overlay
  one, and it now has a caller waiting for it.
- **The simulator stands in with a slider and a *Sensor connected* box**
  ([Settings](../platform/simulator/include/sim/Settings.h)). The box is what makes the
  "no chip" state reachable without a chip to unplug.

Left over: the overlay configuration is still not persisted, along with the other two
overlays' — the stored format has no variable-length field for the text yet
([Persistence.h](../firmware/inc/Persistence/Persistence.h)).

## 3. Status command

**Done** — command 12 answers the firmware version, the uptime, the illuminance in lux, the
temperature, the address, the link quality and the free memory, see
[serial-commands.md](serial-commands.md). Two things it settled:

- **Read-only fields.** The command takes no options at all, so an option sent to it is
  answered with `ERROR_PARAMETER_UNKNOWN` by the base parser rather than ignored. The
  field names live in the [catalog](../firmware/inc/Communication/MessageCatalog.h) with a
  `ReadOnly` flag, which is what lets both front ends label an answer without offering the
  fields as inputs.
- **The version is hand-kept** in [Version.h](../firmware/inc/Version/Version.h). Neither
  platform's build has the working tree when it compiles, so deriving it from the
  repository would mean build plumbing in two places; a macro that a build can override
  from the outside was the smaller answer.

The fields that only the platform knows - uptime, address, link quality, free memory -
came with `System.h` in section 5 and are in the answer too.

## 4. RPCs for a remote control

The eight increment/decrement ids are shaped for a button that has no display to
show a value. That set is incomplete for the same use:

| Id | RPC |
|----|-----|
| 34/35 | Animation next / previous |
| 36 | Clock mode next |
| 37/38/39 | Display toggle / brightness automatic toggle / gamma correction toggle |
| 40 | Colour reset (white) |

Worth doing before the IR receiver in the backlog, not after: the receiver then maps
keys onto an interface that already exists and can be tested over the serial line.

## 5. Platform hooks

**Done** — `System.h` is in the [contract](../platform/hardware/README.md), and with it the
procedures 31 to 33 (restart, resynchronise the time, reconnect the network) and the four
status fields that only the platform knows: uptime, address, link quality, free memory.
Three things it settled:

- **Every one of them can answer "no".** A clock with no network has no address, and a
  simulator has no heap worth reporting, so all four getters carry a return code and the
  status command sends an empty field rather than a zero that reads like a value.
- **The restart is deferred.** A command's answer is sent after `process()` returns, so a
  controller restarting inside the procedure would take that answer with it. `restart()`
  asks; the application's tick carries it out, after the UART has been flushed.
- **The time resynchronisation reuses the application's own `configTzTime()` call** rather
  than reaching into SNTP, which keeps the zone rule in one place.

## Backlog

From the comparison with wordclock24h, in the order they would change daily use.

1. ~~**WiFi provisioning.**~~ **Done.** Credentials live in NVS and are set with command
   13; a clock with none opens an access point and serves the console on it, which is where
   the first pair is entered. What is left of this item is the *configuration UI*: the web
   interface is deliberately a console
   ([WebInterface.h](../platform/esp32/include/WebInterface.h)), so provisioning means
   typing a command rather than filling in a form. The catalog already describes the
   command, so a form over it is a page change rather than a firmware one.
2. **Night switch-off / timer.** No equivalent at all today; the clock lights around
   the clock. A firmware module with its own command, and the first setting that needs
   a time of day rather than a value.
3. **OTA update.** Cheap on the ESP32, and the natural companion to 1.
4. ~~**RTC with battery.**~~ **Done** with section 2's chip: the time registers are read
   while the system clock holds nothing, and written back from it once an hour and after
   every hand-set time ([RealTimeClock.cpp](../platform/esp32/src/RealTimeClock.cpp)). The
   chip keeps UTC, because a chip keeping local time has nothing to say about the hour that
   occurs twice when summer time ends. Which side wins was the decision: the chip is the
   source of last resort and never overwrites a system clock that SNTP has set, since it
   drifts a few seconds a month and SNTP does not.
5. **Colour animations.** All fifteen animations are transitions; a slow colour cycle
   while the display stands still is a different mechanism and does not exist.
6. **IR receiver**, after section 4.
7. **Ambilight**, a second stripe with its own colour and timer.
8. **The degree sign in the font tables**, which the temperature overlay is waiting for
   (section 2).

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
