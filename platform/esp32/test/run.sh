#!/usr/bin/env bash
#
# Builds and runs the host tests for the ESP32 backend, and optionally the local web
# console that serves the page with the firmware behind it.
#
# The backend is compiled with the host compiler against the stand-ins in stubs/ rather
# than the real framework. That leaves the hardware out of reach - the pulse timing, the
# sensor's bus, the network - but it does reach everything above it: the frame the driver
# hands over, the path a command takes from a web socket into the parser, and what comes
# back. Those are the parts a board would not show any more clearly.
#
#   test/run.sh              build and run the tests
#   test/run.sh serve [port] the same binaries, serving the console on localhost
#
set -euo pipefail

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM_DIR="$(cd "$TEST_DIR/.." && pwd)"
ROOT="$(cd "$PLATFORM_DIR/../.." && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# The platform's own headers first: its Arduino.h shadows the stub and has to win.
INCLUDES=(-I"$PLATFORM_DIR/include" -I"$TEST_DIR/stubs" -I"$ROOT/firmware/inc")
while IFS= read -r directory; do INCLUDES+=(-I"$directory"); done < <(find "$ROOT/firmware/inc" -type d)

# The path this platform's Arduino.h includes the core's own through. On the target the
# build script finds it in the framework; here it is the stub.
INCLUDES+=(-DWORDCLOCK_CORE_ARDUINO_H="\"$TEST_DIR/stubs/Arduino.h\"")

FLAGS=(-std=gnu++17 -Wall -Wextra -Werror)

# WebInterface serves a header the build generates from the page. Produced by the same
# script PlatformIO calls, so the tests compile against what the device would carry.
python3 "$PLATFORM_DIR/scripts/embed_web.py" "$PLATFORM_DIR/web/index.html" "$WORK"
INCLUDES+=(-I"$WORK")

CORE=()
while IFS= read -r source; do CORE+=("$source"); done < <(find "$ROOT/firmware/src" -name '*.cpp')

# WebInterface is not in here: it needs the HTTP server, and the two tests that want it
# bring their own so they can see what it sends. A test that has no business with HTTP
# should not have to stub it.
BACKEND=(
    "$PLATFORM_DIR/src/BH1750.cpp"
    "$PLATFORM_DIR/src/DS3231.cpp"
    "$PLATFORM_DIR/src/Pixels.cpp"
    "$PLATFORM_DIR/src/RealTimeClock.cpp"
    "$PLATFORM_DIR/src/Storage.cpp"
    "$PLATFORM_DIR/src/WordclockSerial.cpp"
)
WEB="$PLATFORM_DIR/src/WebInterface.cpp"

SHARED=("$TEST_DIR/hardware_port.cpp" "$TEST_DIR/stubs.cpp")

build() {   # name, then the sources that belong to it
    local name="$1"; shift
    echo "building $name"
    g++ "${FLAGS[@]}" "${INCLUDES[@]}" -o "$WORK/$name" "$@" "${SHARED[@]}"
}

# frame_test brings its own RMT calls, because it keeps the frame that was transmitted. It
# still needs WordclockSerial, which Pixels reports a failed channel through.
build frame_test  "$TEST_DIR/frame_test.cpp" "$PLATFORM_DIR/src/Pixels.cpp" "$PLATFORM_DIR/src/WordclockSerial.cpp"
build serial_test "$TEST_DIR/serial_test.cpp" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/rmt_stubs.cpp"
build temperature_test "$TEST_DIR/temperature_test.cpp" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/rmt_stubs.cpp"
build web_test    "$TEST_DIR/web_test.cpp" "$WEB" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/rmt_stubs.cpp"
build webhost     "$TEST_DIR/webhost.cpp" "$WEB" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/rmt_stubs.cpp"

if [ "${1:-}" = "serve" ]; then
    # Not exec'd: the binaries live in a temporary directory, and the trap that removes it
    # again only fires if this shell is still around to run it.
    node "$TEST_DIR/serve.js" "$PLATFORM_DIR/web/index.html" "$WORK/webhost" "${2:-8080}"
    exit
fi

FAILED=0
for name in frame_test serial_test temperature_test web_test; do
    echo
    echo "--- $name ---"
    "$WORK/$name" || FAILED=1
done

echo
if [ "$FAILED" -ne 0 ]; then
    echo "FAILURES"
    exit 1
fi
echo "all host tests passed"
