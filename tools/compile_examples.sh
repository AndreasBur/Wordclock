#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Compiles every example of a library, or the image of a firmware, for each target in each variant
# and prints flash and RAM of each, as a Markdown table: a column per variant, named
# target/variant where there is more than one target.
# Fails where an example does not compile or warns outside the toolchain's own files, where a
# build that passed left no ELF file of one, and where check_elf() of tools/project.sh fails on
# what was built. When every example compiles, the table goes to build/sizes.md as well, which
# tools/documented_sizes.py compares with the README's (`make sizes`); else that file is removed,
# so that no table of an older build is taken for this one's.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi

  local failed=0
  local target
  local program
  local variant
  local label
  local measured
  local built
  local elf
  local listed
  local programs
  local -A flash_size
  local -A ram_size
  local labels=()
  # Each program once, in the order the first target and variant named it.
  local rows=()
  local -r sizes="${BUILD_DIR}/sizes.md"
  rm -f "${sizes}"

  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    for variant in "${VARIANTS[@]}"; do
      label="$(target_label "${variant}")"
      labels+=("${label}")
      built=1
      if ! target_build "${variant}"; then
        built=0
        failed=1
      fi
      if ! listed="$(target_programs "${variant}")"; then
        failed=1
        continue
      fi
      mapfile -t programs <<< "${listed}"
      for program in "${programs[@]}"; do
        if [[ " ${rows[*]} " != *" ${program} "* ]]; then
          rows+=("${program}")
        fi
        elf="$(target_elf "${variant}" "${program}")"
        if [[ ! -f "${elf}" ]]; then
          if ((built)); then
            err "${program} (${label}): built, but there is no ${elf}."
            failed=1
          fi
          continue
        fi
        check_elf "${variant}" "${elf}" || failed=1
        measured="$(target_sizes "${variant}" "${program}")"
        flash_size["${program}/${label}"]="${measured% *}"
        ram_size["${program}/${label}"]="${measured#* }"
      done
    done
  done

  # Example, as a library's table has always been headed, where there are examples; else what
  # is built is a firmware's image.
  local header="| Example |"
  if [[ ! -d "${ROOT_DIR}/examples" ]]; then
    header="| Image |"
  fi
  local rule="|---|"
  for label in "${labels[@]}"; do
    header+=" Flash, ${label} |"
    rule+="---|"
  done
  for label in "${labels[@]}"; do
    header+=" RAM, ${label} |"
    rule+="---|"
  done
  local table="${header}"$'\n'"${rule}"
  local row
  for program in "${rows[@]}"; do
    row="| ${program} |"
    for label in "${labels[@]}"; do
      row+=" ${flash_size["${program}/${label}"]:-} |"
    done
    for label in "${labels[@]}"; do
      row+=" ${ram_size["${program}/${label}"]:-} |"
    done
    table+=$'\n'"${row}"
  done
  echo "${table}"
  if ((failed == 0)); then
    mkdir -p "${BUILD_DIR}"
    echo "${table}" > "${sizes}"
  fi
  exit "${failed}"
}

main "$@"
