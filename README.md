# Wordclock

A word clock based on Arduino: an 11×10 grid of letters that spells out the
time in German ("ES IST FÜNF NACH ZEHN"), with animations, overlays (date,
temperature, text) and a serial command interface.

## Repository layout

| Directory | Purpose |
|-----------|---------|
| [Wordclock_Simulator/](Wordclock_Simulator/) | **Canonical firmware** (`Arduino/`) + a wxWidgets desktop simulator. Develop and debug the clock logic on a PC. |
| [Wordclock_xmegaForArduino/](Wordclock_xmegaForArduino/) | Hardware target (Atmel xmega). Intended home of the on-device build. |
| [FontCreator/](FontCreator/) | Tool that generates the bitmap font tables (`Font*.cpp`). |

The firmware logic lives once, in
[Wordclock_Simulator/Arduino/](Wordclock_Simulator/Arduino/); the simulator
swaps only the hardware layer (LED output, real-time clock, light sensor) for
wxWidgets implementations. See the
[simulator README](Wordclock_Simulator/README.md) for build instructions
(CMake, dev container, Code::Blocks).

## History note

Several older, diverged copies of the firmware (`Wordclock/`,
`Wordclock_xmega_Template/`, and a `.7z` archive) were removed during cleanup.
They remain fully recoverable from the git tag `archive/legacy-forks`:

```bash
git checkout archive/legacy-forks -- <path>
```
