#!/usr/bin/env bash
#
# Builds and runs the host tests for the RP2350 backend.
#
# The backend is compiled with the host compiler against the stand-ins in stubs/ rather
# than the real framework. That leaves the hardware out of reach - the pulse timing, the
# sensor's bus, the network - but it does reach everything above it: the frame the driver
# hands over, the path a command takes from a web socket into the parser, and what comes
# back. Those are the parts a board would not show any more clearly.
#
#   test/run.sh              build and run the tests
#   test/run.sh clean        throw the object cache away
#
# There is no `serve` here. The page is shared with the ESP32 backend, so the host that puts
# a browser in front of it only has to exist once - platform/esp32/test/run.sh serve.
#
# Every source is compiled once into an object cache under .pio/ and the binaries are
# linked from those objects. It used to compile straight to executables, which meant the
# whole firmware core - some thirty files - was compiled once per binary and again on
# every run, because the binaries were built in a temporary directory that was deleted
# afterwards. That was three quarters of a minute for a one-line change.
#
set -euo pipefail

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM_DIR="$(cd "$TEST_DIR/.." && pwd)"
ROOT="$(cd "$PLATFORM_DIR/../.." && pwd)"
# The cases that are not about this platform. They compile this backend's sources against
# this backend's stubs, but the source of the case itself says nothing about either - so it
# lives once and is built by whichever backend is being tested.
SHARED_TEST_DIR="$ROOT/platform/test"

# Objects survive a run; the linked binaries do not. Under .pio/ because that is where
# this platform's build output lives and what .gitignore already covers.
CACHE="$PLATFORM_DIR/.pio/host-tests"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

if [ "${1:-}" = "clean" ]; then
    rm -rf "$CACHE"
    echo "object cache removed"
    exit
fi

mkdir -p "$CACHE"

# The platform's own headers first: its Arduino.h shadows the stub and has to win.
INCLUDES=(-I"$PLATFORM_DIR/include" -I"$TEST_DIR" -I"$SHARED_TEST_DIR" -I"$TEST_DIR/stubs" -I"$ROOT/firmware/inc")
while IFS= read -r directory; do INCLUDES+=(-I"$directory"); done < <(find "$ROOT/firmware/inc" -type d)

# The path this platform's Arduino.h includes the core's own through. On the target the
# build script finds it in the framework; here it is the stub.
INCLUDES+=(-DWORDCLOCK_CORE_ARDUINO_H="\"$TEST_DIR/stubs/Arduino.h\"")

FLAGS=(-std=gnu++17 -Wall -Wextra -Werror)

# WebInterface serves a header the build generates from the page. Produced by the same
# script PlatformIO calls, so the tests compile against what the device would carry. Into
# the cache rather than into the temporary directory: the include path is part of what an
# object was compiled with, and one that changed every run would invalidate the cache
# every run. A changed page is noticed through the dependency file instead.
python3 "$PLATFORM_DIR/scripts/embed_web.py" "$ROOT/web/index.html" "$CACHE"
INCLUDES+=(-I"$CACHE")

# What the objects were compiled with. Anything else means they cannot be reused, and the
# cheapest correct answer to that is to throw them away.
KEY_FILE="$CACHE/flags"
KEY="${FLAGS[*]} ${INCLUDES[*]}"
if [ ! -f "$KEY_FILE" ] || [ "$(cat "$KEY_FILE")" != "$KEY" ]; then
    find "$CACHE" -name '*.o' -delete
    find "$CACHE" -name '*.d' -delete
    printf '%s' "$KEY" > "$KEY_FILE"
fi

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
    "$PLATFORM_DIR/src/System.cpp"
    "$PLATFORM_DIR/src/WordclockMain.cpp"
    "$PLATFORM_DIR/src/WordclockSerial.cpp"
    "$PLATFORM_DIR/src/WS2812Pio.cpp"
)
WEB="$PLATFORM_DIR/src/WebInterface.cpp"

