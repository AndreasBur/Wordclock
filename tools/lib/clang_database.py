#!/usr/bin/env python3
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Rewrites the compilation database of a GCC cross compiler into one that clang-tidy reads.

clang-tidy is clang, and a compilation database is a list of what GCC was told. Read as it is,
clang finds none of the toolchain's own headers, avr-libc's among them, so every translation unit
stops part of the way, and what is left is reported as findings: a loop whose counter is
declared in a header it did not read is "infinite", branches whose types it never saw are
"identical". So each entry is changed:

- the target is named, taken from the compiler's own name (avr-g++ -> avr);
- the compiler is asked where its headers are, and those folders are passed instead of clang's
  guesses for the host, in GCC's order, which #include_next depends on;
- the flags clang does not know are dropped: options of code generation, none of which changes
  what a header declares;
- a relative include folder is made absolute, as a header known by a relative path escapes the
  HeaderFilterRegex of .clang-tidy;
- -Wno-unknown-attributes, as clang does not know avr-gcc's __progmem__ and takes such a table
  as in RAM, which changes nothing clang-tidy checks;
- -Wno-unknown-warning-option and -Wno-ignored-optimization-argument, as a build may ask GCC for
  warnings and optimisations clang does not have, as -Wduplicated-cond or ESP-IDF's
  -freorder-blocks, and clang-tidy reports each as an error of every file;
- a define no source reads, which only a build system passes on, is dropped;
- each file is named by its whole path, as a build system may name it relative to its own.

Out of Wordclock's tools/cross-clang-tidy.py, which reads the same three kinds of build.

Usage:
    tools/lib/clang_database.py BUILD OUTPUT
"""

import argparse
import json
import os
import shlex
import subprocess
import sys
from pathlib import Path

# Defines no source reads, which only a build system passes on: PICO_PLATFORM=rp2350-arm-s is the
# Pico SDK's CMake variable as a macro, an unparenthesised subtraction that
# bugprone-macro-parentheses reports against every translation unit, with no file to answer in.
UNREAD_DEFINES = {
    'PICO_PLATFORM',
}

GCC_ONLY_FLAGS = {
    '-fcallgraph-info=su',
    '-fno-shrink-wrap',
    '-fno-tree-switch-conversion',
    '-fstrict-volatile-bitfields',
    '-mdisable-hardware-atomics',
    '-mlongcalls',
    '-mtext-section-literals',
}


def arguments_of(entry):
    return entry['arguments'] if 'arguments' in entry else shlex.split(entry['command'])


def cxx_compiler(database):
    """The C++ compiler, by its full path; a build system may name the C compiler bare."""
    for entry in database:
        compiler = arguments_of(entry)[0]
        if compiler.endswith('g++') and os.path.isabs(compiler):
            return compiler
    raise SystemExit('clang_database.py: no C++ compiler with a full path in the database')


def clang_target(compiler):
    """xtensa-esp32s3-elf-g++ -> xtensa: clang knows the architecture, not Espressif's name for
    one chip of it. The others are triples clang takes as they are."""
    prefix = os.path.basename(compiler).removesuffix('-g++')
    return 'xtensa' if prefix.startswith('xtensa') else prefix


def system_includes(compiler):
    """Exactly the list GCC prints for -v, in its order: the C++ library first and the C library
    last."""
    output = subprocess.run([compiler, '-xc++', '-E', '-v', '-'], input='',
                            capture_output=True, text=True, check=True).stderr.splitlines()
    start = output.index('#include <...> search starts here:') + 1
    end = output.index('End of search list.')
    return [os.path.normpath(line.strip()) for line in output[start:end]]


def absolute_includes(arguments, directory):
    """-Iinclude and -I include alike; -isystem and the rest are a framework's, absolute
    already, and left alone."""
    result = []
    joined = False
    for argument in arguments:
        if joined:
            argument = os.path.normpath(os.path.join(directory, argument))
            joined = False
        elif argument == '-I':
            joined = True
        elif argument.startswith('-I'):
            argument = '-I' + os.path.normpath(os.path.join(directory, argument[2:]))
        result.append(argument)
    return result


def clang_arguments(arguments, target, includes):
    """-nostdlibinc rather than -nostdinc: it drops the host's /usr/include and keeps clang's own
    builtin headers, which GCC's include folders rely on for stdarg and the like as clang's do."""
    kept = [argument for argument in arguments[1:] if argument not in GCC_ONLY_FLAGS
            and not (argument.startswith('-D') and argument[2:].split('=')[0] in UNREAD_DEFINES)]
    flags = [flag for directory in includes for flag in ('-isystem', directory)]
    return ([arguments[0], f'--target={target}', '-nostdlibinc', '-nostdinc++'] + flags + kept
            + ['-Wno-unknown-attributes', '-Wno-unknown-warning-option',
               '-Wno-ignored-optimization-argument'])


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('build', type=Path, help='the folder of the compile_commands.json to read')
    parser.add_argument('output', type=Path, help='the folder to write the rewritten one into')
    arguments = parser.parse_args()

    database = json.loads((arguments.build / 'compile_commands.json').read_text())
    compiler = cxx_compiler(database)
    target = clang_target(compiler)
    includes = system_includes(compiler)
    for entry in database:
        entry['arguments'] = clang_arguments(
            absolute_includes(arguments_of(entry), entry['directory']), target, includes)
        entry.pop('command', None)
        # By its whole path, as tools/tidy.sh finds a file's entry: PlatformIO names one relative
        # to its project, as ../../src/Blinker/Blinker.cpp.
        entry['file'] = os.path.normpath(os.path.join(entry['directory'], entry['file']))
    arguments.output.mkdir(parents=True, exist_ok=True)
    (arguments.output / 'compile_commands.json').write_text(json.dumps(database, indent=1))
    return 0


if __name__ == '__main__':
    sys.exit(main())
