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
| 9 | Animation | `-A<id>` `-S<speed>` | Animation id + speed |
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
| 20 / 21 | Power on / off — ⚠️ declared but **not handled** in the `process()` switch (currently no-ops) |

## Examples

```
2 -R255 -G128 -B0     # set display color to orange and show it
4 -I5 -S1             # turn pixel 5 on
1 -P7                 # run display test
10 -H14 -M30 -S0      # set time to 14:30:00
10                    # query current time (echoes H=.. M=.. S=..)
```
