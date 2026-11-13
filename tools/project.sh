#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# What the scripts in tools/ need to know of this project. tools/common.sh sources it after
# its own defaults, so what is defined here wins. The template wrote this file once; it is the
# project's, and `copier update` leaves it alone. What it may define:
#
#   NAMESPACE            the library's namespace; tools/docs.sh lets gaps in
#                        NAMESPACE::internal pass
#   TARGETS              the targets the project is built for, each a name of lower case,
#                        digits and -, which names its folders of build/; one, board, if not
#                        set. Each takes the settings below, from BUILD_SYSTEM to
#                        SELF_CHECK_EXAMPLE, as they are set here, and target_settings()
#                        changes them for a target
#   BUILD_SYSTEM         what builds a target: arduino-cli, or cmake with a toolchain file
#   CMAKE_TOOLCHAIN      for cmake: the toolchain file, as cmake/toolchains/avr-gcc.cmake
#   CMAKE_DEFINES        for cmake: what the toolchain file is told, as the part and its clock,
#                        an array of -D options
#   PIO_DIR              for platformio: the folder of the platformio.ini, as platform/avr
#   PIO_ENV              for platformio: its environment the target is
#   FQBN                 for arduino-cli: the board the examples are compiled for
#   ARDUINO_CORE         for arduino-cli: the core, pinned, as arduino-cli installs it
#   ARDUINO_BOARD_URLS   for arduino-cli: the index of a third-party core, for update-index
#   SIMAVR_MCU           what simavr runs examples/SelfCheck as; empty: no run in simavr
#   SIMAVR_FREQUENCY     its clock, in Hz
#   DATABASE_EXAMPLE     for arduino-cli: the example whose flags tools/tidy.sh takes for all
#   SELF_CHECK_EXAMPLE   the example tools/self_check.sh runs
#   VARIANTS             the builds every example is compiled in, see variant_flags()
#   TIDY_VARIANTS        the variants clang-tidy and cppcheck read: those whose defines change
#                        the code. clangd reads the database of the first, on the first target
#   DOCS_INTERNAL_API    a regex of the files in NAMESPACE::internal that are API after all,
#                        such as a base a public class takes members from
#   CPPCHECK             0 if cppcheck cannot parse the library, with the reason; else 1
#   COVERAGE_MINIMUM     the percentages of lines, branches and conditions of src/ the tests
#                        have to reach, in that order; 100 100 100 if not set. Lower only for
#                        a project brought in below them, at what it reaches, and raised as
#                        it reaches more (Testing.md TS-4)
#   HOST_CODE            folders of code that runs on the host beside the library, as a
#                        simulator, built by CMakeLists.txt: tools/lint.sh formats and checks
#                        them as src/, tools/tidy.sh reads them with the tests
#
#   target_settings TARGET  sets what differs for a target, as its FQBN; TARGET is set as well
#   variant_flags VARIANT   prints the compiler flags of a variant, one per line
#   prepare                 builds what compiling needs beyond the core, such as a plugin
#   check_elf VARIANT ELF   fails if a compiled example is wrong in a way no warning says

# shellcheck disable=SC2034  # read by the scripts that source this file

# Three targets, each an image of src/ with the folder of platform/ that fits it. The AVR is
# built by CMake with platform/avr-dx's toolchain file; the ESP32-S3 and the Pico 2 W by
# PlatformIO, as platform/<name>/platformio.ini names their framework, board and packages.
TARGETS=(avr128da48 esp32 rp2350)
target_settings() {
  case "$1" in
    avr128da48)
      BUILD_SYSTEM="cmake"
      CMAKE_TOOLCHAIN="platform/avr-dx/toolchain-avr.cmake"
      CMAKE_DEFINES=(-DPLATFORM=avr-dx)
      ;;
    esp32 | rp2350)
      BUILD_SYSTEM="platformio"
      PIO_DIR="platform/$1"
      PIO_ENV="wordclock"
      ;;
  esac
}

# The global namespace: Wordclock's classes are named as its files, Display in Display.h, and
# have no namespace to sort them (CodingStyle.md F-9).
NAMESPACE=""
# An image runs as long as the part has power; there is no SelfCheck that ends for simavr.
SIMAVR_MCU=""
# The simulator: the window, the web host and the stand-ins for the hardware that the tests of
# test/ are built against.
HOST_CODE=(platform/simulator)
VARIANTS=(default)
TIDY_VARIANTS=(default)
