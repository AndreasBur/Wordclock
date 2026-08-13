cmake_minimum_required(VERSION 3.16)

# Top-level entry point. Selects which platform backend to build; the firmware
# core in firmware/ is shared and pulled in by the chosen platform.
project(Wordclock NONE)

# enable_testing() rather than include(CTest): the latter also pulls in the
# Nightly/Experimental/Continuous dashboard targets and wants a CTestConfig.cmake,
# none of which this project uses.
enable_testing()

# On by default because the tests share their objects with the application, so
# they cost one more translation unit and a link rather than a second build of
# the firmware core.
option(WORDCLOCK_BUILD_TESTS "Build the platform test executable" ON)

set(PLATFORM simulator CACHE STRING "Target platform: simulator | hardware")

if(PLATFORM STREQUAL "simulator")
    add_subdirectory(platform/simulator)
elseif(PLATFORM STREQUAL "hardware")
    message(FATAL_ERROR
        "The hardware platform is built with the AVR toolchain, not CMake. "
        "See platform/hardware/README.md.")
else()
    message(FATAL_ERROR "Unknown PLATFORM '${PLATFORM}' (expected: simulator | hardware)")
endif()
