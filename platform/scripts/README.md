# Shared build scripts

The build steps that are not about any one platform. Both PlatformIO projects reach out of
their own directory for these — `pre:../scripts/embed_web.py` in their `platformio.ini` — and
so does each `test/run.sh`, so what they produce is the same on both backends and written down
once.

```
platform/scripts/
└── embed_web.py    web/ -> a C++ header the firmware serves out of flash: both pages, the
                    manifest and the two icons
```

The rule for what belongs here is the one [`platform/test/`](../test/README.md) uses, read
across: **a script belongs here when its own source names neither platform.** `embed_web.py`
passes — it reads files, compresses the two pages among them and writes arrays, and nothing in
it knows which controller will serve them.

`firmware_includes.py` fails it and stays with its platform, twice. The two copies look alike
and are not: one looks for `cores/esp32/Arduino.h` under `framework-arduinoespressif32`, the
other for `cores/rp2040/Arduino.h` under `framework-arduinopico` — and the second is named
after the RP2040 even on an RP2350, because one core serves both parts. A shared version would
have to carry a table of framework names and core directories, which is the platform knowledge
this directory exists to be free of.
