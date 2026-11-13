#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Runs clang-tidy on the library and its examples, or on a firmware and its platform/, with the
# flags each target compiles them with, once for each of TIDY_VARIANTS in tools/project.sh, and on
# the tests on the host; the checks are in .clang-tidy. The database of a target and variant is
# build/tidy/targets/<target>/<variant>, written by target_database() of tools/common.sh:
# arduino-cli's rewritten as below, a CMake build's by tools/lib/clang_database.py for clang.
# build/tidy/clangd points to that of the first target and variant, which clangd reads for
# examples/ and platform/, see .clangd.
#
# With --database-only it writes the database of the first variant of each target, lists in
# build/tidy/databases the folders of those that name the target's own compiler, one per line,
# for tools/self_contained_headers.py, and stops. clang-tidy runs from the environment uv.lock
# pins, like the tools of lint.sh.
#
# arduino-cli compiles copies of a sketch's files in its build folder, and a finding there would
# point to the copy. So the flags are taken from its compilation database, and a database of our
# own names the files in the repository; an .ino is compiled as C++ with Arduino.h included
# first, which is what arduino-cli adds to it.
#
# The library is read with each target's flags, not only through the host's database: an int has
# 16 bits on the AVR and 32 on the host, and a check that only saw the host would never meet the
# code as the board does.
#
# A firmware of several targets has a folder of platform/ for each, and a target compiles its own
# and leaves the others' out. So a source of src/ or examples/ is read with every target, and one
# of platform/ with those whose database has it; one of platform/ that no target compiles fails,
# as it would otherwise never be read. The code of HOST_CODE is read with the tests, and that of
# TIDY_SKIP by a target of the project's own, with a database of its own.
#
# The checks of our own, house-*, are a module of clang-tidy's that tools/lib/tidy_module.sh builds
# from tools/lib/tidy_module/ and clang-tidy loads; the database alone does not need it.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly RUN_CLANG_TIDY=(uv run --frozen run-clang-tidy.py)
readonly TIDY_DIR="${BUILD_DIR}/tidy"

# Runs clang-tidy on the files given, with the entries the database in <dir> has for them, one
# file per core: clang-tidy alone reads one file after the other. run-clang-tidy prints the
# findings of a file together, and reads every file of the database it is given, so it gets a
# database of these files alone, written beside the others; a file the database lacks fails
# here, as clang-tidy fails on it, rather than going unread. One entry per file: CMake compiles a
# source of the library into each example, and the file would be read once for each.
# Arguments:
#   dir, the header filter (empty for that of .clang-tidy), then the files
run_tidy() {
  local dir="$1"
  local header_filter="$2"
  shift 2
  local selected="${TIDY_DIR}/selected/${dir#"${BUILD_DIR}/"}"
  mkdir -p "${selected}"
  jq --arg root "${ROOT_DIR}" '
    def path: if (.file | startswith("/")) then .file else .directory + "/" + .file end;
    ($ARGS.positional | map($root + "/" + .)) as $wanted
    | [.[] | select(path as $path | any($wanted[]; . == $path))] | unique_by(path)
    | . as $entries
    | ($wanted - ($entries | map(path))) as $missing
    | if ($missing | length) > 0
      then error("in no entry of the database: " + ($missing | join(", ")))
      else $entries end' \
    "${dir}/compile_commands.json" --args "$@" > "${selected}/compile_commands.json" ||
    return 1
  local args=(-p "${selected}" -j "$(nproc)" -quiet -hide-progress)
  if [[ -n "${MODULE}" ]]; then
    args+=("-load=${MODULE}")
  fi
  if [[ -n "${header_filter}" ]]; then
    args+=("-header-filter=${header_filter}")
  fi
  "${RUN_CLANG_TIDY[@]}" "${args[@]}"
}

# Prints those of the files given that the database in <dir> has an entry for, one per line.
# Arguments:
#   dir, then the files, relative to the repository
in_database() {
  local dir="$1"
  shift
  jq -r --arg root "${ROOT_DIR}" '
    def path: if (.file | startswith("/")) then .file else .directory + "/" + .file end;
    map(path) as $paths
    | $ARGS.positional[] | select(($root + "/" + .) as $wanted | any($paths[]; . == $wanted))' \
    "${dir}/compile_commands.json" --args "$@"
}

