#!/bin/sh
# Names every program this image is meant to carry, and fails listing the ones it does not.
#
# This replaced three --version calls in devcontainer-feature.json, which answered for the
# simulator build and for nothing else. That is how a container built before doxygen was
# added to ../Dockerfile went on passing the check: tools/docs.py, a step of the "Doxygen
# blocks" job in static-analysis.yml, then died with a FileNotFoundError traceback instead
# of a message - and only on the day someone edited a Doxygen block, long after the rebuild
# that would have fixed it.
#
# Every program below is installed unconditionally by ../Dockerfile, so a missing one means
# the image is older than the line that installs it. That is a rebuild and never a choice,
# which is why this fails rather than warns: the cost of being told at creation is one line
# in a log, and the cost of not being told is a job that fails in CI.
set -e

# One name per program, each the one a script or a CI job actually calls. avr-objcopy is
# here next to avr-gcc because the AVR image is sized through it and a toolchain can carry
# the compiler without the binutils; the Playwright environment is a path rather than a
# name, as it is deliberately not on PATH.
TOOLS='cmake ninja wx-config
       avr-gcc avr-objcopy
       pio
       clang-tidy run-clang-tidy cppcheck doxygen
       python3 gh
       xdotool xwininfo import rsvg-convert'
PLAYWRIGHT=/opt/playwright/bin/python

missing=''
for tool in $TOOLS; do
    command -v "$tool" >/dev/null 2>&1 || missing="$missing $tool"
done
[ -x "$PLAYWRIGHT" ] || missing="$missing $PLAYWRIGHT"

if [ -n "$missing" ]; then
    echo "Wordclock toolchain check: missing$missing" >&2
    echo "This image is older than the .devcontainer/Dockerfile line that installs it." >&2
    echo "Rebuild the container (Dev Containers: Rebuild Container)." >&2
    exit 1
fi

# The three the check used to print. Their versions decide something: wx-config's is what
# find_package(wxWidgets) matches, and the other two are what CI pins its runners to.
echo "Wordclock toolchain check: complete."
wx-config --version
cmake --version | head -1
pio --version
