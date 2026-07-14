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
# Firmware logic shared with the real hardware target...
file(GLOB_RECURSE ARDUINO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Arduino/src/*.cpp")

# ...plus the wxWidgets simulator glue.
set(SIMULATOR_SOURCES
    WordclockApp.cpp
    WordclockMain.cpp
    src/Pixels.cpp)

set(SOURCES ${ARDUINO_SOURCES} ${SIMULATOR_SOURCES})

# Windows: compile the icon/resource script.
if(WIN32)
    list(APPEND SOURCES resource.rc)
endif()

# On Linux/macOS the window icon is loaded from the XPM array (see src/Pixels.cpp),
# gated behind the PIXELS define that the old Code::Blocks projects set.
if(NOT WIN32)
    set(PIXELS_DEFINE PIXELS)
endif()

add_executable(Wordclock WIN32 ${SOURCES})

# --------------------------------------------------------------------------
# Include directories
# --------------------------------------------------------------------------
# The simulator's own headers, plus the project root (holds WordclockIcon.xpm).
target_include_directories(Wordclock PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Every Arduino/inc/* module directory (Animation, Clock, Display, ...).
file(GLOB ARDUINO_INC_DIRS LIST_DIRECTORIES true
    "${CMAKE_CURRENT_SOURCE_DIR}/Arduino/inc/*")
target_include_directories(Wordclock PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Arduino/inc")
foreach(dir ${ARDUINO_INC_DIRS})
    if(IS_DIRECTORY "${dir}")
        target_include_directories(Wordclock PRIVATE "${dir}")
    endif()
endforeach()
# Nested Communication/MessageParser dirs are not caught by the glob above.
target_include_directories(Wordclock PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Arduino/inc/Communication/MessageParser"
    "${CMAKE_CURRENT_SOURCE_DIR}/Arduino/inc/Communication/MessageParser/Base")

# --------------------------------------------------------------------------
# Warnings & libraries
# --------------------------------------------------------------------------
if(MSVC)
    target_compile_options(Wordclock PRIVATE /W4)
else()
    target_compile_options(Wordclock PRIVATE -Wall -Wextra)
endif()

target_compile_definitions(Wordclock PRIVATE ${PIXELS_DEFINE})

target_link_libraries(Wordclock PRIVATE ${wxWidgets_LIBRARIES})
