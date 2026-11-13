cmake_minimum_required(VERSION 3.16)

project(Wordclock LANGUAGES CXX)

# The template's checks: the options the presets sanitize, coverage and fuzz turn on, the
# instrumentation they ask for in CHECK_INSTRUMENT, and add_fuzzers().
include(${CMAKE_SOURCE_DIR}/cmake/checks.cmake)

# --------------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# bin/ under the build directory, which is what the README and every note in this repository
# tell a reader to run: build/host/bin/Wordclock, in the default preset.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# --------------------------------------------------------------------------
# wxWidgets, found once here rather than per platform: the three hand-kept IDE projects this
# build replaced each carried their own `wx-config` or library path, and keeping three in step
# is what CMake is for.
# --------------------------------------------------------------------------
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
# Platform-agnostic firmware core, shared with the hardware target.
set(FIRMWARE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

# Firmware logic shared with the real hardware target...
file(GLOB_RECURSE ARDUINO_SOURCES CONFIGURE_DEPENDS
    "${FIRMWARE_DIR}/*.cpp")

# --------------------------------------------------------------------------
# The two pages, as an array in a generated header - the same step both PlatformIO builds run
# as a pre-action and both host-test harnesses run by hand. Here it is a build rule so that an
# edit to web/ is a rebuild rather than something to remember: the sources listed as its
# dependencies are exactly the assets embed_web.py reads.
# --------------------------------------------------------------------------
set(WEB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../web")
set(EMBED_WEB "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/embed_web.py")
set(WEB_PAGE_HEADER "${CMAKE_CURRENT_BINARY_DIR}/generated/WebPage.h")

file(GLOB WEB_ASSETS CONFIGURE_DEPENDS
    "${WEB_DIR}/*.html" "${WEB_DIR}/*.webmanifest" "${WEB_DIR}/*.png")

add_custom_command(
    OUTPUT "${WEB_PAGE_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
    COMMAND python3 "${EMBED_WEB}" "${WEB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/generated"
    DEPENDS ${WEB_ASSETS} "${EMBED_WEB}"
    COMMENT "Embedding web/ into WebPage.h"
    VERBATIM)

add_custom_target(WordclockWebPage DEPENDS "${WEB_PAGE_HEADER}")

# ...plus the wxWidgets stand-ins for the hardware. Only the two entry-point
# files below are specific to the app; everything here is shared with the tests.
set(SHIM_SOURCES
    src/Pixels.cpp
    src/PixelsFrame.cpp
    src/Settings.cpp
    src/MessageBuilder.cpp
    src/MessageDecoder.cpp
    src/SerialShim.cpp
    src/Storage.cpp
    src/WebHost.cpp
    src/WebSocket.cpp)

# On Linux and macOS the window icon comes from the XPM array rather than from a resource file,
# which is what PIXELS gates in src/PixelsFrame.cpp - Windows takes the .ico instead.
if(NOT WIN32)
    set(PIXELS_DEFINE PIXELS)
endif()

# --------------------------------------------------------------------------
# Build settings shared by every target
# --------------------------------------------------------------------------
# An INTERFACE library rather than repeating the include paths, warnings and
# libraries per target: the app and the tests compile the same sources and must
# not drift apart in how they see them.
add_library(WordclockBuildSettings INTERFACE)

# The simulator's own headers, plus the project root (holds WordclockIcon.xpm).
# Keep include/sim ahead of include so that the core's own includes - "Pixels.h" and the rest
# of the platform seam, by name and never by path - resolve to this backend's implementations
# without a wrapper header in between.
target_include_directories(WordclockBuildSettings INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/sim"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${FIRMWARE_DIR}")

# Every module directory of the core (Animation, Clock, Display, ...) and the ones nested in
# them (Communication/MessageParser/Base), recursively, as platform/avr-dx does it: a list of the
# nested ones had to be extended by hand whenever a module grew a subfolder.
file(GLOB_RECURSE FIRMWARE_ENTRIES LIST_DIRECTORIES true "${FIRMWARE_DIR}/*")
foreach(dir ${FIRMWARE_ENTRIES})
    if(IS_DIRECTORY "${dir}")
        target_include_directories(WordclockBuildSettings INTERFACE "${dir}")
    endif()
endforeach()
target_include_directories(WordclockBuildSettings INTERFACE
    # Where the rule above puts WebPage.h.
    "${CMAKE_CURRENT_BINARY_DIR}/generated")

if(MSVC)
    target_compile_options(WordclockBuildSettings INTERFACE /W4)
else()
    # Each flag asked of the compiler first, because this is the one build that meets whatever
    # compiler its host has - and an unknown -W is a warning of its own, which -Werror in CI
    # would turn into a failed build on a clang.
    include(CheckCXXCompilerFlag)
    foreach(flag ${WORDCLOCK_WARNINGS})
        string(MAKE_C_IDENTIFIER "HAS${flag}" known)
        check_cxx_compiler_flag(${flag} ${known})
        if(${known})
            target_compile_options(WordclockBuildSettings INTERFACE ${flag})
        endif()
    endforeach()
    # wxWidgets' layout flags are two enumerations or'd together - wxALL | wxEXPAND - which is how
    # that library is written, and clang's -Wconversion reports each of them. The same reason
    # .clang-tidy gives for bugprone-suspicious-enum-usage. GCC has no such warning.
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(WordclockBuildSettings INTERFACE -Wno-enum-enum-conversion)
    endif()
endif()

target_compile_definitions(WordclockBuildSettings INTERFACE ${PIXELS_DEFINE})

# What the presets sanitize, coverage and fuzz instrument with, on every object of this build and
# not on the tests alone: the branches coverage counts and libFuzzer steers by are the core's.
target_compile_options(WordclockBuildSettings INTERFACE ${CHECK_INSTRUMENT})
target_link_options(WordclockBuildSettings INTERFACE ${CHECK_INSTRUMENT})

# The firmware's web front end, compiled in. This backend serves the same two pages a clock
# does - which is the whole point of it being here rather than only on the boards: the pages
# can be opened from the binary that already draws the window, with no board and no separate
# host process behind them.
target_compile_definitions(WordclockBuildSettings INTERFACE WEB_FRONTEND_SUPPORT=STD_ON)

# Winsock, which is not linked by default the way the POSIX sockets are. And the lean
# windows.h, because the full one brings the RPC headers, whose `boolean` is an unsigned char
# where arduino/Types.h has already made it a bool - WebHost.cpp included both and did not
# compile. Set here rather than above the include in that file, so that it holds wherever
# windows.h is first reached from.
if(WIN32)
    target_link_libraries(WordclockBuildSettings INTERFACE ws2_32)
    target_compile_definitions(WordclockBuildSettings INTERFACE WIN32_LEAN_AND_MEAN)
endif()
target_link_libraries(WordclockBuildSettings INTERFACE ${wxWidgets_LIBRARIES})

# --------------------------------------------------------------------------
# Shared objects
# --------------------------------------------------------------------------
# OBJECT libraries so the firmware core and the shims are compiled once and
# reused by both executables instead of once per executable.
add_library(WordclockCore OBJECT ${ARDUINO_SOURCES})
target_link_libraries(WordclockCore PUBLIC WordclockBuildSettings)
# WebFrontend.cpp is in here and includes the generated header, so it has to exist first.
add_dependencies(WordclockCore WordclockWebPage)

add_library(WordclockShims OBJECT ${SHIM_SOURCES})
target_link_libraries(WordclockShims PUBLIC WordclockBuildSettings)
add_dependencies(WordclockShims WordclockWebPage)

# --------------------------------------------------------------------------
# Simulator application
# --------------------------------------------------------------------------
set(APP_SOURCES
    WordclockApp.cpp
    WordclockMain.cpp)

# Windows: compile the icon/resource script.
if(WIN32)
    list(APPEND APP_SOURCES resource.rc)
endif()

add_executable(Wordclock WIN32 ${APP_SOURCES})
target_link_libraries(Wordclock PRIVATE WordclockCore WordclockShims)

# --------------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------------
if(WORDCLOCK_BUILD_TESTS)
    # One binary out of several files, not one binary per file: the ESP32 side went that
    # way and had to build an object cache to stop compiling the core once per executable.
    file(GLOB TEST_CASES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/test/cases/*.cpp)
    add_executable(WordclockTests ${CMAKE_SOURCE_DIR}/test/WordclockTests.cpp ${TEST_CASES})
    # The root, so that a test names its helpers as "test/check.h", as Google wants an include to name
    # the folder.
    target_include_directories(WordclockTests PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(WordclockTests PRIVATE WordclockCore WordclockShims)
    add_test(NAME WordclockTests COMMAND WordclockTests)
endif()

# --------------------------------------------------------------------------
# Fuzzers
# --------------------------------------------------------------------------
# One executable per *Fuzz.cpp of test/fuzz, by add_fuzzers() of cmake/checks.cmake, into fuzz/
# of the build, where tools/fuzz.sh runs them. Each is linked against the same core and shims as
# the tests, gathered into one library, as add_fuzzers() takes one: a static one, because the
# objects of an OBJECT library reach only what links it directly. The core is instrumented as
# well, through CHECK_INSTRUMENT on WordclockBuildSettings above.
if(WORDCLOCK_FUZZ)
    add_library(WordclockFuzzBase STATIC
        $<TARGET_OBJECTS:WordclockCore> $<TARGET_OBJECTS:WordclockShims>)
    target_link_libraries(WordclockFuzzBase PUBLIC WordclockBuildSettings)
endif()
add_fuzzers(DIRECTORY ${CMAKE_SOURCE_DIR}/test/fuzz LIBRARY WordclockFuzzBase)
