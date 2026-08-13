# Hardware platform (interface contract — xmega, not implemented)

This directory is a **placeholder** for an xmega backend of the Wordclock firmware.
It is intentionally empty of code: that port would have to be built and tested on
the real xmega hardware with the AVR toolchain, which is out of scope for the
desktop tooling in this repo.

The document below is still the description of the platform seam, and worth reading
as such. For a backend that exists, see [`../esp32/`](../esp32/) — the xmega is no
longer the intended target.

## How the platform seam works

The firmware core in [`../../firmware/`](../../firmware/) is platform-agnostic. It
reaches the hardware **only through header names**, resolved via the compiler's
include path:

```cpp
#include "Arduino.h"        // core runtime
#include "Pixels.h"         // LED matrix
#include "RealTimeClock.h"  // time source
#include "BH1750.h"         // ambient light sensor
```

A platform backend is simply a directory that provides these headers (plus an
application entry point) and is placed on the include path instead of
[`../simulator/include/`](../simulator/include/). No `#include` in the core
changes between platforms — this is a compile-time swap with zero runtime cost
(no virtual dispatch, which matters on AVR).

The simulator backend in [`../simulator/`](../simulator/) is the reference
implementation to mirror.

## What a hardware backend must provide

| Header / unit | Contract | Hardware implementation |
|---------------|----------|-------------------------|
| `Arduino.h` | `byte`, `boolean`, `F()`, `PROGMEM`, `pgm_read_byte`, `memcpy_P`, `bitRead`, `itoa`, and a `Serial` object exposing `print` / `println` / `available` / `read` | The real Arduino/AVR core |
| `Pixels.h` | `Pixels` singleton: `getInstance`, `setPixel(Fast)` / `clearPixel(Fast)` / `getPixel(Fast)`, `setBrightness`, `show`, `clearPixels`, `init(pin)`; also doubles as the serial console (`print`/`read`) | WS2812 LED driver + UART |
| `RealTimeClock.h` | `RealTimeClock` singleton holding a `ClockDateTime`; the core only *reads* it via `getDateTime()` | Feed `setDateTime()` from an RTC chip (e.g. DS3231) |
| `BH1750.h` | Ambient-light driver exposing the illuminance reading the core consumes | BH1750 over I²C |
| app entry point | Equivalent of the simulator's `WordclockApp` / `WordclockMain`: initialise, then repeatedly tick `Scheduler::task()` and update the `RealTimeClock` | AVR `main()` / `setup()` + `loop()` |

The tick has to come every `Scheduler::getTaskIntervalMs()` milliseconds: every
module's task cycle counts in that unit, so a tick at another rate silently
rescales animation speed, the sensor's sampling interval and the serial poll rate
alike. Read the value rather than repeating the number, as the simulator's timer
does; a backend whose timer cannot be set that freely changes
`SCHEDULER_TASK_INTERVAL_MS` instead. Note that the BH1750 needs roughly 120 ms
per high-resolution conversion, which its task cycle must stay above.

## Suggested port path

For an **xmega** target: reuse the concrete drivers already present in
[`../../Wordclock_xmegaForArduino/`](../../Wordclock_xmegaForArduino/) (WS2812,
RTC, I²C, ArduinoCore) and adapt them to the interfaces above. That project
currently builds an **older** firmware architecture; the work is to retarget its
drivers at today's `firmware/` core, then build and flash with the AVR toolchain.

That advice does not carry over to [`../esp32/`](../esp32/), which is why none of
those three drivers appear there. The RMT peripheral generates the WS2812 pulses in
hardware, so the xmega's 800-line USART-as-SPI bit pusher has no counterpart; the
time comes from SNTP rather than from an RTC chip; and I²C comes with the Arduino
core. Only the BH1750's register handling was worth carrying across.

**Status:** contract only. Nothing here compiles or runs yet.
