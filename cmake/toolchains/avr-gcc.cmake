# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# avr-gcc for an AVR without the Arduino core: the toolchain of a target with
# BUILD_SYSTEM=cmake in tools/project.sh, which names it in CMAKE_TOOLCHAIN and gives the part and
# its clock in CMAKE_DEFINES:
#
#   cmake -S . -B build/avr -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/avr-gcc.cmake \
#         -DAVR_MCU=atmega328p -DAVR_F_CPU=16000000
#
# Given when the build directory is configured, not by CMakeLists.txt: the compiler cannot be
# changed after project() has run. avr-gcc and avr-libc come from apt (gcc-avr, avr-libc), which
# .devcontainer/apt-packages.txt names for such a project. This file comes from the template.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(AVR_MCU "" CACHE STRING "The part, as avr-gcc's -mmcu names it")
set(AVR_F_CPU "" CACHE STRING "Its clock in Hz, F_CPU to the code")
if(NOT AVR_MCU OR NOT AVR_F_CPU)
  message(FATAL_ERROR "avr-gcc.cmake needs -DAVR_MCU=<part> and -DAVR_F_CPU=<Hz>.")
endif()
# CMake reads this file again for each try_compile, in a build of its own that knows only the
# variables named here.
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES AVR_MCU AVR_F_CPU)

find_program(CMAKE_C_COMPILER avr-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER avr-g++ REQUIRED)
find_program(CMAKE_OBJCOPY avr-objcopy REQUIRED)
find_program(CMAKE_SIZE avr-size REQUIRED)

# CMake proves a compiler works by linking an executable. For a part without an operating system
# that needs a main() and the startup files of the part, neither of which exists yet; a static
# library proves the same.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# The part and its clock reach every file, and each function and object its own section, so that
# the linker drops what nothing calls: a library's function that no example uses costs no flash.
# -fstack-usage -fcallgraph-info=su: each function's frame and calls, beside each object, which
# tools/avr_stack.py adds up; the code is byte for byte the same without them.
set(CMAKE_C_FLAGS_INIT "-mmcu=${AVR_MCU} -DF_CPU=${AVR_F_CPU}UL -ffunction-sections -fdata-sections \
-fstack-usage -fcallgraph-info=su")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mmcu=${AVR_MCU} -Wl,--gc-sections")

# Nothing of the build host is a candidate for a part without an operating system.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