SHARED=("$TEST_DIR/stubs/hardware_port.cpp" "$TEST_DIR/stubs/stubs.cpp")

# One object per source, named after the path it came from so that two files of the same
# name - there are several - cannot land on each other.
objectOf() {
    local relative="${1#"$ROOT"/}"
    printf '%s/%s.o' "$CACHE" "${relative//\//_}"
}

# An object is reusable while it is newer than every file the compiler recorded as its
# input - the source and every header it reached, which is what the .d file lists.
isUpToDate() {
    local object="$1" dependency="${1%.o}.d"

    [ -f "$object" ] && [ -f "$dependency" ] || return 1

    local file
    for file in $(sed -e 's/^.*://' -e 's/\\//g' "$dependency"); do
        [ -e "$file" ] || return 1
        [ "$file" -nt "$object" ] && return 1
    done
    return 0
}

compile() {
    local source="$1" object; object="$(objectOf "$1")"

    isUpToDate "$object" && return 0

    echo "compiling ${source#"$ROOT"/}"
    # A failure is recorded rather than raised: this runs as a background job, where a
    # non-zero exit would end the job and nothing else.
    g++ "${FLAGS[@]}" "${INCLUDES[@]}" -MMD -MF "${object%.o}.d" -c "$source" -o "$object" \
        || touch "$CACHE/failed"
}

# Everything any of the binaries needs, compiled once and in parallel. The compiler is
# single-threaded and there are more than thirty files, so this is where the wall clock
# goes.
compileAll() {
    local jobs; jobs="$(nproc)"
    local source

    rm -f "$CACHE/failed"

    for source in "$@"; do
        compile "$source" &
        while [ "$(jobs -rp | wc -l)" -ge "$jobs" ]; do wait -n; done
    done
    wait

    if [ -f "$CACHE/failed" ]; then
        rm -f "$CACHE/failed"
        echo "compilation failed" >&2
        exit 1
    fi
}

link() {   # name, then the sources that belong to it
    local name="$1"; shift
    local objects=() source

    for source in "$@" "${SHARED[@]}"; do objects+=("$(objectOf "$source")"); done

    g++ "${FLAGS[@]}" -o "$WORK/$name" "${objects[@]}"
}

compileAll "${CORE[@]}" "${BACKEND[@]}" "${SHARED[@]}" "$WEB" \
           "$TEST_DIR/cases/frame_test.cpp" "$SHARED_TEST_DIR/cases/serial_test.cpp" "$SHARED_TEST_DIR/cases/ds3231_test.cpp" \
           "$TEST_DIR/cases/web_test.cpp" "$TEST_DIR/stubs/pio_stubs.cpp"

# frame_test brings its own PIO and DMA calls, because it keeps the frame that was handed
# over. It still needs WordclockSerial, which Pixels reports a failed driver through.
link frame_test  "$TEST_DIR/cases/frame_test.cpp" "$PLATFORM_DIR/src/Pixels.cpp" \
                 "$PLATFORM_DIR/src/WS2812Pio.cpp" "$PLATFORM_DIR/src/WordclockSerial.cpp"
link serial_test "$SHARED_TEST_DIR/cases/serial_test.cpp" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/stubs/pio_stubs.cpp"
link ds3231_test "$SHARED_TEST_DIR/cases/ds3231_test.cpp" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/stubs/pio_stubs.cpp"
link web_test    "$TEST_DIR/cases/web_test.cpp" "$WEB" "${BACKEND[@]}" "${CORE[@]}" "$TEST_DIR/stubs/pio_stubs.cpp"
# No `serve` here. The page is shared with the ESP32 backend now, so the host that puts a
# browser in front of it only has to exist once - see platform/esp32/test/run.sh, which
# serves the same file.

FAILED=0
for name in frame_test serial_test ds3231_test web_test; do
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
