#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The build system platformio: a target with BUILD_SYSTEM=platformio is the environment PIO_ENV of
# the platformio.ini in PIO_DIR of tools/project.sh, built by the PlatformIO uv.lock pins. Its
# platform, board and framework, the Arduino core or none, are the platformio.ini's to name, and
# pinned there. tools/common.sh sources this file and calls the functions platformio_*, which
# target_setup() and the others there describe. This file comes from the template.
#
# What the tools need to differ from a plain `pio run` they give it in PlatformIO's own variables,
# so that platformio.ini stays as a developer builds with it:
#
#   PLATFORMIO_BUILD_DIR        <target build dir>/<variant>, one build per variant
#   PLATFORMIO_BUILD_FLAGS      the variant's flags, as variant_flags() prints them, on every file
#   PLATFORMIO_BUILD_SRC_FLAGS  -Werror, on the project's own sources only: a warning of a
#                               framework, as of the Arduino core, is not the project's to fix

readonly PIO=(uv run --frozen pio)

# Fails where the selected target names no platformio.ini, and installs its platform and
# packages where it does.
platformio_setup() {
  if [[ -z "${PIO_DIR}" || -z "${PIO_ENV}" ]]; then
    err "Target ${TARGET}: BUILD_SYSTEM=platformio needs PIO_DIR and PIO_ENV in tools/project.sh."
    return 1
  fi
  if [[ ! -f "${ROOT_DIR}/${PIO_DIR}/platformio.ini" ]]; then
    err "Target ${TARGET}: PIO_DIR=${PIO_DIR} holds no platformio.ini."
    return 1
  fi
  "${PIO[@]}" pkg install --silent -d "${ROOT_DIR}/${PIO_DIR}" -e "${PIO_ENV}"
}

# Prints the build folder of a variant.
platformio_dir() {
  local dir
  dir="$(target_build_dir)"
  echo "${dir}/$1"
}

# Runs pio with the arguments given for a variant, with the variables above set, and prints what
# it prints. Its callers test its status, which turns off set -e within it.
# Arguments:
#   variant, then what `pio run` is given beyond the folder and the environment
platformio_run() {
  local variant="$1"
  shift
  local flags
  flags="$(variant_flags "${variant}")" || return
  local dir
  dir="$(platformio_dir "${variant}")"
  PLATFORMIO_BUILD_DIR="${dir}" \
    PLATFORMIO_BUILD_FLAGS="${flags//$'\n'/ }" \
    PLATFORMIO_BUILD_SRC_FLAGS="-Werror" \
    "${PIO[@]}" run -d "${ROOT_DIR}/${PIO_DIR}" -e "${PIO_ENV}" "$@" 2>&1
}

# Builds the image of a variant, failing on an error and on a warning in the project's own
# sources, and keeps what pio printed beside it, which platformio_sizes() reads.
# Arguments:
#   variant: one of VARIANTS
#   program: optional; there is one, the image
platformio_build() {
  local variant="$1"
  local label
  label="$(target_label "${variant}")"
  local dir
  dir="$(platformio_dir "${variant}")"
  mkdir -p "${dir}"
  rm -f "${dir}/build.log"
  local output
  if ! output="$(platformio_run "${variant}")"; then
    err "${label}: the build failed:"
    err "${output}"
    return 1
  fi
  echo "${output}" > "${dir}/build.log"
}

# Prints the name of the image: the project's, as the folder's, which cpplint requires it to be.
platformio_programs() {
  basename "${ROOT_DIR}"
}

# Prints the path of the image's ELF file.
# Arguments:
#   variant, program
platformio_elf() {
  local dir
  dir="$(platformio_dir "$1")"
  echo "${dir}/${PIO_ENV}/firmware.elf"
}

# Prints flash and RAM of the image as pio reported them, nothing where it was not built.
# Arguments:
#   variant, program
platformio_sizes() {
  local dir
  dir="$(platformio_dir "$1")"
  if [[ ! -f "${dir}/build.log" ]]; then
    return 0
  fi
  local output
  output="$(< "${dir}/build.log")"
  local flash
  local ram
  flash="$(sed -n 's/^Flash:.*(used \([0-9]*\) bytes.*/\1/p' <<< "${output}")"
  ram="$(sed -n 's/^RAM:.*(used \([0-9]*\) bytes.*/\1/p' <<< "${output}")"
  echo "${flash} ${ram}"
}

# Writes the database of a variant, as `pio run -t compiledb` writes it, rewritten for clang into
# <dir> with tools/lib/clang_database.py, and prints the folder of the one as pio wrote it, which
# names the target's own compiler. pio writes it into PIO_DIR, whatever else it is told, so it is
# moved out of the sources at once.
# Arguments:
#   variant: one of TIDY_VARIANTS
#   dir: the folder to write into
platformio_database() {
  local variant="$1"
  local dir="$2"
  local output
  if ! output="$(platformio_run "${variant}" -t compiledb)"; then
    err "${output}"
    return 1
  fi
  local compiler_dir
  compiler_dir="$(platformio_dir "${variant}")/compiledb"
  mkdir -p "${compiler_dir}"
  mv "${ROOT_DIR}/${PIO_DIR}/compile_commands.json" "${compiler_dir}/compile_commands.json"
  python3 "${ROOT_DIR}/tools/lib/clang_database.py" "${compiler_dir}" "${dir}" || return
  echo "${compiler_dir}"
}
