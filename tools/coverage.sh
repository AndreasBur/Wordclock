#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs the tests on the host with --coverage and fails unless they reach every line, branch and
# condition of src/ (docs/Testing.md TS-4), or as much of them as COVERAGE_MINIMUM of
# tools/project.sh says: what no test reaches on the terminal, and all of it as HTML in
# build/coverage/html/. The variants of CMakeLists.txt count together, as each takes other
# branches of #if, and with them the suites coverage_suites() of tools/project.sh runs. gcovr
# runs from the environment uv.lock pins, like the tools of lint.sh.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly COVERAGE_DIR="${BUILD_DIR}/coverage"

# Removes the notes (.gcno) of a source that no longer exists, renamed or
# deleted: the build writes notes only for what it compiles and never
# removes old ones, and gcovr reads every one it finds, so the lines of a
# header that only such a file reached stood as missed. A note lies at
# <dir>/CMakeFiles/<target>.dir/<source>.gcno, which gives its source: <dir>
# is the build directory of the CMakeLists.txt that adds the target, empty for
# the root's and <subdir>/ for one of add_subdirectory(<subdir>), and <source>
# is relative to that CMakeLists.txt, with each .. written as __: a source outside the folder of
# its CMakeLists.txt, as ../../src/Display.cpp, lies at __/__/src/Display.cpp.gcno.
remove_stale_notes() {
  local notes
  notes="$(find "${COVERAGE_DIR}" -name '*.gcno')"
  local note
  local subdir
  local source
  while IFS= read -r note; do
    subdir="${note#"${COVERAGE_DIR}"/}"
    subdir="${subdir%%CMakeFiles/*}"
    source="${note#*.dir/}"
    source="${source%.gcno}"
    source="${ROOT_DIR}/${subdir}${source//__\//..\/}"
    if [[ -n "${note}" && ! -f "${source}" ]]; then
      rm -f "${note}"
    fi
  done <<< "${notes}"
}

# Fails on a line of src/ excluded from coverage without its reason (Testing.md TS-4): a marker
# that starts an exclusion is followed by a colon and the reason, as
# `// GCOVR_EXCL_LINE: the bus may hang, no test can make it`. The end of a block needs none.
check_exclusions() {
  local bare
  bare="$(grep -rnE 'GCOVR_EXCL_(BR_)?(LINE|START)' src/ \
    | grep -vE 'GCOVR_EXCL_(BR_)?(LINE|START): [^ ]' || true)"
  if [[ -n "${bare}" ]]; then
    err "An exclusion from coverage without its reason (Testing.md TS-4):"
    err "${bare}"
    exit 1
  fi
}

# Runs gcovr on the TRACEFILE with the further arguments given, the counts merged:
# - separate: a function under an #if of a variant lies on another line in
#   each, and is two functions, not one to merge.
# - --merge-lines: gcov counts a template per instance, and each lambda is a
#   type of its own, so a line reached by one instance stood as missed by
#   another; merged, a line counts as reached if any instance reaches it. The
#   tracefile keeps each instance, so it is merged where it is read.
# Arguments:
#   tracefile, then what gcovr is to write
report() {
  local tracefile="$1"
  shift
  uv run --frozen gcovr --root "${ROOT_DIR}" --json-add-tracefile "${tracefile}" \
    --merge-mode-functions=separate \
    --merge-lines \
    "$@"
}

# Fails unless COVERAGE_MINIMUM is three percentages, as gcovr would take a fourth in silence and
# read a word as 0.
check_minimum() {
  local valid=1
  if ((${#COVERAGE_MINIMUM[@]} != 3)); then
    valid=0
  fi
  local value
  for value in "${COVERAGE_MINIMUM[@]}"; do
    if [[ ! "${value}" =~ ^[0-9]+$ ]] || ((value > 100)); then
      valid=0
    fi
  done
  if ((valid == 0)); then
    err "COVERAGE_MINIMUM of tools/project.sh is three percentages, of the lines, branches and"
    err "conditions, as (100 100 100); it is (${COVERAGE_MINIMUM[*]})."
    exit 1
  fi
}

main() {
  cd "${ROOT_DIR}"
  check_minimum
  check_exclusions
  # Counts from an earlier run would add to this one.
  find "${COVERAGE_DIR}" -name '*.gcda' -delete 2>/dev/null || true
  cmake --workflow --preset coverage
  remove_stale_notes
  # The project's suites beyond CMake's, as a backend's tests built by a script of its own.
  local listed
  listed="$(coverage_suites)"
  local suites=()
  if [[ -n "${listed}" ]]; then
    mapfile -t suites <<< "${listed}"
  fi

  # No counts is a build that was not instrumented, not a library without code: a build
  # directory configured with another compiler is made anew, and CMake drops the preset's
  # options with its old cache, --coverage among them. gcovr would report 0 of 0 lines and pass.
  if ! find "${COVERAGE_DIR}" -name '*.gcda' -print -quit | grep -q .; then
    err "No coverage counts in ${COVERAGE_DIR}: the build was not instrumented."
    err "Remove ${COVERAGE_DIR} and run this again."
    exit 1
  fi

  mkdir -p "${COVERAGE_DIR}/html"
  # The counts are read from gcov once, into a tracefile, and each report is made from it: asked
  # for several metrics, gcovr writes one table without the lines missed, so each metric gets a
  # table of its own, one after the other in coverage.txt.
  # --exclude-throw-branches: on the host a call that may throw has a branch
  # to its exception, which no test takes; a sketch is built without
  # exceptions, so on the AVR there is no such branch to reach.
  local tracefile="${COVERAGE_DIR}/coverage.json"
  uv run --frozen gcovr --root "${ROOT_DIR}" --filter 'src/' \
    --exclude-throw-branches \
    --json "${tracefile}" "${COVERAGE_DIR}" "${suites[@]}"
  local metric
  : > "${COVERAGE_DIR}/coverage.txt"
  for metric in line branch condition; do
    report "${tracefile}" --txt-metric "${metric}" --txt "${COVERAGE_DIR}/${metric}.txt"
    cat "${COVERAGE_DIR}/${metric}.txt" >> "${COVERAGE_DIR}/coverage.txt"
  done
  # --fail-under-*: gcovr checks the merged counts, and exits with 2, 4 and 8 for the lines,
  # branches and conditions, or-ed, after it has written the reports.
  local status=0
  report "${tracefile}" \
    --html-details "${COVERAGE_DIR}/html/index.html" \
    --fail-under-line "${COVERAGE_MINIMUM[0]}" --fail-under-branch "${COVERAGE_MINIMUM[1]}" \
    --fail-under-condition "${COVERAGE_MINIMUM[2]}" \
    --print-summary || status=$?
  echo
  # The files with something missed, and the lines of what. gcovr names no line for a condition:
  # it counts one as reached once either of its outcomes is. The HTML report marks it.
  grep -vE ' (100|--)%' "${COVERAGE_DIR}/coverage.txt" || true
  if (( status & ~14 )); then
    exit "${status}"
  fi
  if (( status != 0 )); then
    if [[ "${COVERAGE_MINIMUM[*]}" == "100 100 100" ]]; then
      err "Not every line, branch and condition of src/ is reached by a test (Testing.md TS-4)."
    else
      err "The tests reach less of src/ than COVERAGE_MINIMUM of tools/project.sh, the lines," \
        "branches and conditions in percent: ${COVERAGE_MINIMUM[*]} (Testing.md TS-4)."
    fi
    err "The conditions missed are marked in ${COVERAGE_DIR}/html/index.html."
    exit 1
  fi
}

main "$@"
