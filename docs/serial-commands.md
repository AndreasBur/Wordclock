# Serial command reference

The firmware exposes a line-based serial command interface. This document is
generated from the parser sources under
[firmware/inc/Communication/MessageParser/](../firmware/inc/Communication/MessageParser/).

## Wire format

```
<command-number> [-<opt><value> ...]\n
```

- The command is a **leading integer**
  ([`atoi`](../firmware/inc/Communication/MessageParser/MsgCmdParser.h) of the
  message).
- Options are introduced by `-`, then a **single-letter short name**, then the
  value directly (e.g. `-R255`), space-separated
  ([`MsgParameterParser::parse`](../firmware/inc/Communication/MessageParser/Base/MsgParameterParser.h)).
- Messages are terminated by newline `\n`
  ([`Communication::EndOfMessageChar`](../firmware/inc/Communication/Communication.h)).
- Every command **echoes back** the command number plus current values as
  `X=value` pairs. Sending a command with no options acts as a **query**: values
  are read back without being changed.
- Argument types supported in this build: `uint8`, `uint16`, and `string` only.

## Top-level commands

Command numbers below are for the default build (all overlays `STD_ON` in
[`Overlays.h`](../firmware/inc/Overlay/Overlays.h)). The enum values shift if an
overlay is disabled at compile time.

| # | Command | Options | Notes |
|---|---------|---------|-------|
| 0 | *(none)* | — | Reserved; empty/invalid command → `ERROR_WRONG_COMMAND` |
| 1 | Remote Procedure Call | `-P<id>` (uint8) | Fire-and-forget action; see RPC table below |
| 2 | Display Color | `-R<0-255>` `-G<0-255>` `-B<0-255>` | RGB color; triggers `Display.show()` |
| 3 | Display Brightness | `-B<0-255>` `-A<0\|1>` `-G<0\|1>` | Brightness, auto-brightness, gamma correction; see below |
| 4 | Display Pixel | `-I<index>` `-S<0\|1>` | Set a single LED on/off; out-of-range index → error |
| 5 | Overlay Date | `-P -E -M -D -V -A` | See overlay options |
| 6 | Overlay Temperature | `-P -E -M -D -V -A` | See overlay options |
| 7 | Overlay Text | `-P -E -M -D -V -T -S -F -A` | Text overlay adds text/speed/font |
| 8 | Clock Mode | `-M<mode>` `-I<0\|1>` | Regional wording, and whether "es ist" stands permanently |
| 9 | Animation | `-A<id>` `-M<mode>` `-S<speed>` `-F<0\|1>` | Animation id, selection mode, speed, favourite flag |
| 10 | Time | `-H<hour>` `-M<min>` `-S<sec>` | Sets RTC time |
| 11 | Date | `-Y<year>` (uint16) `-M<month>` `-D<day>` | Sets RTC date |
| 12 | Status | *(none)* | Read-only; version, uptime, illuminance, temperature, address, link quality, free memory |
| 13 | Network | `-S<name>` `-P<pass phrase>` | Which network the clock joins. The pass phrase goes in and never comes back |

### Overlay options (shared)

From
[`MsgCmdBaseOverlayParser.h`](../firmware/inc/Communication/MessageParser/Base/MsgCmdBaseOverlayParser.h):

| Opt | Meaning |
|-----|---------|
| `-P` | Period (minutes) |
| `-E` | Endurance (seconds) |
| `-M` | Month |
| `-D` | Day |
| `-V` | Valid-in-days |
| `-A` | Active (0/1) |
| `-T` | Text (string) |
| `-S` | Speed |
| `-F` | Font |

The Date and Temperature overlays ignore `-T` / `-S` / `-F`.

### Temperature overlay (`command 6`)

Shows the temperature of the DS3231 clock chip, as `23.4C` — one decimal, and the letter
rather than a degree sign, which the font tables do not carry
([`Text::convertCharToFontIndex`](../firmware/src/Text/Text.cpp) covers ASCII plus six
umlauts). What is measured is the chip's own die, so in a closed case it reads above the
room by whatever that case turns out to add.

Until the chip has answered once the overlay stays away entirely — it does not start in
its period, and `1 -P26` answers `Error=8`. A clock built without the chip therefore
never shows it, rather than showing a zero that reads like a measurement. In the
simulator the **Clock chip** group in the settings window stands in for it: the reading
counts only while *Sensor connected* is ticked, which is also how the "no chip" state is
reached again.

### Brightness (`command 3`)

