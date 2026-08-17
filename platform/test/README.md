# Shared backend tests

The host-test cases that are not about any one platform. Both the ESP32 and the RP2350
harness compile these against their own backend and their own stand-ins, so what they pin
down is checked twice over on two different frameworks — and written down once.

```
platform/test/
├── check.h     the one assertion the tests share, and the tally main() returns
└── cases/
    ├── ds3231_test.cpp   the clock chip in both directions
    └── serial_test.cpp   a command's path from a socket into the parser and back
```

The rule for what belongs here is simple and worth keeping: **a case belongs here when its
own source names neither platform.** These two reach the backend only through `Wire` and
through `WordclockSerial`, both of which every backend provides under the same name — so
the case reads the same whichever one is underneath.

`frame_test.cpp` and `web_test.cpp` fail that test and stay with their platforms. The first
compares against a byte array on one and a word array on the other, because that is the
shape each peripheral reads; the second drives a C API on one and an object on the other.
Making either of them platform-independent would mean an abstraction over the difference
that the difference is the point of.

## Running them

There is no runner here. Each platform's `test/run.sh` compiles these alongside its own
cases and links them against its own backend:

```bash
platform/esp32/test/run.sh
platform/rp2350/test/run.sh
```

A case added here is therefore built by both, and one that only compiles against one of
them breaks the other's job in CI rather than going unnoticed.
