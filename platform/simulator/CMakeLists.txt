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

# Put binaries next to where the old Code::Blocks projects placed them.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# --------------------------------------------------------------------------
# wxWidgets  (replaces the per-platform `wx-config` / *.a handling
#             that used to live in the three .cbp files)
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

# ...plus the wxWidgets stand-ins for the hardware. Only the two entry-point
# files below are specific to the app; everything here is shared with the tests.
set(SHIM_SOURCES
    src/Pixels.cpp
    src/PixelsFrame.cpp
    src/Settings.cpp
    src/MessageBuilder.cpp
    src/MessageDecoder.cpp
    src/SerialShim.cpp
    src/Storage.cpp)

# On Linux/macOS the window icon is loaded from the XPM array (see src/PixelsFrame.cpp),
# gated behind the PIXELS define that the old Code::Blocks projects set.
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
# Keep include/sim ahead of include so legacy includes like "Pixels.h" resolve
# directly to simulator implementations even without compatibility wrappers.
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
# Nested Communication/MessageParser dirs are not caught by the glob above.
target_include_directories(WordclockBuildSettings INTERFACE
    "${FIRMWARE_DIR}/inc/Communication/MessageParser"
    "${FIRMWARE_DIR}/inc/Communication/MessageParser/Base")

if(MSVC)
    target_compile_options(WordclockBuildSettings INTERFACE /W4)
else()
    target_compile_options(WordclockBuildSettings INTERFACE -Wall -Wextra)
endif()

target_compile_definitions(WordclockBuildSettings INTERFACE ${PIXELS_DEFINE})
target_link_libraries(WordclockBuildSettings INTERFACE ${wxWidgets_LIBRARIES})

# --------------------------------------------------------------------------
# Shared objects
# --------------------------------------------------------------------------
# OBJECT libraries so the firmware core and the shims are compiled once and
# reused by both executables instead of once per executable.
add_library(WordclockCore OBJECT ${ARDUINO_SOURCES})
target_link_libraries(WordclockCore PUBLIC WordclockBuildSettings)

add_library(WordclockShims OBJECT ${SHIM_SOURCES})
target_link_libraries(WordclockShims PUBLIC WordclockBuildSettings)

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
    add_executable(WordclockTests tests/WordclockTests.cpp)
    target_link_libraries(WordclockTests PRIVATE WordclockCore WordclockShims)
    add_test(NAME WordclockTests COMMAND WordclockTests)
endif()
