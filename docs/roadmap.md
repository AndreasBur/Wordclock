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

Two more were read later and are named where they contributed. ESPWortuhr's
[Multilayout-ESP-Wordclock](https://github.com/ESPWortuhr/Multilayout-ESP-Wordclock)
is the one to measure a configuration page against, and the one that ships a
flashable file per chip per release.
[ednieuw's Arduino-ESP32-Nano-Wordclock](https://github.com/ednieuw/Arduino-ESP32-Nano-Wordclock)
contributed its *transports* rather than its structure: it reaches the same command
set over serial, Bluetooth, a browser and an SD card log through one pair of
functions. Its structure is the opposite of this one - a single `.ino` with global
state and clock faces chosen by `#define` - and nothing here should move towards it.

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
  [platform contract](../platform/avr-dx/README.md).
- **No reading, no overlay.** The reading comes with a return code rather than as a
  number, because a build without the chip has to be told apart from a reading of zero
  degrees. `Overlay::canShow()` asks before every start, so the overlay neither fires in
  its period nor through `1 -P26` until the chip has answered once.
- ~~**The degree sign is still not in the fonts**~~ **Done.** The overlay shows `23.4°C`.
  Appended by hand as index 102 rather than regenerated with
  [FontCreator](https://github.com/theAndreas/FontCreator), which rasterises the Windows
  faces the tables came from and would have replaced the other 102 glyphs to add one; see
  *Adding a single glyph* in [fonts.md](fonts.md). What it settled: **the sign goes to the
  display only**. `getTemperatureString()` still reports `23.4C`, because that string
  travels the web socket, and a text frame carrying a raw Latin-1 0xB0 is closed by the
  browser as invalid UTF-8 rather than drawn — `getTemperatureStringToShow()` is the form
  that carries the sign.
- **The simulator stands in with a slider and a *Sensor connected* box**
  ([Settings](../platform/simulator/include/sim/Settings.h)). The box is what makes the
  "no chip" state reachable without a chip to unplug.

~~Left over: the overlay configuration is still not persisted~~ **Done.** All three
overlays are stored, the text included, at format version 3. What it settled:

- **The text is a fixed field, not a length-prefixed one.** A variable field would have
  saved some forty bytes and cost the property everything in
  [Persistence.h](../firmware/inc/Persistence/Persistence.h) rests on: that the settings
  are one fixed block, which `memcmp` compares and the checksum walks without knowing
  what is in it.
- **All three slots exist whether or not the build compiles the overlays in.**
  `Overlays::OverlayIdType` is conditional and would have been the obvious index, which is
  why it is not the one used — a layout that moved with `OVERLAYS_SUPPORT_*` would let two
  builds write mutually unreadable blobs under the same version number.
- **`STORAGE_CAPACITY` went to 256** on both platforms. The blob was 110 bytes then and is
  118 with the night switch and the colour cycle in it, so the old 128 would still have held
  it - but only just, which is the whole reason for the headroom.
- **A reset now has overlays to undo**, so `Overlays::resetToDefaults()` joins the four
  modules `Persistence::reset()` already asks. The defaults are named constants rather
  than literals written twice, so the member initialisers and the reset cannot drift.

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

## 4. RPCs for controls that hold no state

The eight increment/decrement ids are shaped for a button that has no display to
show a value. That set was incomplete for the same use.

These are not for a phone. A phone knows the state, offers a list and sends the value
it wants - `2 -A7`, and no "next" is needed. What cannot know the state is a knob or a
button on the case, which can only ever say "one further"; that is what the existing
eight are for and what these seven complete.

The reason to have such a control at all is the one thing no phone gives: a guest can
dim the clock. No app, no network password, nobody hunting for a phone - and it is the
only way in that survives the WiFi being down.

This used to be written as preparation for the IR receiver, which is now in the "not
planned" list. The interface outlived its first reason: a rotary encoder needs exactly
the same calls, and can be developed against them over the serial line before any
hardware exists.

**Done — ids 34 to 40**, see the RPC table in [serial-commands.md](serial-commands.md):
animation next and previous, clock mode next, the three toggles and the colour reset. What
they settled:

- **The step lives in the module that owns the setting**, not in the parser's `switch`:
  `Animations::nextAnimation()`, `Clock::nextMode()`, `Display::toggle()`. The encoder
  driver this is for is not written yet, and when it is it must not re-derive where the
  list wraps - it has one caller's worth of distance from the serial line, and that is the
  whole reason to build the interface before the hardware.
- **A knob's round is not `MODE_SEQUENCE`'s round.** `Animations` already had a walk to the
  next animation and it is the wrong one: `calcNextAnimation()` skips everything but the
  favourites, because that is what the mode picking among them needs. Stepping by hand has
  to reach an animation that is not a favourite, and `ANIMATION_ID_NONE` with it - "no
  animation" is a setting, and a knob is the only way somebody without a phone gets back to
  it.
- **Clock mode next draws nothing.** The wording it switches to says different words, and
  a different word set is exactly what
  [DisplayManager](../firmware/inc/DisplayManager/DisplayManager.h)'s latch already
  redraws for - with the selected animation, which `8 -M<id>` does not do. Where two
  wordings happen to agree at the current time nothing is drawn, which is the right answer
  rather than a missed one.
- **None of the seven can be refused**, so none of them takes a return value and all seven
  answer `Error=0`. A setting has no state it can be in that makes its neighbour
  unreachable - which is the opposite end from ids 22 to 30, and worth saying in the
  documentation next to them.
- **The display toggle needed a state that was not being kept.** `enable()` and `disable()`
  wrote the strip's master brightness and nothing else, and under
  `DISPLAY_USE_PIXELS_DIMMING` that same register carries the brightness setting - so
  reading a zero back there answers "dark", not "somebody switched this off". A flag in
  `Display` is what the toggle asks, and ids 3 and 4 keep it, so a display switched off
  from a phone is switched on by the knob.

Two things it turned up on the way, both older than this section:

- **A colour change never reached the strip.** The pixels carry the colour already dimmed
  by the brightness, and that dimmed copy was recomputed in one place only:
  `applyBrightness()`, behind its early return for a brightness that has not moved. So
  every colour change - command 2, the six increment procedures, and the reset this
  section adds - stayed invisible until something else happened to move the brightness, and
  a redraw did not help because it wrote the same stale copy. `Display::applyColor()` is
  now what both halves go through, and `testColourChangesReachTheStrip()` fails if either
  one stops.
- **`testDisplayOffAndOnAgain()` was written, declared and never called.** It sat in
  `cases.h` and not in the runner, which is the same failure mode as the `getOutputPixel`
  entry in backlog item 9: a list nothing enforces loses an entry and nobody hears about it.
  It is in the runner now, and it passes.

## 5. Platform hooks

**Done** — `System.h` is in the [contract](../platform/avr-dx/README.md), and with it the
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

## 6. Cutting the strip's supply

Ids 20 and 21 had been reserved since the enum was written and were the only two that accepted
a call and answered `Error=0` without doing anything. What they were waiting for was a circuit,
and there is a published one — the WC MiniDev Shield v5 switches the strip's 5 V with a
single active-high port, and its hardware end is written down in [the ESP32 backend's
notes](../platform/esp32/README.md#switching-the-strips-supply).

What the clock gains is the night. A strip told to be dark is still a strip drawing its
quiescent current: some 1 mA per WS2812, so about 110 mA and half a watt for this display,
all night and every night, for a wall that shows nothing. That is the whole return, and it is
also the reason this is worth less than it looks on a clock nobody dims.

**Done** — [Power](../firmware/inc/Power/Power.h) in the core, a `PowerSwitch` in each of the
four backends, and ids 20 and 21 answering for real. Five things it settled:

- **The two ids are a sequence, not a write.** Data arriving at DIN while the rail is down
  pushes current into it through the strip's own protection diode, so switching off blanks the
  strips, waits for that frame to *leave the wire*, and only then drops the port.
  `Pixels::render()` runs on the application's tick and the transmission outlives the call that
  started it, so none of it can happen inside `process()` — the procedure asks and a task
  carries it out, the way `System::restart()` does. Switching on is the same order backwards,
  the port a tick ahead of the data line, so the strip has its rail before the first frame.
- **Blanking is not enough to keep the line quiet.** This is what the plan above missed. The
  clock keeps running with the supply off, every pixel written marks the buffer, and the next
  tick would transmit again — so the output is *gated* rather than only darkened, and the gate
  is what the new `suspendOutput` / `resumeOutput` on the platform seam is for. Resuming has to
  mark the buffer as well: the LEDs lose their registers with their supply, so what comes back
  is a redraw and not a resume.
- **`disable()` is not `POWER_OFF`.** Ids 3 and 4 stayed exactly what they were, and 21 does
  not imply 4: it uses the same darkening as its first step, but a caller asking for one of
  them is not asking for the other, and only one of them saves any current. A test says so, so
  that collapsing them later fails rather than passes.
- **`isFitted()` is false on all three hardware backends, and true on the simulator.** The
  switch is optional hardware and the pin is not confirmed on any board, so what ships is a
  compile-time `STD_OFF` and an honest answer: `Error=9`, `ERROR_POWER_SWITCH_ABSENT`, rather
  than the `Error=8` that would have said "something went wrong". Two consequences worth
  keeping: the code behind the flag is still compiled rather than `#if`-ed out, because a
  branch that has never been compiled is not code; and `isSupplyOn()` answers **true** where
  the switch is absent, since that board has the high side bridged and its strip cannot be
  unpowered. A flag-based answer there would have reported a dark strip that is in fact lit.
- **The waiting sits in the core, the port in the backend.** Only a backend knows when its
  peripheral has finished, so `isFrameOnTheWire()` joined the seam; the state machine that uses
  it is written once. `Display` forwards both, so the module driving the switch has one
  collaborator rather than two.

One thing deliberately left: an animation still running keeps the buffer dirty and postpones
the cut by a tick each time. The display is dark from the first step, so what a longer wait
costs is the current the switch saves and not the darkness somebody asked for — which is not
worth a mechanism to cut short.

The follow-up is **done** as well, and it is the one that turns the switch into something a
clock uses on its own rather than something a command asks for:
[NightSwitch](../firmware/inc/NightSwitch/NightSwitch.h) cuts the supply when the night
brightness is zero, and gives it back in the morning. 82 bytes of flash on the AVR and one of
RAM. Two things it settled:

- **The night asks Power rather than the port**, and the display it darkens is Power's own
  first step, so the sequence and its waits are written once. Where no switch is fitted the
  night darkens the display the way it always did and says nothing about a rail it cannot
  move — the same honesty the procedures answer with.
- **The hand at two in the morning is watched as a state, and it is the only thing that is.**
  A cut rail turns "display on" into a switch that does nothing: enabled, gated, and a wall
  that is still dark. So a tick that finds the display enabled inside a window it took the
  supply away for hands the rail back — which keeps the crossing rule rather than breaking
  it, since what the tick does is let the hand win. What it does *not* do is the reverse: a
  display switched off by hand keeps its rail until the next edge, because a clock that undid
  what somebody just did is the behaviour this module exists to avoid. Only a cut the night
  made is given back, or a supply somebody dropped by hand at noon would be undone a second
  later.

## Backlog

From the comparison with wordclock24h, in the order they would change daily use.

1. ~~**WiFi provisioning.**~~ **Done.** Credentials live in NVS and are set with command
   13; a clock with none opens an access point and serves the console on it, which is where
   the first pair is entered. What is left of this item is the *configuration UI*: the web
   interface is deliberately a console
   ([WebInterface.h](../platform/esp32/include/WebInterface.h)), so provisioning means
   typing a command rather than filling in a form. The catalog already describes the
   command, so a form over it is a page change rather than a firmware one.

   What that page should look like is worth writing down now that there is something to
   compare against. ESPWortuhr serves a sidebar of task pages - colours, functions, view
   options, settings - with a colour wheel, two sliders and eight preset swatches, and at
   the one thing somebody does often it is plainly better than typing a command with three
   numbers. Four notes from looking at it:

   - ~~**The presets carry the weight, not the wheel.**~~ **Done.** Nobody picks a colour
     twice. They pick the one they had last month, and eight swatches are that in one tap.
     Fixed eight rather than remembered ones: this page is opened from whichever phone is to
     hand, and a remembered list is empty on the one you are holding.
   - ~~**A numeric field belongs beside the wheel.**~~ **Done**, as `#RRGGBB` next to the
     picker and typable back into it. Theirs has none, so a colour found by dragging cannot
     be written down, passed to somebody else or restored after a reset. That is what a wheel
     costs when it is the only way in.
   - ~~**Two unlabelled sliders are one too many.**~~ **Done**, and it did not need saying
     twice: the one slider here is labelled, and the automatic that overrides it sits under
     it - a slider that does nothing reads as broken, and theirs gives no way to tell which
     of the two states you are in.
   - ~~**The clock face can be drawn in the page**~~ **Done**, and it was done before this
     item was written down: `/display` answers with the letter grid as JSON
     ([WebInterface.cpp](../platform/rp2350/src/WebInterface.cpp)), and the lit letters
     arrive as binary frames on the `/ws` socket the console already holds open, which
     `showFrame()` in [index.html](../web/index.html) paints. So the page shows what the
     wall shows, which is the difference between changing a colour and walking into the
     other room to see what it did. One decision inside it worth keeping: only *whether* a
     pixel is lit is taken from the frame, not its colour, because the bytes arrive already
     dimmed and a word at low brightness would be painted a dark grey on a dark background -
     unreadable in the case one opens the view for. The wx window stays the colour-accurate
     view.

   **Done**, and the shape it took is worth keeping. What the page grew is *one* card above
   the generated groups, for the colour and the brightness - 1.6 KB of compressed flash on
   both network backends - and not a sidebar of task pages. Four things it settled:

   - **A purpose-built control cannot be generated, and that is the whole reason for the
     card.** The catalog says "three numbers called Red, Green and Blue, 0 to 255", which is
     what a colour *is*; that it deserves eight swatches and a picker is a judgement about
     what somebody does often. So the card is the one place in the page that names a command
     by its number, and it hides itself where the catalog does not carry them - a firmware
     built without the colour command shows the groups it does have rather than a dead card.
   - **The picker is the browser's own.** A wheel drawn here would be a canvas, a pointer
     handler and a hue conversion in a page the clock holds in its flash, and the phone's own
     picker is the one its owner already knows.
   - **Nothing is shown before the clock has said it.** A range input with no value sits at
     the middle of its range and a colour input at black, so a card drawn on load would
     claim a brightness of 128 and an unlit display. It appears with the first answer
     instead. The generated groups need none of this: an empty field reads as empty.
   - **Every answer feeds the page, not only the ones it asked for.** A colour typed into the
     console below, or set over the serial line, moves the card with it - two views of one
     clock that disagree are worse than one view. What still depends on who asked is whether
     the line is *printed*: the fourteen answers a page collects on load would bury the log.

   The console stays either way. It is what the simulator dialog and the serial line use,
   and a page that replaced it would have to grow a control for every command before it
   could. What is left of this item is the settings that are *not* colour and brightness -
   they have their generated group and no card, which is the right order of work: a control
   per command is what the groups already are.
2. ~~**Night switch-off / timer.**~~ **Done** — command 14, and
   [NightSwitch](../firmware/inc/NightSwitch/NightSwitch.h) in the core. What it settled:

   - **One brightness field, where zero is off.** "Off" and "very dim" are the same wish at
     different strengths, and two settings would have needed a rule for what they mean
     together.
   - **The dimming is its own level, not the fade.** A fade belongs to a running animation
     and is cleared when it ends — an animation finishing in the small hours would have
     taken the night dimming with it. Both scale what the setting arrived at rather than
     replacing it, so the brightness its owner chose is what morning returns to and what
     `Persistence` keeps storing.
   - **It acts on the crossing, not on the state.** A clock switched on by hand at two in
     the morning stays on until the window's next edge. Re-asserting on every tick would
     make a hand-switched display go dark a second later, which reads as a fault.
   - **An empty window is no window**, not a whole day — that is what an unconfigured
     clock has.
3. **OTA update.** **Done on the ESP32**, which is where this item said it was cheap:
   `POST /update` takes the image as the request body and the console's last panel sends it,
   16 KB of flash for the handler and 0.9 KB for the panel. The default partition table
   already carried two app slots and an `otadata`, so nothing about the layout moved - which
   is what made it cheap, and would have been the expensive part. Three things it settled:

   - **The body, not a form.** A multipart parser in flash would exist only to undo what a
     `<form>` did on the way out. What that buys is `curl --data-binary` as a first-class
     way in, which is the one that still works on the day the page is the thing that broke.
   - **Halfway is safe and has to read that way.** The bootloader is pointed at the new
     image only by the last byte, so an interrupted upload leaves the clock running what it
     was running. The panel says exactly that instead of reporting a failure, because the
     safe outcome and the alarming message would otherwise be the same event.
   - **The reboot is asked for, not taken**, the same deferral RPC 31 uses: a controller that
     restarts inside the handler sends the browser nothing, which reads as a failed update.

   **Done on the RP2350 too**, and the estimate above was right about the cost and wrong about
   one thing that made it cheaper: the loader is already there. Every image this build produces
   carries the core's OTA loader in a 10 028-byte `.ota` section at the front, put there by the
   default linker script - so what was missing was room and a handler, not a bootloader.

   The rest went as predicted. `POST /update` writes the image into LittleFS, `PicoOTA` writes
   the command page that names it, and the loader copies it into place on the next boot. The
   filesystem went from 64 KB to
   [1 MB](../platform/rp2350/platformio.ini) to hold an image of some 490 KB with room to
   grow, which leaves 3 MB of the part's 4 for a sketch using 16 % of them. And the migration
   is the one that was foreseen: the region grows downwards, its start moves, the filesystem is
   not found at the new start, and the settings go with it - so the first release carrying this
   is installed over USB and resets the settings once. Three things it settled:

   - **The safety comes from the order, not from a second slot.** The command page is written
     last, once the image is complete and measures what the request announced. No command page,
     no update - so an interrupted upload leaves a file nobody reads and a clock still running
     what it had, which is the same promise the ESP32's two slots make by construction.
   - **The body handler needs its own authorisation check.** The library hands over the body
     *before* it calls the request handler, so a check that sat only in the handler would have
     let an unauthorised upload fill the filesystem and be refused afterwards. A case fails if
     that check is taken out again.
   - **An upload that ends short of its own Content-Length is cleaned up; one whose connection
     dies is not.** The second never reaches the handler at all, so what removes its remains is
     the next upload, which deletes the old image before it writes - which is also why the free
     space is measured after that deletion and not before.

   What none of it has done is apply an image on a board. The handler and its five refusals are
   exercised on the host against stand-ins for LittleFS and PicoOTA; the loader itself has not
   been watched booting into something it was sent.

   ~~What is still missing on both is **authentication**.~~ **Done**, on both backends, with
   command 16: a password in NVS beside the WiFi credentials, off until one is set, and every
   route behind it - the socket's handshake included, since a console that asked for a password
   and then took commands on an unchecked socket would be a lock on the wrong door. What it is
   worth is stated where somebody will read it rather than assumed: Basic over plain http is
   not encryption, it keeps out a guest who opens the page and not anybody who can watch the
   traffic. A forgotten password is cleared from the serial line, which is the one way in that
   does not go through the console it locks.
4. ~~**RTC with battery.**~~ **Done** with section 2's chip: the time registers are read
   while the system clock holds nothing, and written back from it once an hour and after
   every hand-set time ([RealTimeClock.cpp](../platform/esp32/src/RealTimeClock.cpp)). The
   chip keeps UTC, because a chip keeping local time has nothing to say about the hour that
   occurs twice when summer time ends. Which side wins was the decision: the chip is the
   source of last resort and never overwrites a system clock that SNTP has set, since it
   drifts a few seconds a month and SNTP does not.
5. ~~**Colour animations.**~~ **Done** — [ColorCycle](../firmware/inc/ColorCycle/ColorCycle.h)
   and command 15. All fifteen animations are transitions, so the clock had nothing at all
   for the five minutes between two word changes; this is what happens during them. 1192
   bytes of flash on the AVR and 18 of RAM. Four things it settled:

   - **Not an animation id.** An id would put it in the list a word change picks from, where
     it has nothing to do - it neither begins nor ends with a transition. The favourites mask
     is exactly full at sixteen ids as well, so it could not have gone there without widening
     that first, which would have been the wrong reason to widen it.
   - **A level of its own, not a write to the colour.** `DisplayColor` decides in one place
     what reaches the strip, so the colour somebody chose is still what command 2 answers and
     what Persistence stores - and switching the cycle off needs no remembered copy to put
     back, because nothing was overwritten. The same arrangement the night brightness has.
   - **The current limit had to follow it.** The cap is computed from a colour, and with the
     cycle running the colour on the strip is not the one in the setting - so the limiter now
     asks what is *shown*. Without that a saturated hue on a full display would be budgeted
     as whatever pale value sits underneath it, which is a guess in the one direction this
     must never guess. A case fails if it goes back to reading the setting.
   - **The hue is not stored.** Persistence writes when the blob differs from the live
     settings, so a stored hue would be a flash write per step - hundreds of thousands a day
     on a part rated for a hundred thousand. Whether the cycle runs and how fast is stored;
     where the wheel was is nobody's business after a restart.

   The stored format therefore goes to **version 5**, which throws the saved settings away
   once on the update - there is no converting an old blob here, and that is the price of
   the two fields.

   What it does not do is animate the colour *per pixel* - a rainbow across the letters, or
   a wave running along a word. That needs a colour per pixel rather than one for the
   display, which is a different thing from this and a much larger one: every animation, the
   current limit and the strip's own byte order all assume a single colour today.
6. ~~**Make the console installable.**~~ **Done**, with one limit that was not visible when
   this was written and is the more interesting half of the answer.
   [manifest.webmanifest](../web/manifest.webmanifest) is served beside the page, with the
   two home screen icons [generate-icons.sh](../assets/generate-icons.sh) now rasterises
   from the letter master. 14.4 kB of flash on both network backends, almost all of it the
   512-pixel icon.

   A native app would still cost two platforms, two stores, signing and an update whenever
   either OS moves, and would end up sending the same commands over the same web socket.
   For something configured three times a year that remains the wrong trade.

   **The limit: Chromium installs only from `https` or `localhost`**, and the clock is
   reached over plain `http` on a house's own network. So on Android and on the desktop the
   manifest buys nothing today - "Add to Home screen" there makes a shortcut that still
   opens in a browser with its address bar. Where it does work is iOS, whose Add to Home
   Screen is not gated on the scheme, and the local development server, which answers on
   `localhost` and is therefore the one place the install can be tried at all. The rest is
   waiting on `https`, not on the page.

   That also settles what *not* to add: there is no service worker and there is no point in
   one, since it needs the same secure context. Offline is not what this page wants anyway -
   a console whose clock is unreachable has nothing to show.

   Two smaller things it settled. The manifest is served **uncompressed** while the page is
   gzipped: it is 485 bytes, and the 228 that compressing saves buys a header on the wire
   and an inflate in everything that wants to read it, `curl` and the backends' own tests
   included. And the icons are **16-colour palette PNGs** - the picture is two colours and
   some smoothed edges, and at 512 pixels the difference between a palette and 32-bit RGBA
   is 11 kB against 65 kB of a clock's flash.
7. **Ambilight**, a second stripe with its own colour and timer.
8. ~~**The degree sign in the font tables**~~ **Done**, see section 2.
9. **Is the checked/`Fast` accessor pair worth what it costs?** Every platform accessor
   comes in two forms: one that validates its index and answers `StdReturnType`, one that
   trusts the caller and answers the value. The justification is real for this target — a
   freestanding 8-bit part built with `-Os` and a loop over 110 pixels per frame, where a
   bounds check per pixel is time rather than a formality.

   What put this on the list is that `getOutputPixel` existed for its whole life with only
   the `Fast` half, on all three backends, and nothing noticed. A pattern in two parts
   whose second part nothing enforces loses its second part.

   **Measured, and the justification does not survive it.** The bounds check was put inside
   the platform seam's `Fast` forms, so that every one of the three hundred call sites pays for
   it without a single one being rewritten, and the AVR image was built either way:

   | | text |
   |---|---|
   | as it is | 48 210 |
   | with the seam checking every access | **48 114** |

   The checked build is **96 bytes smaller**, and the reason is worth more than the number.
   Per symbol, `Pixels::setPixelFast` grew by 66 bytes and then stopped being inlined, which
   took 48 bytes off `Display::setPixel`, 46 off `Display::setPixelFast`, 42 off
   `AnimationMatrix::setTimeTask` and 48 off the RPC dispatcher. So what the difference
   measures is an inlining threshold, not the cost of a comparison — and a flash argument that
   turns on the inliner's mood is not an argument.

   **Two things the measurement turned up that the item did not anticipate.**

   The pair is invisible to the linker almost everywhere. Of 45 declared pairs, **35 appear in
   the image as neither half** - fully inlined or dropped - and only three (`getPixel`,
   `setPixel`, `writePixel`) exist as both. So for most of the pattern there is no runtime
   object to be cheap or expensive about; it is upkeep and nothing else, which is exactly what
   this item suspected.

   And in the core, `Fast` does not mean unchecked. `Display::setPixel(Column, Row)` validates
   nothing itself: it computes an index and hands it to `Pixels::setPixel`, which is where the
   only check in the chain lives. The two halves in `Display` differ in whether the caller is
   *told*, not in whether anything is verified — so half the pattern is not a safety mechanism
   at all.

   **The time argument, which no host can measure, comes out negligible by arithmetic.** The
   frame path is 110 accesses, and `render()` returns early unless the buffer is dirty, so the
   worst case is a display changing on every 10 ms tick: 11 000 checks a second. At a generous
   four cycles each that is 44 000 of 24 000 000, **0.18 % of the part**, and a clock showing a
   settled face pays none of it.

   So by this item's own rule the answer is to drop one half rather than to guard it - with one
   wrinkle worth deciding before anyone starts. The halves are not symmetrical for *getters*:
   `getPixelFast(Index)` answers the value, `getPixel(Index, Pixel)` answers a code through an
   out-parameter, and dropping the first would make every reader of a pixel worse to read.

   **Done**, by the second of the two options: the check moved into the single implementation
   and the name went with the pattern. There is one accessor per operation now, and where a
   reader has two forms the argument list says which - `getPixel(Index, Pixel)` answers a code,
   `getPixel(Index)` answers the pixel, and **both check**. Writers have one form, which
   answers; a caller with nothing to do with that answer ignores it. 440 mentions across the
   core and all four backends, and the image came out **586 bytes smaller** - 48 664 bytes
   before, 48 078 after, RAM unchanged to the byte. Six things it settled:

   - **The wrinkle decided itself, and not by taste.** Two writers cannot differ in their
     return type alone, so a writer's unchecked half had nowhere to go but out. Readers differ
     in their argument list, so both of theirs live under one name - and the one answering a
     value needs something to answer for an index that is not there. That is a default in every
     case: an unlit pixel, a null character, a word of zero length, an empty glyph row. None of
     them can be mistaken for something that is on the display, which is what makes the answer
     honest rather than a zero that reads like data.
   - **The unchecked read did not disappear, it went private.** `getDisplayCharactersTableElement`,
     `getDisplayWordsTableElement`, `getFontTableElement` and the pixel buffer are the primitives
     both public forms go through, so each path checks exactly once and the class keeps one place
     that touches the table.
   - **A column past the last one used to light the next row.** `Display::setPixel(Column, Row)`
     computed an index and handed it to `Pixels`, where 11 on an 11-wide display is a perfectly
     valid index - the first letter of the row below. So the write was neither refused nor put
     where it was asked for. `isColumnAndRowValid` is asked by every entry point that takes a
     column now, and the case that found it is in the tests; reading the code had not.
   - **Two branches that had never compiled fell out.** `FontChar`'s checked `setRow` and
     `setColumn` assigned a member from a `const` function, and `Display::togglePixel`'s
     serpentine branch passed a pointer where a reference was expected. A template nobody
     instantiates and a `#if` branch nobody selects are not code, which is this item's own thesis
     arriving from the other side - and the glyph accessors are instantiated by a test now.
   - **The duplication was worth more than the checks were.** That is where the 586 bytes come
     from, and not from the bounds tests: `Text::setChar` existed as two 900-byte bodies inlined
     into a caller each and is one function now, and six `#if` ladders in `Display` mapping a
     column and a row onto an index became one `toIndex`. Of the 45 declared pairs, nine had both
     halves in the image at 486 bytes; `Display::setPixel(Column, Row)` alone was 258 bytes of
     pair and is 78 as one.
   - **One substitution had to be moved rather than dropped.** The scrolling text relied on the
     unchecked `setChar` drawing a space for a character it could not map, which is what clears
     the cells the previous shift step wrote; the surviving `setChar` refuses, because a test
     says it must. So the shift task substitutes the space itself, where it can be read - and a
     case fails if it stops. The `[[nodiscard]]` that used to police the four `Text` twins went
     with them, and so did the `assert` in three backends' unchecked writers: what they caught is
     what the two new cases check, in CI rather than under a debugger.

10. **Feed the ESP32's RMT channel by DMA.** Today the frame is refilled from the driver's
    interrupt, so the strip's timing depends on that interrupt being served: a block holds
    48 symbols, about 58 µs of output, and the part is running WiFi at the same time. A
    missed refill stalls the data line, and a WS2812 reads a long enough gap as the end of a
    frame — which shows up as a display latching a picture shifted by a pixel, the failure
    that reads like a wiring fault.

    `flags.with_dma` in [Pixels.cpp](../platform/esp32/src/Pixels.cpp) is the whole change,
    and it takes the deadline away rather than widening it. Two reasons it is not just done:
    **only the S3 can do it** — `SOC_RMT_SUPPORT_DMA` is absent on the classic ESP32, the S2,
    the C3 and the C6 — so it needs a case distinction rather than a flag, and
    `mem_block_symbols` stops meaning blocks and starts meaning a DMA buffer, so the constant
    beside it has to be reconsidered at the same time.

    What settles it is not a build. Nothing in this backend has ever driven a strip, so the
    interrupt that this would remove has never been observed missing a deadline; the honest
    order is a first board and an oscilloscope, then this.

11. **A Bluetooth transport for the command set.** ednieuw's clock is reachable over the
    Nordic UART service with NimBLE, from a phone and from a browser terminal, and the same
    single-letter commands arrive over it as over the serial line. Here that is a fourth
    consumer of a command set that already has three, so the parser and the catalog would not
    move at all.

    What it is *not* is first-boot provisioning - item 1 settled that with an access point,
    and a clock that already serves its console over WiFi gains nothing from a second way to
    do the same thing. Its value is the clock that is **already on the wall**: no cable
    reaches it, and a WiFi it can no longer join is exactly the fault that leaves no way in.
    Bluetooth answers when the network does not.

    It belongs on the platform seam rather than in the core, with the same honesty as the
    power switch: the ESP32-S3 and the Pico 2 W have a radio, the AVR128DA48 has none, and
    the simulator would have to fake one - so a backend that cannot do it says so rather
    than being compiled out.

12. **A second language in the word tables.** ESPWortuhr carries 35 layouts across 13
    languages, as one header per layout under `include/WordClockTypes/` with a generated
    `ClockType.gen.h` collecting them, chosen from a dropdown at runtime. That is the shape
    to copy if this is ever wanted, and the mechanism it needs is half here already: four
    German wordings switch at runtime through `Clock::ModeType`
    ([Clock.h](../firmware/inc/Clock/Clock.h)), so "several ways to say the time, chosen
    while running" is a solved problem rather than a new one.

    What is German is deeper than the tables, though, and that is the reason this is an idea
    and not a plan. `DisplayWords::WordIdType` names its entries `WORD_FUENF` and
    `WORD_HOUR_ZWOELF`, which is cosmetic; the letter grid in `DisplayCharacters` is not,
    because it *is* the front plate. A second language is therefore a second front plate,
    and its size need not be 11×10 - which turns this from a table into a layout
    abstraction, with the column and row counts becoming values instead of the compile-time
    constants they are today.

    So the honest order is the same as the DMA item's: this is worth doing when a second
    plate exists to justify it, and guessing at the abstraction beforehand would fix the
    wrong things. The AVR128DA48 also has a say - runtime-switchable layouts cost flash that
    the part may not have to spare, and it may end up carrying one layout where the ESP32
    carries several.

13. **MQTT and Home Assistant discovery.** Both comparison projects have it, ednieuw's over
    the JSON light schema, and it is the one feature of theirs a user would notice missing:
    a clock that dims with the rest of the house rather than on its own schedule.

    It fits the architecture better than its size suggests - another transport onto the
    existing command set, like item 11 - but it is not free the way that one is: it needs a
    broker to talk to, a reconnect policy for when the broker is the thing that is down, and
    a discovery payload that has to keep agreeing with what Home Assistant expects across
    its releases. That last part is the real cost, because it is upkeep rather than work.
    Wanted, but after 11 and only if the house it joins actually runs one.

Deliberately not planned: weather reports, MP3 playback and alarms, games on the
display, and an **IR receiver**. The first four are what wordclock24h grew over years and
none of them is a word clock. The receiver is a different kind of no: a handset is a
thing to lose, with a flat battery when it is found and a line of sight to keep, in front
of a clock that is on the network anyway. What it was wanted for - control without a
phone - is better answered by a knob on the case, which section 4 now serves.

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
