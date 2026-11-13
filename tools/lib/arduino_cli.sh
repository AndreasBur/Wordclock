#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The build system arduino-cli: how a target with BUILD_SYSTEM=arduino-cli is set up, how an
# example is compiled for it, and the flags clang-tidy reads it with. tools/common.sh sources this
# file and calls the functions arduino_cli_*, which target_setup() and the others there describe;
# every function works on the target select_target() selected, through its FQBN, ARDUINO_CORE,
# ARDUINO_BOARD_URLS and DATABASE_EXAMPLE. This file comes from the template.

# Prints the name of every example, one per line.
list_examples() {
  local dir
  for dir in "${ROOT_DIR}"/examples/*/; do
    basename "${dir}"
  done
}

# Installs the core of the selected target, pinned, from the index of ARDUINO_BOARD_URLS as well
# if it names one.
arduino_cli_setup() {
  local urls=()
  if [[ -n "${ARDUINO_BOARD_URLS}" ]]; then
    urls=(--additional-urls "${ARDUINO_BOARD_URLS}")
  fi
  arduino-cli core update-index "${urls[@]}"
  arduino-cli core install "${ARDUINO_CORE}" "${urls[@]}"
}

# Prints the bin directory of the avr-gcc that arduino-cli uses for FQBN.
avr_gcc_bin_dir() {
  local properties
  properties="$(arduino-cli compile --fqbn "${FQBN}" --show-properties \
    "${ROOT_DIR}/examples/${DATABASE_EXAMPLE}" 2>/dev/null)"
  sed -n 's/^compiler\.path=//p' <<< "${properties}"
}

# Compiles an example in a variant into <target build dir>/<variant>/<example> and prints what
# arduino-cli prints. The flags of the variant go into compiler.cpp.extra_flags, which reaches
# every C++ file of the build, the core's as well. Its callers test its status, which turns off
# set -e within it, so it returns on a failure itself.
# Arguments:
#   variant: one of VARIANTS
#   example: the name of a folder in examples/
compile_example() {
  local variant="$1"
  local example="$2"
  local args=(
    --fqbn "${FQBN}"
    --library "${ROOT_DIR}"
    --warnings all
    --build-path "$(target_build_dir)/${variant}/${example}"
  )
  local flags
  flags="$(variant_flags "${variant}")" || return
  flags="${flags//$'\n'/ }"
  if [[ -n "${flags}" ]]; then
    args+=(--build-property "compiler.cpp.extra_flags=${flags}")
  fi
  arduino-cli compile "${args[@]}" "${ROOT_DIR}/examples/${example}" 2>&1
}

# Prints the path of the file arduino_cli_build() keeps what arduino-cli printed for an example
# in, which arduino_cli_sizes() reads.
# Arguments:
#   variant, example
arduino_cli_log() {
  local variant="$1"
  local example="$2"
  local dir
  dir="$(target_build_dir)"
  echo "${dir}/${variant}/${example}.log"
}

# Compiles every example in a variant, or the one named, and fails where one does not compile
# or warns outside the Arduino core.
# Arguments:
#   variant: one of VARIANTS
#   example: optional, the name of a folder in examples/
arduino_cli_build() {
  local variant="$1"
  local examples=("${2:-}")
  if [[ -z "${examples[0]}" ]]; then
    local listed
    listed="$(list_examples)"
    mapfile -t examples <<< "${listed}"
  fi
  local label
  label="$(target_label "${variant}")"
  local failed=0
  local example
  local log
  local output
  local warnings
  for example in "${examples[@]}"; do
    log="$(arduino_cli_log "${variant}" "${example}")"
    mkdir -p "${log%/*}"
    rm -f "${log}"
    if ! output="$(compile_example "${variant}" "${example}")"; then
      err "${example} (${label}) does not compile:"
      err "${output}"
      failed=1
      continue
    fi
    echo "${output}" > "${log}"
    warnings="$(grep 'warning:' <<< "${output}" | grep -v '/cores/arduino/' || true)"
    if [[ -n "${warnings}" ]]; then
      err "${example} (${label}) warns:"
      err "${warnings}"
      failed=1
    fi
  done
  return "${failed}"
}

# Prints the name of every example, which arduino_cli_build() compiles each.
arduino_cli_programs() {
  list_examples
}

# Prints the path of the ELF file of an example that arduino_cli_build() made.
arduino_cli_elf() {
  elf_of "$@"
}

# Prints flash and RAM of an example as arduino-cli reported them, nothing where it did not
# compile.
# Arguments:
#   variant, example
arduino_cli_sizes() {
  local log
  log="$(arduino_cli_log "$1" "$2")"
  if [[ ! -f "${log}" ]]; then
    return 0
  fi
  local output
  output="$(< "${log}")"
  local flash
  local ram
  flash="$(sed -n 's/^Sketch uses* \([0-9]*\) bytes.*/\1/p' <<< "${output}")"
  ram="$(sed -n 's/^Global variables uses* \([0-9]*\) bytes.*/\1/p' <<< "${output}")"
  echo "${flash} ${ram}"
}