`-B` sets the brightness that is *asked for*. What reaches the LEDs is that value
after gamma correction (`-G1`) and the light sensor (`-A1`), recalculated by the
display task, so the automatic follows the room without any command being sent.

The automatic scales the brightness by the sensor reading divided by the calibrated
maximum, which is why it needs a calibration first — RPC `1` with the sensor in the
brightest light it should ever see, RPC `2` in the darkest. Without it the maximum is
the sensor's full range and the display stays dim in a normal room.

Two bounds keep it usable: the automatic never dims below a floor, so a dark room
does not make the clock disappear, and it never brightens beyond what `-B` asked
for. A `-B0` therefore stays off, floor or not.

In the simulator there is no sensor to read, so the **Illuminance** slider in the
window stands in for it. It only has an effect while `-A1` is set.

### Clock modes (`command 8 -M<mode>`)

From [`Clock.h`](../firmware/inc/Clock/Clock.h). The modes are the regional ways of
saying the time in German, and they differ in exactly two places: how the quarters
are said, and whether 20 and 40 minutes past are counted from the hour or from the
half hour. Everything else is identical between them.

| mode | Name | 4:15 | 4:20 | 4:40 | 4:45 |
|------|------|------|------|------|------|
| 0 | Wessi *(default)* | viertel **nach** vier | zehn vor **halb** fünf | zehn nach **halb** fünf | viertel **vor** fünf |
| 1 | Ossi | **viertel fünf** | zehn vor **halb** fünf | zehn nach **halb** fünf | **dreiviertel fünf** |
| 2 | Rhein-Ruhr | viertel **nach** vier | **zwanzig nach** vier | **zwanzig vor** fünf | viertel **vor** fünf |
| 3 | Schwaben | **viertel fünf** | **zwanzig nach** vier | **zwanzig vor** fünf | **dreiviertel fünf** |

The quarters of Ossi and Schwaben name the hour they are counting towards, so
"viertel fünf" is a quarter past four, not a quarter past five.

`-I<0|1>` decides whether **ES IST** stands in every time or only in the two it is
said in — `-I0` lights it at the full and the half hour alone, `-I1` at every time.
It used to be a compile-time switch; the default is still what
[`CLOCK_SHOW_IT_IS_PERMANENTLY`](../firmware/inc/Clock/Clock.h) says, and a clock
that was never told otherwise comes up with it.

```
8 -M1 -I0             # Ossi, and "es ist" only where it is spoken
8                     # query both (echoes M=.. I=..)
```

### Animation ids (`command 9 -A<id>`)

From [`Animations.h`](../firmware/inc/Animation/Animations.h). Like the command
numbers, these shift if an animation is disabled through its
`ANIMATIONS_SUPPORT_*` switch:

| id | Animation | What it looks like |
|----|-----------|--------------------|
| 0 | *(none)* | The new time simply appears, no transition |
| 1 | Cursor | A single pixel walks across all LEDs and leaves the letters of the new time lit behind it |
| 2 | Teletype | Writes the words of the new time letter by letter, one word after the other |
| 3 | Drop | The letters of the new time fall down their column into place |
| 4 | Shift | Shifts the previous time out of the display, then shifts the new one in — so the display is empty in between |
| 5 | Fade | Dims the display down to dark, changes the time, and fades it back up |
| 6 | Snake | A short snake travels along the LED strip and leaves the new letters lit behind it |
| 7 | Wipe | A diagonal line sweeps across the display and uncovers the new letters |
| 8 | Cube | A rectangle shrinks towards the centre and grows back out, leaving the new letters behind |
| 9 | Flicker | The display flickers a few times, then the new time is there |
| 10 | Squeeze | The words of the previous time are eaten away letter by letter, from the left or from the right as selected for each word change; the new ones grow out of their first letter |
| 11 | Explode pixels | The previous time is pulled into the centre, all of its pixels at once, until the display is empty; then the new time expands out of the centre the same way |
| 12 | Explode words | The same two halves, but word by word: every word of the previous time travels into the centre on its own, then the words of the new time come back out one after the other |
| 13 | Matrix | Falling code rain in the display color; the new time stays behind as the drops pass over its letters |
| 14 | Roll | The previous time rolls out while the new one rolls in behind it, direction selected for each word change — never empty |
| 15 | Collapse | The letters slide to one edge of their row and close every gap, then the new ones slide apart into their words; the edge is selected for each word change |

### Animation mode (`command 9 -M<mode>`)

