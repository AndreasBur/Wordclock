#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks the library against the rules of the Arduino Library Manager, as arduino-lint does in
# CI. Not the working tree: the Library Manager sees the repository, and the working tree has
# build/ and .venv beside it, whose links arduino-lint reports as a library's. So what git has,
# tracked or new and not ignored, is copied into a folder of the library's name in build/, and
# that is checked. With --copy-only it makes the copy, prints its path and stops, for a workflow
# that checks it with arduino-lint's action.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  local copy_only=0
  if [[ "${1:-}" == "--copy-only" ]]; then
    copy_only=1
  elif [[ $# -gt 0 ]]; then
    err "Usage: $0 [--copy-only]"
    exit 2
  fi

  cd "${ROOT_DIR}"
  # A library of CMake alone is no Arduino library, and the Library Manager's rules are not its.
  if [[ ! -f library.properties ]]; then
    echo "arduino_lint.sh: no library.properties, no Arduino library to check."
    exit 0
  fi
  local name
  name="$(sed -n 's/^name=//p' library.properties)"
  if [[ -z "${name}" ]]; then
    err "library.properties names no library."
    exit 1
  fi
  local copy="${BUILD_DIR}/arduino-lint/${name}"
  rm -rf "${BUILD_DIR}/arduino-lint"
  mkdir -p "${copy}"
  # Without a file deleted and not yet staged, which git still lists and tar could not read.
  local file
  git ls-files -z --cached --others --exclude-standard \
    | while IFS= read -r -d '' file; do
      if [[ -e "${file}" ]]; then
        printf '%s\0' "${file}"
      fi
    done \
    | tar --null --files-from=- --create --file=- \
    | tar --extract --file=- --directory="${copy}"
  if ((copy_only)); then
    echo "${copy}"
    exit 0
  fi
  arduino-lint --library-manager submit --compliance strict "${copy}"
}

main "$@"
