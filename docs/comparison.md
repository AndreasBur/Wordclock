# The projects this one was measured against

A word clock is a well-populated idea, and most of what is worth deciding has been
decided somewhere else first. This is what was read, what each one contributed, and
where this project ended up ahead — kept separately from the [roadmap](roadmap.md),
because a yardstick does not change when work gets done.

## wordclock24h

[ukw100/wordclock24h](https://github.com/ukw100/wordclock24h) (Frank Meyer, documented
at [mikrocontroller.net](https://www.mikrocontroller.net/articles/WordClock_mit_WS2812))
is the most complete of the published word clocks and the one this project overlaps with
most. The backlog was originally written as a comparison against it, item by item, which
is why the finished work reads as a list of things it already had: WiFi provisioning, a
night switch, OTA, an RTC with a battery, colour animations.

It is also the source of the *deliberately not planned* list. Weather reports, MP3
playback, alarms and games are what it grew over years, and seeing them together is what
made it clear that none of them is a word clock.

## Multilayout-ESP-Wordclock

[ESPWortuhr/Multilayout-ESP-Wordclock](https://github.com/ESPWortuhr/Multilayout-ESP-Wordclock)
is the one to measure a configuration page against, and the one that ships a flashable
file per chip per release.

Its configuration page is what the panel at `/` was built against: a sidebar of task
pages with a colour wheel, sliders and preset swatches. Four things came out of looking
at it, and all four are in the panel — the presets carry the weight rather than the
wheel, since nobody picks a colour twice; a numeric `#RRGGBB` field belongs beside the
wheel, or a colour found by dragging cannot be written down or restored; one labelled
slider beats two unlabelled ones, with the automatic that overrides it directly beneath;
and the clock face can be drawn in the page itself, which is the difference between
changing a colour and walking into the other room to see what it did.

Its 35 layouts across 13 languages are also the shape to copy if a second language is
ever wanted — one header per layout, collected by a generated file, chosen at runtime.
Why that is an idea rather than a plan is in the [roadmap](roadmap.md).

## Arduino-ESP32-Nano-Wordclock

[ednieuw/Arduino-ESP32-Nano-Wordclock](https://github.com/ednieuw/Arduino-ESP32-Nano-Wordclock)
contributed its *transports* rather than its structure: it reaches the same command set
over serial, Bluetooth, a browser and an SD card log through one pair of functions. That
is the argument for the Bluetooth item in the roadmap, and it is a good one — a command
set with three consumers takes a fourth without moving.

Its structure is the opposite of this one: a single `.ino` with global state and clock
faces chosen by `#define`. Nothing here should move towards it.

## Where this project is ahead

Worth writing down, because it is what the roadmap's items must not break.

- Fifteen transition animations with three selection modes, a speed per animation and a
  favourite flag ([Animations.h](../firmware/inc/Animation/Animations.h)).
- Four German regional wordings switchable at runtime
  ([Clock.h](../firmware/inc/Clock/Clock.h)).
- A platform abstraction with a full desktop simulator, so the firmware is developed and
  tested without hardware.
- A BH1750 over I²C with min/max calibration and gamma correction, rather than an LDR on
  an ADC pin.
- One command set for the serial line, the browser console and the simulator dialog,
  machine-readable in the
  [MessageCatalog](../firmware/inc/Communication/MessageCatalog.h) — a command added
  once appears in all three.
