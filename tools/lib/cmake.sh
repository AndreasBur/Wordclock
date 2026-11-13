#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The build system cmake: a target with BUILD_SYSTEM=cmake is CMakeLists.txt configured with the
# toolchain file CMAKE_TOOLCHAIN and the definitions CMAKE_DEFINES of tools/project.sh, one build
# folder per variant, <target build dir>/<variant>. Cross-compiled, CMakeLists.txt builds what it
# builds for a target there as programs/<Name>.elf, the examples of a library or the image of a
# firmware, and names them in programs.txt. tools/common.sh sources this file and calls the
# functions cmake_*, which target_setup() and the others there describe. This file comes from the
# template.

# Fails where the selected target names no toolchain file, or one that is not there.
cmake_setup() {
  if [[ -z "${CMAKE_TOOLCHAIN}" ]]; then
    err "Target ${TARGET}: BUILD_SYSTEM=cmake, but no CMAKE_TOOLCHAIN in tools/project.sh."
    return 1
  fi
  if [[ ! -f "${ROOT_DIR}/${CMAKE_TOOLCHAIN}" ]]; then
    err "Target ${TARGET}: CMAKE_TOOLCHAIN=${CMAKE_TOOLCHAIN}, which is no file of the project."
    return 1
  fi
}

# Prints the build folder of a variant.
cmake_dir() {
  local variant="$1"
  local dir
  dir="$(target_build_dir)"
  echo "${dir}/${variant}"
}

# Configures the build folder of a variant, with the flags of variant_flags() in VARIANT_FLAGS,
# which CMakeLists.txt puts on every file, and warnings as errors, as an example of arduino-cli
# fails on a warning as well. MinSizeRel, as the flash of the part is what a build for it is for.
# Prints nothing but an error.
# Arguments:
#   variant: one of VARIANTS or TIDY_VARIANTS
cmake_configure() {
  local variant="$1"
  cmake_setup || return
  local flags
  flags="$(variant_flags "${variant}")" || return
  local dir
  dir="$(cmake_dir "${variant}")"
  local output
  if ! output="$(cmake -S "${ROOT_DIR}" -B "${dir}" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/${CMAKE_TOOLCHAIN}" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_COMPILE_WARNING_AS_ERROR=ON \
    -DVARIANT_FLAGS="${flags//$'\n'/ }" \
    "${CMAKE_DEFINES[@]}" 2>&1)"; then
    err "${output}"
    return 1
  fi
}

# Builds every example of a variant, or the one named; a warning is an error, see
# cmake_configure().
# Arguments:
#   variant: one of VARIANTS
#   example: optional, the name of a folder in examples/
cmake_build() {
  local variant="$1"
  local example="${2:-}"
  local label
  label="$(target_label "${variant}")"
  if ! cmake_configure "${variant}"; then
    err "${label}: CMake could not configure the build."
    return 1
  fi
  local dir
  dir="$(cmake_dir "${variant}")"
  local args=(--build "${dir}")
  if [[ -n "${example}" ]]; then
    args+=(--target "${example}")
  fi
  local output
  if ! output="$(cmake "${args[@]}" 2>&1)"; then
    err "${label}: the build failed:"
    err "${output}"
    return 1
  fi
}

# Prints what CMakeLists.txt builds in a variant, as programs.txt of its build folder names it.
# Arguments:
#   variant: one of VARIANTS
cmake_programs() {
  local dir
  dir="$(cmake_dir "$1")"
  if [[ ! -f "${dir}/programs.txt" ]]; then
    err "${dir}/programs.txt is missing: CMakeLists.txt names there what it builds for a target."
    return 1
  fi
  cat "${dir}/programs.txt"
}

# Prints the folder CMake compiles a program's objects into, where avr-gcc writes the call graph
# of each beside it, for tools/avr_stack.py.
# Arguments:
#   variant, program
cmake_graphs() {
  local dir
  dir="$(cmake_dir "$1")"
  echo "${dir}/CMakeFiles/$2.dir"
}

# Prints the cache entry of a variant's build folder named, empty where it has none.
# Arguments:
#   variant, name, as AVR_MCU
cmake_cache() {
  local dir
  dir="$(cmake_dir "$1")"
  sed -n "s/^$2:[A-Z]*=//p" "${dir}/CMakeCache.txt"
}

# Prints the path of the ELF file of a program.
# Arguments:
#   variant, program
cmake_elf() {
  local dir
  dir="$(cmake_dir "$1")"
  echo "${dir}/programs/$2.elf"
}

# Prints flash and RAM of an example, from its sections as the toolchain's size reads them:
# CMAKE_SIZE of the toolchain file, which CMake keeps in its cache. Nothing where it was not built.
# Arguments:
#   variant, example
cmake_sizes() {
  local elf
  elf="$(cmake_elf "$1" "$2")"
  if [[ ! -f "${elf}" ]]; then
    return 0
  fi
  local dir
  dir="$(cmake_dir "$1")"
  local size
  size="$(sed -n 's/^CMAKE_SIZE:FILEPATH=//p' "${dir}/CMakeCache.txt")"
  if [[ -z "${size}" ]]; then
    err "${CMAKE_TOOLCHAIN} sets no CMAKE_SIZE, which the sizes are read with."
    return 1
  fi
  local sections
  sections="$("${size}" -A "${elf}")"
  awk '$1 == ".text" || $1 == ".data" || $1 == ".bootloader" { flash += $2 }
       $1 == ".data" || $1 == ".bss" || $1 == ".noinit" { ram += $2 }
       END { print flash + 0, ram + 0 }' <<< "${sections}"
}

# Configures the build folder of a variant, writes its database rewritten for clang into <dir>
# with tools/lib/clang_database.py, and prints the build folder, whose database names the
# toolchain's own compiler. Configured, not built: clang-tidy compiles each file itself.
# Arguments:
#   variant: one of TIDY_VARIANTS
#   dir: the folder to write into
cmake_database() {
  local variant="$1"
  local dir="$2"
  cmake_configure "${variant}" || return
  local build
  build="$(cmake_dir "${variant}")"
  python3 "${ROOT_DIR}/tools/lib/clang_database.py" "${build}" "${dir}" || return
  echo "${build}"
}
