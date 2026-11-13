#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs examples/SelfCheck in simavr, for each target in each variant, and compares what it prints
# with what it should, see expected_of(). The sketch ends itself by sleeping with interrupts off,
# which ends simavr. Leaves out a target whose SIMAVR_MCU is empty: simavr does not know every
# board.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# A self check ends within a second; one that runs longer has hung, or crashed into a loop of
# resets, as a call through a null pointer jumps to address 0.
readonly TIMEOUT_SECONDS=10

# Prints the file of what the self check prints in a variant on the selected target, the first
# of <example>.<target>.<variant>.expected, <example>.<target>.expected,
# <example>.<variant>.expected and <example>.expected there is.
expected_of() {
  local variant="$1"
  local base="${ROOT_DIR}/examples/${SELF_CHECK_EXAMPLE}/${SELF_CHECK_EXAMPLE}"
  local file
  for file in "${base}.${TARGET}.${variant}" "${base}.${TARGET}" "${base}.${variant}"; do
    if [[ -f "${file}.expected" ]]; then
      echo "${file}.expected"
      return
    fi
  done
  echo "${base}.expected"
}

# Prints what the sketch sent on the UART, and returns the exit status of simavr: 124 if it did
# not end in time. simavr writes the UART to stderr, a line in color with CR LF shown as "..",
# and its own messages to stdout.
run_in_simavr() {
  local elf="$1"
  local uart
  local status=0
  uart="$(timeout "${TIMEOUT_SECONDS}" simavr -m "${SIMAVR_MCU}" -f "${SIMAVR_FREQUENCY}" \
    "${elf}" 2>&1 >/dev/null)" || status=$?
  sed -e 's/\x1b\[[0-9;]*m//g' -e 's/\.\.$//' <<< "${uart}" \
    | grep -v '^$' || true
  return "${status}"
}

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi

  local failed=0
  local ran=0
  local target
  local variant
  local label
  local elf
  local actual
  local status
  local expected
  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    if [[ -z "${SIMAVR_MCU}" ]]; then
      echo "self_check.sh: no SIMAVR_MCU for ${target}, which simavr cannot run."
      continue
    fi
    for variant in "${VARIANTS[@]}"; do
      ran=1
      label="$(target_label "${variant}")"
      if ! target_build "${variant}" "${SELF_CHECK_EXAMPLE}"; then
        failed=1
        continue
      fi
      elf="$(target_elf "${variant}" "${SELF_CHECK_EXAMPLE}")"
      status=0
      actual="$(run_in_simavr "${elf}")" || status=$?
      if ((status == 124)); then
        err "${SELF_CHECK_EXAMPLE} (${label}) did not end within ${TIMEOUT_SECONDS} s:" \
          "it hung or crashed. What it printed until then:"
        failed=1
      elif ((status != 0)); then
        err "${SELF_CHECK_EXAMPLE} (${label}): simavr ended with ${status}."
        failed=1
      fi
      expected="$(expected_of "${variant}")"
      if diff -u --label expected --label "${label}" "${expected}" <(echo "${actual}"); then
        echo "${SELF_CHECK_EXAMPLE} (${label}): as expected"
      else
        failed=1
      fi
    done
  done
  if ((!ran)); then
    echo "self_check.sh: no target with a SIMAVR_MCU in tools/project.sh, nothing to run."
  fi
  exit "${failed}"
}

main "$@"
