# Wordclock, AVR Dx backend.
#
# The firmware core is not copied or vendored in here: the sources are picked up from
# ../../firmware/src and its header directories are put on the include path. There is
# therefore exactly one copy of the clock logic, shared with the simulator and the ESP32.

cmake_minimum_required(VERSION 3.16)
project(Wordclock LANGUAGES CXX)

set(AVR_MCU avr128da48 CACHE STRING "Target device")
set(AVR_F_CPU 24000000 CACHE STRING "Peripheral clock in Hz, the timing constants are derived from it")
# UPDI programmer for the flash target; pymcuprog names it, see its --help for the others.
set(AVR_PROGRAMMER pkobn_updi CACHE STRING "pymcuprog tool name used by the flash target")
# For a Dx part avr-libc does not know: an unpacked Microchip .atpack. Defaulted from the
# environment as well as the cache, so an image or a job that carries one does not make
# every build command name it.
set(AVR_DFP "$ENV{AVR_DFP}" CACHE PATH "Optional Microchip device pack")

set(FIRMWARE_DIR ${CMAKE_SOURCE_DIR}/firmware)

file(GLOB_RECURSE FIRMWARE_SOURCES CONFIGURE_DEPENDS ${FIRMWARE_DIR}/src/*.cpp)
file(GLOB PLATFORM_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Every directory under the core's include tree, because its headers include each other
# by bare name rather than by path.
file(GLOB_RECURSE FIRMWARE_INCLUDE_DIRS LIST_DIRECTORIES true ${FIRMWARE_DIR}/inc/*)
list(FILTER FIRMWARE_INCLUDE_DIRS EXCLUDE REGEX "\\.[hc]pp?$")

add_executable(Wordclock ${PLATFORM_SOURCES} ${FIRMWARE_SOURCES})

target_include_directories(Wordclock PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stl
    ${FIRMWARE_DIR}/inc
    ${FIRMWARE_INCLUDE_DIRS})

target_compile_definitions(Wordclock PRIVATE F_CPU=${AVR_F_CPU}UL)

# Release unless asked otherwise: this is a firmware image, and the size it has to fit in
# is the whole reason for the part it runs on.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Release for the flash budget, Debug to step through it" FORCE)
endif()

# Optimised for size and linked whole-program. -flto is worth 9 KiB, see README.md.
set(AVR_OPTIONS_RELEASE -Os -flto)
# -Og and no LTO, because a debugger is the point here: inlining across translation units
# leaves breakpoints on lines that no longer exist and locals the target cannot show.
# -g3 rather than -g so that the configuration macros are visible too - most of the timing
# in this backend is a macro derived from F_CPU.
set(AVR_OPTIONS_DEBUG -Og -g3)

# -fno-threadsafe-statics: the singletons are function-local statics, and the guards the
#   standard asks for pull in __cxa_guard_acquire, which avr-libc does not have. There is
#   one thread here, so the guards protect against nothing.
# -fno-exceptions / -fno-rtti: neither is used, and both cost flash whether used or not.
target_compile_options(Wordclock PRIVATE
    -mmcu=${AVR_MCU}
    -Wall -Wextra
    -fno-exceptions -fno-rtti -fno-threadsafe-statics
    -ffunction-sections -fdata-sections
    "$<$<CONFIG:Debug>:${AVR_OPTIONS_DEBUG}>"
    "$<$<NOT:$<CONFIG:Debug>>:${AVR_OPTIONS_RELEASE}>")

target_link_options(Wordclock PRIVATE
    -mmcu=${AVR_MCU}
    -Wl,--gc-sections
    "$<$<CONFIG:Debug>:${AVR_OPTIONS_DEBUG}>"
    "$<$<NOT:$<CONFIG:Debug>>:${AVR_OPTIONS_RELEASE}>")

set_target_properties(Wordclock PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS ON
    SUFFIX ".elf")

if(AVR_DFP)
    target_compile_options(Wordclock PRIVATE -B ${AVR_DFP}/gcc/dev/${AVR_MCU}/ -I${AVR_DFP}/include)
    target_link_options(Wordclock PRIVATE -B ${AVR_DFP}/gcc/dev/${AVR_MCU}/)
endif()

# The image that gets flashed, and what it costs - printed on every build rather than
# asked for, because the flash budget is the reason this part was chosen.
add_custom_command(TARGET Wordclock POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom $<TARGET_FILE:Wordclock> $<TARGET_FILE_DIR:Wordclock>/Wordclock.hex
    COMMAND ${CMAKE_SIZE} --format=avr --mcu=${AVR_MCU} $<TARGET_FILE:Wordclock>
    VERBATIM)

add_custom_target(flash
    COMMAND pymcuprog write -d ${AVR_MCU} -t ${AVR_PROGRAMMER} -f $<TARGET_FILE_DIR:Wordclock>/Wordclock.hex --erase --verify
    DEPENDS Wordclock
    VERBATIM)
