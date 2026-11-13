#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Builds the reference from the Doxygen blocks into build/docs/html and fails on a warning: a
# block Doxygen cannot read, a \param that names no parameter, an entity of the API without a
# block. Two warnings are let pass and counted. "Is not documented" in NAMESPACE::internal of
# tools/project.sh, which a sketch does not use; but not in a file DOCS_INTERNAL_API names, which
# is API after all. And a missing \return: a one-line block whose \brief says what comes back,
# "Returns the number of elements.", needs none, while every parameter needs its \param.
# Doxygen cannot be told to fail on some warnings and not others, so they go into a log and
# are sorted here.
#
# A firmware whose backends each define a class of the same name, a Pixels of every board, runs
# Doxygen once per backend, over the input folders docs_inputs() of tools/project.sh prints, one
# line a run, each into build/docs/<n>/; one run over all of them would merge the classes into one.
# A warning of what the runs share, src/, is counted once. Doxygen comes from apt, in the Dockerfile and in CI alike, as it is not on
# PyPI.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly DOCS_DIR="${BUILD_DIR}/docs"
readonly LOG="${DOCS_DIR}/warnings.log"

# Prints the warnings that fail, one per line, with paths from the root.
failing_warnings() {
  # grep ends with 1 on a log without a line, which pipefail would take for
  # a failure.
  sed "s#^${ROOT_DIR}/##" "${LOG}" \
    | { grep -v '^$' || true; } \
    | INTERNAL="${NAMESPACE}::internal" API="${DOCS_INTERNAL_API}" awk '
        # From the environment, as -v would read the backslashes of the regex as escapes.
        BEGIN { internal = ENVIRON["INTERNAL"]; api = ENVIRON["API"] }
        internal != "::internal" && /is not documented\.$/ \
            && index($0, internal) && !(api != "" && $0 ~ api) { next }
        /warning: return type of member .* is not documented$/ { next }
        { print }'
}

# Runs Doxygen over the folders given, or over the Doxyfile's INPUT where none are, into the
# folder given first, and adds its warnings to LOG.
# Arguments:
#   the output folder, then the input folders
run_doxygen() {
  local out="$1"
  shift
  mkdir -p "${out}"
  {
    cat Doxyfile
    echo "OUTPUT_DIRECTORY = ${out}"
    echo "WARN_LOGFILE = ${out}/doxygen.log"
    if (($# > 0)); then
      echo "INPUT = $*"
    fi
  } | doxygen -
  cat "${out}/doxygen.log" >> "${LOG}"
}

# Prints LOG with each warning once: a warning starts with its path and line, and the lines after
# it that do not are part of it.
unique_warnings() {
  awk '
    /^[^ \t:][^:]*:[0-9]+: / { if (record != "" && !seen[record]++) print record; record = $0; next }
    { record = (record == "" ? $0 : record "\n" $0) }
    END { if (record != "" && !seen[record]++) print record }' "${LOG}"
}

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi
  if ! command -v doxygen >/dev/null; then
    err "doxygen is missing; it comes from apt, see the Dockerfile."
    exit 1
  fi

  cd "${ROOT_DIR}"
  mkdir -p "${DOCS_DIR}"
  : > "${LOG}"
  local listed
  listed="$(docs_inputs)"
  local where="${DOCS_DIR}/html"
  if [[ -z "${listed}" ]]; then
    run_doxygen "${DOCS_DIR}"
  else
    local index=0
    local line
    local inputs
    while IFS= read -r line; do
      index=$((index + 1))
      read -r -a inputs <<< "${line}"
      run_doxygen "${DOCS_DIR}/${index}" "${inputs[@]}"
    done <<< "${listed}"
    where="${DOCS_DIR}/<n>/html, one for each line of docs_inputs()"
  fi
  local unique
  unique="$(unique_warnings)"
  printf '%s\n' "${unique}" > "${LOG}"

  local all
  all="$(grep -c 'warning:' "${LOG}" || true)"
  local failing
  failing="$(failing_warnings)"
  if [[ -n "${failing}" ]]; then
    echo "${failing}"
  fi
  # Warnings, not lines: a warning may go on over lines of its own, as the parameters it names.
  local count=0
  if [[ -n "${failing}" ]]; then
    count="$(grep -c ': warning: ' <<< "${failing}" || true)"
  fi
  local passing="a missing return"
  if [[ -n "${NAMESPACE}" ]]; then
    passing="in ${NAMESPACE}::internal, or ${passing}"
  fi
  echo "docs.sh: ${count} warnings that fail, $((all - count)) gaps let pass: ${passing};" \
    "the reference is in ${where}."
  if ((count > 0)); then
    exit 1
  fi
}

main "$@"
