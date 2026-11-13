# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# What every build on the host is checked with, for CMakeLists.txt to include: the warnings,
# the instrumentation of the presets sanitize and coverage, and the tests of what must not
# compile. This file comes from the template and is brought up to date with `copier update`;
# what the project builds is in CMakeLists.txt.
#
#   CHECK_WARNINGS     the warnings, for target_compile_options(); errors only where
#                      CMAKE_COMPILE_WARNING_AS_ERROR is on, as the ci- presets set it
#   CHECK_INSTRUMENT   --coverage, or the sanitizers, for the compile and the link options
#   add_compile_fail_tests(DIRECTORY dir STANDARD std INCLUDES dir...)
#   add_compile_warn_tests(DIRECTORY dir STANDARDS std... INCLUDES dir...)
#   add_fuzzers(DIRECTORY dir LIBRARY target)

option(WORDCLOCK_COVERAGE "Build with --coverage, for gcovr" OFF)
option(WORDCLOCK_SANITIZE "Build with AddressSanitizer and UBSan" OFF)
option(WORDCLOCK_FUZZ "Build the fuzzers of test/fuzz/, with libFuzzer, ASan and UBSan" OFF)

# -Wconversion and -Wsign-conversion above all: an int has 16 bits on the AVR, and a
# conversion that is harmless on the host can lose a value there. Braced initialization finds
# a narrowing only where a variable is initialized, not in the arithmetic after it. -Wshadow,
# as a name that hides another is read as the other. No -Werror: CMAKE_COMPILE_WARNING_AS_ERROR
# decides, on in CI and under `make check`, off while somebody is in the middle of debugging.
set(CHECK_WARNINGS -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow)

set(CHECK_INSTRUMENT)
if(WORDCLOCK_COVERAGE)
  list(APPEND CHECK_INSTRUMENT --coverage)
  # And whether each condition of a decision alone turns its outcome, MC/DC, which gcovr reads
  # beside the lines and branches (docs/Testing.md TS-4): GCC from 14. Without it,
  # tools/coverage.sh checks no condition and would not say so, so the configuration does.
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
    list(APPEND CHECK_INSTRUMENT -fcondition-coverage)
  else()
    message(WARNING "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} measures no condition: "
                    "tools/coverage.sh checks the lines and branches alone. GCC 14 and later do.")
  endif()
endif()
if(WORDCLOCK_SANITIZE)
  list(APPEND CHECK_INSTRUMENT -fsanitize=address,undefined
       -fno-sanitize-recover=all -fno-omit-frame-pointer)
endif()
# The library as well as each fuzzer: libFuzzer steers by the coverage of the code it reaches, and
# code built without -fsanitize=fuzzer-no-link is code it cannot see into. clang only, which
# brings libFuzzer; the preset fuzz names it.
if(WORDCLOCK_FUZZ)
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR "WORDCLOCK_FUZZ needs clang, which brings libFuzzer: cmake --preset fuzz")
  endif()
  list(APPEND CHECK_INSTRUMENT -fsanitize=fuzzer-no-link,address,undefined
       -fno-sanitize-recover=all -fno-omit-frame-pointer)
endif()

