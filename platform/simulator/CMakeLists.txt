cmake_minimum_required(VERSION 3.16)

project(Wordclock LANGUAGES CXX)

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
# tell a reader to run: build/bin/Wordclock.
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
set(FIRMWARE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../firmware")

# Firmware logic shared with the real hardware target...
file(GLOB_RECURSE ARDUINO_SOURCES CONFIGURE_DEPENDS
    "${FIRMWARE_DIR}/src/*.cpp")

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
    src/WebHost.cpp)

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
    "${FIRMWARE_DIR}/inc")

# Every firmware inc/* module directory (Animation, Clock, Display, ...).
file(GLOB ARDUINO_INC_DIRS LIST_DIRECTORIES true
    "${FIRMWARE_DIR}/inc/*")
foreach(dir ${ARDUINO_INC_DIRS})
    if(IS_DIRECTORY "${dir}")
        target_include_directories(WordclockBuildSettings INTERFACE "${dir}")
    endif()
endforeach()
# Nested Communication/* dirs are not caught by the glob above.
target_include_directories(WordclockBuildSettings INTERFACE
    "${FIRMWARE_DIR}/inc/Communication/MessageParser"
    "${FIRMWARE_DIR}/inc/Communication/MessageParser/Base"
    "${FIRMWARE_DIR}/inc/Communication/WebFrontend"
    # Where the rule above puts WebPage.h.
    "${CMAKE_CURRENT_BINARY_DIR}/generated")

if(MSVC)
    target_compile_options(WordclockBuildSettings INTERFACE /W4)
else()
    target_compile_options(WordclockBuildSettings INTERFACE -Wall -Wextra)
endif()

target_compile_definitions(WordclockBuildSettings INTERFACE ${PIXELS_DEFINE})

# The firmware's web front end, compiled in. This backend serves the same two pages a clock
# does - which is the whole point of it being here rather than only on the boards: the pages
# can be opened from the binary that already draws the window, with no board and no separate
# host process behind them.
target_compile_definitions(WordclockBuildSettings INTERFACE WEB_FRONTEND_SUPPORT=STD_ON)

# Winsock, which is not linked by default the way the POSIX sockets are.
if(WIN32)
    target_link_libraries(WordclockBuildSettings INTERFACE ws2_32)
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
    file(GLOB TEST_CASES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/cases/*.cpp)
    add_executable(WordclockTests tests/WordclockTests.cpp ${TEST_CASES})
    target_include_directories(WordclockTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    target_link_libraries(WordclockTests PRIVATE WordclockCore WordclockShims)
    add_test(NAME WordclockTests COMMAND WordclockTests)
endif()
