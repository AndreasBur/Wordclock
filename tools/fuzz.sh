#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs every fuzzer of test/fuzz/ side by side for FUZZ_SECONDS each, 60 if not set, and fails if
# one of them found something. `make fuzz` builds them first, with the preset fuzz, into
# build/fuzz/fuzz/.
#
# Each fuzzer runs in a folder of its own, build/fuzz/work/<Name>, and what it finds stays there:
# corpus/ the inputs that reached something new, crash-* an input that failed, log.txt what it
# said. The corpus is read again on the next run, so a second run goes on from where the first
# stopped. A crash replays with the fuzzer and the file alone: build/fuzz/fuzz/<Name> crash-<hash>.
#
# Beside a fuzzer's source in test/fuzz/, the project may keep:
#   corpus/<Name>/   inputs to start from, read and never written to
#   <Name>.dict      a dictionary of the tokens its input is made of, as libFuzzer reads one
#   <Name>.options   further options of libFuzzer, one per line, as -max_len=256

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly FUZZ_DIR="${ROOT_DIR}/test/fuzz"
readonly PROGRAM_DIR="${BUILD_DIR}/fuzz/fuzz"
readonly WORK_DIR="${BUILD_DIR}/fuzz/work"

# Prints the options of a fuzzer beyond the time, one per line: its dictionary and its
# <Name>.options, where it has them.
options_of() {
  local name="$1"
  if [[ -f "${FUZZ_DIR}/${name}.dict" ]]; then
    echo "-dict=${FUZZ_DIR}/${name}.dict"
  fi
  if [[ -f "${FUZZ_DIR}/${name}.options" ]]; then
    grep -v -e '^#' -e '^$' "${FUZZ_DIR}/${name}.options" || true
  fi
}

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi
  local seconds="${FUZZ_SECONDS:-60}"

  local found=""
  if [[ -d "${FUZZ_DIR}" ]]; then
    found="$(find "${FUZZ_DIR}" -maxdepth 1 \( -name '*Fuzz.cpp' -o -name '*_fuzz.cpp' \))"
  fi
  local sources=()
  if [[ -n "${found}" ]]; then
    mapfile -t sources <<< "${found}"
  fi
  if ((${#sources[@]} == 0)); then
    echo "fuzz.sh: no fuzzer in test/fuzz/, nothing to run."
    exit 0
  fi

  local names=()
  local pids=()
  local source
  local name
  local listed
  local options
  local seeds
  for source in "${sources[@]}"; do
    name="$(basename "${source}" .cpp)"
    if [[ ! -x "${PROGRAM_DIR}/${name}" ]]; then
      err "fuzz.sh: ${PROGRAM_DIR}/${name} is missing; make fuzz builds it."
      exit 1
    fi
    mkdir -p "${WORK_DIR}/${name}/corpus"
    listed="$(options_of "${name}")"
    options=()
    if [[ -n "${listed}" ]]; then
      mapfile -t options <<< "${listed}"
    fi
    seeds=()
    if [[ -d "${FUZZ_DIR}/corpus/${name}" ]]; then
      seeds=("${FUZZ_DIR}/corpus/${name}")
    fi
    # In its own folder, as a fuzzer writes what it finds into the folder it runs in.
    (cd "${WORK_DIR}/${name}" && "${PROGRAM_DIR}/${name}" -max_total_time="${seconds}" \
      -print_final_stats=1 "${options[@]}" corpus "${seeds[@]}" > log.txt 2>&1) &
    names+=("${name}")
    pids+=("$!")
  done

  local failed=0
  local index
  local done_line
  for index in "${!names[@]}"; do
    name="${names[${index}]}"
    if wait "${pids[${index}]}"; then
      done_line="$(grep -E '^#[0-9]+.*DONE' "${WORK_DIR}/${name}/log.txt" | tail -1 || true)"
      echo "${name}: ${done_line}"
    else
      failed=1
      err "${name}: FAILED, see ${WORK_DIR}/${name}/log.txt; it replays with" \
        "${PROGRAM_DIR}/${name} and the crash- file there:"
      grep -E -A40 'ERROR|runtime error|deadly signal' "${WORK_DIR}/${name}/log.txt" \
        | head -60 >&2 || true
    fi
  done
  exit "${failed}"
}

main "$@"
