# Cross-compilation toolchain for the AVR Dx backend.
#
# Passed at configure time, not from the platform's CMakeLists: the compiler cannot be
# changed after project() has run, so the choice has to be made before CMake looks at any
# of this repository's own files.
#
#   cmake -B build -S . -DPLATFORM=avr-dx -DCMAKE_TOOLCHAIN_FILE=platform/avr-dx/toolchain-avr.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

find_program(CMAKE_C_COMPILER avr-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER avr-g++ REQUIRED)
find_program(CMAKE_OBJCOPY avr-objcopy REQUIRED)
find_program(CMAKE_SIZE avr-size REQUIRED)

# CMake proves a compiler works by linking a complete executable. For a freestanding
# target that needs a main() and the device's startup files, neither of which exists at
# this point - so it is told to settle for a static library, which proves the same thing.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Nothing on the build host is a candidate for a target that has no operating system.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