# Writes the database of a variant for the sources of src/ and examples/ with the flags of
# arduino_flags(), and prints its folder: the same database serves clang-tidy and the headers,
# as arduino_flags() already turned it into one that clang reads.
# Arguments:
#   variant: one of TIDY_VARIANTS
#   dir: the folder to write into
arduino_cli_database() {
  local variant="$1"
  local dir="$2"
  local sources
  list_files sources '\.(cpp|ino)$' 'src/*' 'examples/*'
  local flags
  flags="$(arduino_flags "${variant}" "${BUILD_DIR}/tidy/arduino/${TARGET}/${variant}")"
  write_database "${dir}" "${flags}" "${sources[@]}"
  echo "${dir}"
}

# Writes <dir>/compile_commands.json for the files given, with the flags of arduino_flags() of
# tools/lib/arduino_cli.sh.
# Arguments:
#   dir, flags, then the files
write_database() {
  local dir="$1"
  local flags="$2"
  shift 2
  mkdir -p "${dir}"
  jq -n --arg dir "${ROOT_DIR}" --argjson flags "${flags}" '
    $ARGS.positional | map({
      directory: $dir,
      file: .,
      arguments: ($flags
        + (if endswith(".ino") then ["-x", "c++", "-include", "Arduino.h"]
           else [] end)
        + [.])
    })' --args "$@" > "${dir}/compile_commands.json"
}

# Prints the path of the ELF file that compile_example made.
elf_of() {
  local variant="$1"
  local example="$2"
  local dir
  dir="$(target_build_dir)"
  echo "${dir}/${variant}/${example}/${example}.ino.elf"
}

# Prints, as a JSON array, the arguments arduino-cli compiles a file of the sketch
# DATABASE_EXAMPLE with in VARIANT, without the file, its output and -w, and changed for clang.
# Compiled with the defines of the variant, as arduino-cli finds the libraries a sketch needs by
# its includes: one that only a variant includes, as SPI under a switch, has its folder in the
# database of that variant alone. Only the defines: a plugin or an optimization of avr-gcc means
# nothing to clang.
# - The include folders outside the repository, the core's and the board's, become -isystem,
#   so that nothing is reported from headers we do not own.
# - -Wall -Wextra, so that clang's own warnings are reported as well.
# - -Wno-unknown-attributes: clang does not know avr-gcc's attribute __progmem__, which
#   PROGMEM stands for, and takes the tables as in RAM. That changes nothing clang-tidy checks.
# Arguments:
#   variant: one of TIDY_VARIANTS
#   dir: a folder for arduino-cli's own build and database
arduino_flags() {
  local variant="$1"
  local dir="$2"
  local flags
  flags="$(variant_flags "${variant}")"
  local flag
  local defines=()
  while IFS= read -r flag; do
    if [[ "${flag}" == -D* ]]; then
      defines+=("${flag}")
    fi
  done <<< "${flags}"
  local args=(
    --fqbn "${FQBN}"
    --library "${ROOT_DIR}"
    --only-compilation-database
    --build-path "${dir}"
  )
  if ((${#defines[@]} > 0)); then
    args+=(--build-property "compiler.cpp.extra_flags=${defines[*]}")
  fi
  arduino-cli compile "${args[@]}" "${ROOT_DIR}/examples/${DATABASE_EXAMPLE}" >/dev/null
  jq --arg root "${ROOT_DIR}" '[.[] | select(.file | endswith(".ino.cpp"))][0]
    | .file as $file
    | .arguments
    | del(.[index("-o"), index("-o") + 1])
    | map(select(. != $file and . != "-w"))
    | map(if startswith("-I") and (startswith("-I" + $root) | not)
          then "-isystem" + .[2:] else . end)
    + ["-Wall", "-Wextra", "-Wno-unknown-attributes"]' \
    "${dir}/compile_commands.json"
}
