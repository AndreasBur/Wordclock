#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Installs what each target of tools/project.sh needs, as the pinned Arduino core of one built by
# arduino-cli, and builds what prepare() there builds beyond them. The dev container runs it when
# it is created, CI before it compiles an example; run it again after ARDUINO_CORE has changed.
# With --systems it prints the build systems the targets use, one per line, and installs nothing.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  # The build systems of the targets, one per line, so that CI installs arduino-cli only where a
  # target needs it.
  if [[ "${1:-}" == "--systems" ]]; then
    local target
    local systems=()
    for target in "${TARGETS[@]}"; do
      select_target "${target}"
      if [[ " ${systems[*]} " != *" ${BUILD_SYSTEM} "* ]]; then
        systems+=("${BUILD_SYSTEM}")
      fi
    done
    printf '%s\n' "${systems[@]}"
    exit 0
  fi
  if [[ $# -gt 0 ]]; then
    err "Usage: $0 [--systems]"
    exit 2
  fi

  local target
  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    target_setup
  done
  prepare
}

main "$@"
