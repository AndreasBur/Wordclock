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
| 3 | Display Brightness | `-B<0-255>` `-A<0\|1>` `-G<0\|1>` | Brightness, auto-brightness, gamma correction |
| 4 | Display Pixel | `-I<index>` `-S<0\|1>` | Set a single LED on/off; out-of-range index → error |
| 5 | Overlay Date | `-P -E -M -D -V -A` | See overlay options |
| 6 | Overlay Temperature | `-P -E -M -D -V -A` | See overlay options |
| 7 | Overlay Text | `-P -E -M -D -V -T -S -F -A` | Text overlay adds text/speed/font |
| 8 | Clock Mode | `-M<mode>` (uint8) | Sets the clock display mode |
| 9 | Animation | `-A<id>` `-M<mode>` `-S<speed>` `-F<0\|1>` | Animation id, selection mode, speed, favourite flag |
| 10 | Time | `-H<hour>` `-M<min>` `-S<sec>` | Sets RTC time |
| 11 | Date | `-Y<year>` (uint16) `-M<month>` `-D<day>` | Sets RTC date |

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
| 8 | Cube | A rectangle collapses towards the centre and grows back out, uncovering the new time |
| 9 | Flicker | The display flickers a few times, then the new time is there |
| 10 | Squeeze | Every word of the new time grows out of its first letter |
| 11 | Implode | The picture is pulled apart into its four quadrants, then the new time comes back together towards the centre |
| 12 | Explode | The words of the new time travel one after the other from the corner to their place |
| 13 | Matrix | Falling code rain in the display color; the new time stays behind as the drops pass over its letters |
| 14 | Roll | The previous time rolls out while the new one rolls in behind it, direction drawn per minute — never empty |

### Animation mode (`command 9 -M<mode>`)

The mode decides which animation runs on a minute change. `-A` and `-M` are
independent: `-A` selects the animation of `-M0`, and switching back to `-M0` returns
to it.

| mode | Meaning |
|------|---------|
| 0 | Fixed — always the animation selected with `-A` |
| 1 | Random — draws one of the animations 1–14 per minute |
| 2 | Sequence — runs the animations 1–14 in order, one per minute |

A mode only selects while no animation is running, so a minute change during a
running animation is ignored, exactly as the animations themselves ignore it.
Selecting an animation with `-A` shows it right away even in mode 1 or 2, which then
take over again on the next minute change.

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

`-S<speed>` sets the task cycle of the animation selected with `-A`: the animation
task runs every `speed` scheduler ticks, so **lower is faster** and `0` stops it
entirely ([`Scheduler::isCycleHit`](../firmware/src/Scheduler/Scheduler.cpp)). Default
is `ANIMATIONS_TASK_CYCLE_INIT_VALUE` = 10. Every animation keeps its own speed, and
modes 1 and 2 use the speed of whichever animation they picked.

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

The RPC answer is `RpcId=<id> Error=<code>`. An unknown or missing id (including
`0`, e.g. when `-P` is omitted) is rejected with `Error=8`
(`ERROR_RPC_ID_UNKNOWN`) instead of being silently accepted.

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
| 7 | `ERROR_UNKNOWN` | Unspecified failure |
| 8 | `ERROR_RPC_ID_UNKNOWN` | RPC command with an unknown/missing `-P<id>` |

## Examples

```
2 -R255 -G128 -B0     # set display color to orange and show it
4 -I5 -S1             # turn pixel 5 on
1 -P7                 # run display test
9 -A13                # show the matrix animation from now on
9 -M2                 # cycle through all animations, one per minute
9                     # query animation, mode and speed
10 -H14 -M30 -S0      # set time to 14:30:00
10                    # query current time (echoes H=.. M=.. S=..)
```