# What must not compile: each .cpp file in DIRECTORY uses the library wrongly, and its test
# passes only if the compiler stops it with the message of the static_assert that its line
# "// Expected: ..." names. A static_assert that went missing, or another error, fails it. The
# compiler is called directly, with -fsyntax-only and in STANDARD, as avr-gcc builds a sketch:
# no target, so that tests run in parallel never build in the same tree at once.
function(add_compile_fail_tests)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "DIRECTORY;STANDARD" "INCLUDES")
  set(includes)
  foreach(dir IN LISTS arg_INCLUDES)
    list(APPEND includes -I${dir})
  endforeach()
  file(GLOB cases CONFIGURE_DEPENDS ${arg_DIRECTORY}/*.cpp)
  foreach(case IN LISTS cases)
    get_filename_component(name ${case} NAME_WE)
    file(STRINGS ${case} expected REGEX "^// Expected: " LIMIT_COUNT 1)
    string(REGEX REPLACE "^// Expected: " "" expected "${expected}")
    if(expected STREQUAL "")
      message(FATAL_ERROR "${case} names no message: a line \"// Expected: \".")
    endif()
    add_test(NAME compile_fail/${name}
             COMMAND ${CMAKE_CXX_COMPILER} -std=${arg_STANDARD} -fsyntax-only
                     ${includes} ${case})
    # GCC says "static assertion failed: <message>", Clang "static assertion failed due to
    # requirement '...': <message>"; both on one line.
    set_tests_properties(
      compile_fail/${name}
      PROPERTIES PASS_REGULAR_EXPRESSION
                 "static assertion failed[^\n]*${expected}")
  endforeach()
endfunction()

# What the compiler has to warn of: each .cpp file in DIRECTORY does what compiles but is a
# mistake, as dropping a result WORDCLOCK_NODISCARD marks, and its test passes only if
# the compiler warns with what its line "// Expected: ..." names, a regular expression, in each
# of STANDARDS: a macro may be another attribute in each. Compiled to an object, with the
# warnings of the build, as GCC reports warn_unused_result only when it generates code, not
# under -fsyntax-only. clang-tidy does not read the cases (tools/tidy.sh), which warn on purpose
# and are in no database.
function(add_compile_warn_tests)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "DIRECTORY" "STANDARDS;INCLUDES")
  set(includes)
  foreach(dir IN LISTS arg_INCLUDES)
    list(APPEND includes -I${dir})
  endforeach()
  file(GLOB cases CONFIGURE_DEPENDS ${arg_DIRECTORY}/*.cpp)
  foreach(case IN LISTS cases)
    get_filename_component(name ${case} NAME_WE)
    file(STRINGS ${case} expected REGEX "^// Expected: " LIMIT_COUNT 1)
    string(REGEX REPLACE "^// Expected: " "" expected "${expected}")
    if(expected STREQUAL "")
      message(FATAL_ERROR "${case} names no warning: a line \"// Expected: \".")
    endif()
    foreach(standard IN LISTS arg_STANDARDS)
      add_test(NAME compile_warn/${name}/${standard}
               COMMAND ${CMAKE_CXX_COMPILER} -std=${standard} ${CHECK_WARNINGS} ${includes}
                       -c ${case} -o ${CMAKE_CURRENT_BINARY_DIR}/compile_warn_${name}_${standard}.o)
      set_tests_properties(
        compile_warn/${name}/${standard}
        PROPERTIES PASS_REGULAR_EXPRESSION "warning: [^\n]*${expected}")
    endforeach()
  endforeach()
endfunction()

# The fuzzers: each <Name>Fuzz.cpp or <name>_fuzz.cpp of DIRECTORY defines LLVMFuzzerTestOneInput()
# and becomes the program <Name> in fuzz/ of the build, linked with LIBRARY and libFuzzer, under
# ASan and UBSan, so that what a fuzzer finds is a crash, a read out of bounds or undefined
# behaviour. Only with WORDCLOCK_FUZZ on; tools/fuzz.sh runs them.
function(add_fuzzers)
  cmake_parse_arguments(PARSE_ARGV 0 arg "" "DIRECTORY;LIBRARY" "")
  if(NOT WORDCLOCK_FUZZ)
    return()
  endif()
  file(GLOB fuzzers CONFIGURE_DEPENDS ${arg_DIRECTORY}/*Fuzz.cpp ${arg_DIRECTORY}/*_fuzz.cpp)
  foreach(source IN LISTS fuzzers)
    get_filename_component(name ${source} NAME_WE)
    add_executable(${name} ${source})
    target_compile_features(${name} PRIVATE cxx_std_17)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_options(${name} PRIVATE ${CHECK_WARNINGS} ${CHECK_INSTRUMENT})
    target_link_options(${name} PRIVATE -fsanitize=fuzzer,address,undefined)
    target_link_libraries(${name} PRIVATE ${arg_LIBRARY})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/fuzz)
  endforeach()
endfunction()