main() {
  local database_only=0
  if [[ "${1:-}" == "--database-only" ]]; then
    database_only=1
  elif [[ $# -gt 0 ]]; then
    err "Usage: $0 [--database-only]"
    exit 2
  fi

  cd "${ROOT_DIR}"
  mkdir -p "${TIDY_DIR}"
  local sources
  list_files sources '\.(cpp|ino)$' 'src/*' 'examples/*'
  local host_code
  host_code_dirs host_code
  local platform_paths=('platform/*')
  local dir
  for dir in "${host_code[@]}" "${TIDY_SKIP[@]}"; do
    platform_paths+=(":!${dir%/}/*")
  done
  local platform_sources
  list_files platform_sources '\.cpp$' "${platform_paths[@]}"
  local -A compiled=()
  MODULE=''
  if ((!database_only)); then
    MODULE="$(tools/lib/tidy_module.sh)"
  fi

  # Each variant takes other branches of #if, and each target other ones again. The first of both
  # is the one clangd reads, see .clangd.
  local failed=0
  local target
  local variant
  local compiler_database
  local label
  local found
  local own
  local source
  local databases=()
  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    for variant in "${TIDY_VARIANTS[@]}"; do
      dir="${TIDY_DIR}/targets/${target}/${variant}"
      compiler_database="$(target_database "${variant}" "${dir}")"
      if ((database_only)); then
        databases+=("${compiler_database}")
        break
      fi
      label="$(target_label "${variant}")"
      own=()
      if ((${#platform_sources[@]} > 0)); then
        found="$(in_database "${dir}" "${platform_sources[@]}")"
        if [[ -n "${found}" ]]; then
          mapfile -t own <<< "${found}"
        fi
      fi
      for source in "${own[@]}"; do
        compiled["${source}"]=1
      done
      echo "clang-tidy, ${label}:"
      run_tidy "${dir}" '' "${sources[@]}" "${own[@]}" || failed=1
    done
  done
  if ((!database_only)); then
    for source in "${platform_sources[@]}"; do
      if [[ -z "${compiled[${source}]:-}" ]]; then
        err "${source} is compiled by no target of tools/project.sh, so clang-tidy never reads it."
        failed=1
      fi
    done
  fi
  ln -sfn "targets/${TARGETS[0]}/${TIDY_VARIANTS[0]}" "${TIDY_DIR}/clangd"
  if ((database_only)); then
    printf '%s\n' "${databases[@]}" > "${TIDY_DIR}/databases"
    printf 'Wrote %s/compile_commands.json\n' "${databases[@]}"
    exit 0
  fi

  # The tests on the host, against the mock, with the database CMake writes. Only what lies in
  # test/: the tests are C++17, and the headers of the library, checked above as the gnu++11
  # they are, would be held to C++17. Not the mock in test/mock/, which stands in for the
  # Arduino core and follows it, not the Google style. Nor what lies in test/compile_fail/,
  # which must not compile, and in test/compile_warn/, which warns on purpose: neither is in a
  # database. And the code of HOST_CODE, which CMake builds beside the tests, and its headers.
  local paths=('test/*' ':!test/mock/*' ':!test/compile_fail/*' ':!test/compile_warn/*'
    ':!test/fuzz/*')
  local filter='test'
  for dir in "${host_code[@]}"; do
    paths+=("${dir}/*")
    filter+="|${dir}"
  done
  for dir in "${TIDY_SKIP[@]}"; do
    paths+=(":!${dir%/}/*")
  done
  local tests
  list_files tests '\.cpp$' "${paths[@]}"
  cmake --preset default >/dev/null
  echo "clang-tidy, the tests on the host:"
  run_tidy "${BUILD_DIR}/host" "(^|/)(${filter})/" "${tests[@]}" || failed=1

  # The fuzzers, with the database of the preset fuzz, which alone builds them; configured, not
  # built, as clang-tidy compiles each file itself.
  local fuzzers=()
  # Asked only where the folder is: git warns of one it cannot open, as a project has none
  # until its first fuzzer.
  local found=''
  if [[ -d test/fuzz ]]; then
    found="$(git ls-files --cached --others --exclude-standard -- 'test/fuzz/*Fuzz.cpp' \
      'test/fuzz/*_fuzz.cpp')"
  fi
  if [[ -n "${found}" ]]; then
    mapfile -t fuzzers <<< "${found}"
    cmake --preset fuzz >/dev/null
    echo "clang-tidy, the fuzzers:"
    run_tidy "${BUILD_DIR}/fuzz" '(^|/)test/' "${fuzzers[@]}" || failed=1
  fi
  exit "${failed}"
}

main "$@"