The mode decides which animation runs on a word change. `-A` and `-M` are
independent: `-A` selects the animation of `-M0`, and switching back to `-M0` returns
to it.

| mode | Meaning |
|------|---------|
| 0 | Fixed — always the animation selected with `-A` |
| 1 | Random — draws one of the animations 1–15 on each word change |
| 2 | Sequence — runs the animations 1–15 in order, one on each word change |

A mode only selects while no animation is running, so a word change during a
running animation is ignored, exactly as the animations themselves ignore it.
Selecting an animation with `-A` shows it right away even in mode 1 or 2, which then
take over again on the next word change.

`-F<0|1>` decides whether the animation selected with `-A` takes part in modes 1 and
2. All animations do by default. The flag belongs to the animation, so it is set for
whichever id `-A` addresses, and a query reports the flag of that id:

```
9 -A9 -F0             # never show Flicker again in mode 1 or 2
9 -A13 -F1 -M1        # Matrix takes part again, and switch to random
```

The **last** remaining flag cannot be cleared — the selecting modes would be left with
nothing to pick, so the firmware answers `Error=4` and keeps it. Clearing the flag of
id 0 is rejected the same way, since `0` is not an animation.

`-S<speed>` sets the speed of the animation selected with `-A`: **higher is faster**,
`255` is the fastest and `0` stops the animation entirely. Speeds `1` through `255`
map one-to-one to task cycles `255` through `1`; the scheduler counts those cycles in
([`Scheduler::convertSpeedToTaskCycle`](../firmware/inc/Scheduler/Scheduler.h)), which
is why the two run in opposite directions. The default task cycle of `10`
(`ANIMATIONS_TASK_CYCLE_INIT_VALUE`) is reported as `S=246`. Every animation keeps its
own speed, and modes 1 and 2 use the speed of whichever animation they picked.

### Status (`command 12`)

What a remote procedure call cannot answer: its answer is `RpcId=<id> Error=<code>` and
carries no value, so the readings live in a command of their own.

```
12                    # -> 12 V=0.1.0 U=93 I=350 T=23.4C A=192.168.1.23 Q=-62 M=142
```

| field | Meaning |
|-------|---------|
| `V` | Firmware version, hand-kept in [`Version.h`](../firmware/inc/Version/Version.h) |
| `U` | Minutes since the last start. A `uint16` of them, so it wraps after 45 days |
| `I` | What the light sensor measures, in lux — the value the brightness automatic divides by its calibrated maximum |
| `T` | What the clock chip measures, `23.4C`, and **empty** while no chip has answered |
| `A` | The address to type into a browser, and **empty** while the clock has not joined a network |
| `Q` | Received signal strength in dBm, negative, and **empty** without a network |
| `M` | Free heap in KiB — the field that says whether a clock running for months is leaking |

An empty field means there is nothing behind it, which is not the same as a zero: a clock
built without the temperature chip answers `T=` for good, and one on the serial line alone
answers `A=` and `Q=`. In the simulator `U` is real and `M`, `A` and `Q` are always empty —
a host's free heap says nothing about the firmware.

The command takes no options. Its parameter table is empty, so `12 -V1` is answered with
`Error=3:V` (`ERROR_PARAMETER_UNKNOWN`) rather than silently accepted. The field names are
still described in the
[message catalog](../firmware/inc/Communication/MessageCatalog.h), marked read-only, which
is how the simulator dialog and the web console put labels on an answer without offering
the fields as inputs.

### Network (`command 13`)

Which network the clock joins, and the reason it no longer has to be flashed in.

```
13                              # -> 13 S=MyNetwork Error=0
13 -SMyNetwork -Psecret         # store the pair and join
13 -S -P                        # forget it, and open the access point again
```

Both halves or neither. A command carrying only one of them is refused with `Error=8`
rather than joining a network with the previous pass phrase or keeping a pass phrase for
the previous network — both of which are a clock that goes quiet with nothing to say why.

**The pass phrase is never answered.** A console that echoed it would leave it in the
scrollback of every browser watching the same clock.

**A clock with no network opens one.** It comes up as an open access point named
`Wordclock`, and the web console on it is where the credentials are entered — without that,
"configurable at runtime" would still mean a cable for the first time. The access point is
open on purpose: a pass phrase shared by every clock of this firmware protects nothing. It
closes as soon as a network is stored, so the window is the time between unpacking and
configuring.

Two limits come from the wire format rather than from this command:

- A value runs to the next `-`, so **neither the name nor the pass phrase may contain a
  hyphen**. This is the same rule the text overlay lives under.
- A value also carries the space that separates it from the next option, so a **trailing
  space is stripped** — a name or pass phrase cannot end in one over this interface.

What is stored lives beside the settings rather than in them: `1 -P30` (settings reset)
leaves the network alone, because a reset that threw it away would leave a clock nobody can
reach without a cable.

## RPC sub-commands (`command 1 -P<id>`)

From
[`MsgCmdRemoteProcedureCallParser.h`](../firmware/inc/Communication/MessageParser/MsgCmdRemoteProcedureCallParser.h):

| id | Action |
|----|--------|
| 0 | *(none)* |
| 1 | Illuminance calibration — set max value |
| 2 | Illuminance calibration — set min value |
| 3 | Display enable |
| 4 | Display disable |
| 5 | Display show |
| 6 | Display clear |
| 7 | Display test |
| 8 / 9 / 10 | Color red / green / blue **increment** |
| 11 / 12 / 13 | Color red / green / blue **decrement** |
| 14 / 15 | Brightness increment / decrement |
| 16 / 17 | Auto-brightness on / off |
| 18 / 19 | Gamma correction on / off |
| 20 / 21 | Power on / off — **reserved, not implemented yet.** These ids wait for the hardware switch that cuts the 5 V supply of the LED stripes via a controller port. They are accepted and answer `Error=0` without doing anything |
| 22 | Clock refresh — draw the current time again, right now and without an animation |
| 23 | Animation start — run the selected animation on the current time |
| 24 | Animation abort — end a running animation and put the time back |
| 25 / 26 / 27 | Overlay date / temperature / text — show it now instead of at its next period |
| 28 | Overlay abort — end the overlay that is showing |
| 29 | Settings save — write the configuration to the store now instead of within the next two seconds |
| 30 | Settings reset — every setting back to what a clock starts with, and the store emptied |
| 31 | System restart — carried out on the next tick, so this command's answer still goes out |
| 32 | Time resynchronise — ask the time server again, for the clock that came up while it was unreachable |
| 33 | Network reconnect — join the network again |

The RPC answer is `RpcId=<id> Error=<code>`. An unknown or missing id (including
`0`, e.g. when `-P` is omitted) is rejected with `Error=7`
(`ERROR_RPC_ID_UNKNOWN`) instead of being silently accepted.

Ids `22` to `30` are the ones that can be refused: they answer `Error=8`
(`ERROR_UNKNOWN`, the general `E_NOT_OK`) when they could not be carried out —
`29` and `30` when the store did not take the write, the rest when the display is busy
with something else rather than doing it anyway. The clock cannot be refreshed and no animation
started while an overlay owns the display; an overlay cannot be shown while another
one is showing or while it is switched off (`-A0`); and aborting answers the same way
when nothing was running. `22` and `24` are what brings the clock face back after a
display test (`-P7`) or a hand-set pixel (command 4), which otherwise stands until
the next word change — up to five minutes.

## Error codes

Returned in the `Error=<code>` field
([`ErrorMessage::ErrorType`](../firmware/inc/Communication/ErrorMessage.h)):

| code | Name | Meaning |
|------|------|---------|
| 0 | `ERROR_NO_ERROR` | Success |
| 1 | `ERROR_MESSAGE_TOO_LONG` | Incoming message exceeded the buffer |
| 2 | `ERROR_WRONG_COMMAND` | Unknown/empty command number |
| 3 | `ERROR_PARAMETER_UNKNOWN` | Unknown option short name |
| 4 | `ERROR_VALUE_OUT_OF_BOUNDS` | Value outside the allowed range |
| 5 | `ERROR_NO_VALUE_GIVEN` | Option given without a value |
| 6 | `ERROR_DISPLAY_PENDING` | Display busy / show still pending |
| 7 | `ERROR_RPC_ID_UNKNOWN` | RPC command with an unknown/missing `-P<id>` |
| 8 | `ERROR_UNKNOWN` | Unspecified failure — what an `E_NOT_OK` from the firmware becomes |

## Examples

```
2 -R255 -G128 -B0     # set display color to orange and show it
4 -I5 -S1             # turn pixel 5 on
1 -P7                 # run display test
9 -A13                # show the matrix animation from now on
9 -M2                 # cycle through all animations, one per word change
9                     # query animation, mode and speed
10 -H14 -M30 -S0      # set time to 14:30:00
10                    # query current time (echoes H=.. M=.. S=..)
```
