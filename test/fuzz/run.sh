#!/bin/bash
# Runs every fuzzer of this directory side by side for a while, and fails if one of them found
# something. What static-analysis.yml runs on every pull request, through `make fuzz`, and what
# that runs:
#
#   cmake --workflow --preset fuzz
#   test/fuzz/run.sh build-fuzz 300 build-fuzz/work
#
# Each fuzzer gets a directory of its own under the work directory, and runs in it: the
# simulator's store is a file in the working directory, and two fuzzers sharing one would load
# each other's settings. What it finds stays there - corpus/ the inputs that reached something
# new, crash-* an input that failed, log.txt what it said - and the corpus is read again on the
# next run, so a second run goes on from where the first stopped. A crash replays with the
# fuzzer and the file alone: build-fuzz/bin/CommandFuzz crash-<hash>.
#
# The seeds in corpus/<fuzzer> are read as well and never written to.

set -euo pipefail

if [ $# -ne 3 ]; then
    echo "usage: $0 <build directory> <seconds> <work directory>" >&2
    exit 2
fi

HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="$(cd "$1" && pwd)"
DURATION="$2"
mkdir -p "$3"
WORK="$(cd "$3" && pwd)"

# What each fuzzer is told beyond the duration. CommandFuzz's input is a web socket frame, which
# the firmware refuses above WEB_FRONTEND_MAX_FRAME_LENGTH. SettingsFuzz' input is laid over a
# stored blob, at most Storage::kCapacity long, and without -len_control=0 libFuzzer would take
# its time growing an input that far - one run left all but the first 25 bytes untouched.
OptionsOf() {
    case "$1" in
        CommandFuzz)  echo "-max_len=192 -dict=$HERE/CommandFuzz.dict" ;;
        SettingsFuzz) echo "-max_len=256 -len_control=0" ;;
        *)            echo "" ;;
    esac
}

names=()
pids=()
for source in "$HERE"/*Fuzz.cpp; do
    name="$(basename "$source" .cpp)"
    mkdir -p "$WORK/$name/corpus"
    seeds=()
    if [ -d "$HERE/corpus/$name" ]; then seeds=("$HERE/corpus/$name"); fi
    # shellcheck disable=SC2046 # the options are meant to split into their flags
    (cd "$WORK/$name" && "$BUILD/bin/$name" -max_total_time="$DURATION" -print_final_stats=1 \
        $(OptionsOf "$name") corpus "${seeds[@]}" > log.txt 2>&1) &
    names+=("$name")
    pids+=("$!")
done

status=0
for index in "${!names[@]}"; do
    name="${names[$index]}"
    if wait "${pids[$index]}"; then
        echo "$name: $(grep -E '^#[0-9]+.*DONE' "$WORK/$name/log.txt" | tail -1)"
    else
        status=1
        echo "$name: FAILED - $WORK/$name/log.txt"
        grep -E -A40 'ERROR|runtime error|Fuzz[A-Za-z]+:' "$WORK/$name/log.txt" | head -80 || true
    fi
done
exit "$status"
