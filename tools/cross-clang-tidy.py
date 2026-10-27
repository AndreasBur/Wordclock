#!/usr/bin/env python3
"""Runs clang-tidy over a build that a GCC cross compiler made.

clang-tidy is clang, and a compile database is a list of what *GCC* was told. Read as it
is, the three hardware builds fail in a way that does not look like failing: clang finds
none of the toolchain's own headers - <limits>, <atomic>, avr-libc - so every translation
unit stops part-way, and what is left over is reported as findings. Loops whose counter is
declared in a header it did not read are "infinite", branches whose types it never saw are
"identical". On the ESP32 it also stops at -mlongcalls and the other flags only GCC has.

So the database is rewritten first, into a directory of its own, and only then read:

- the target is named, taken from the compiler's own name (avr-g++ -> avr);
- the compiler is asked where its headers are, and those directories are passed instead
  of clang's guesses for the host;
- the flags clang does not know are dropped. They are code generation options, none of
  them changes what a header declares;
- relative include directories are made absolute. PlatformIO passes -Iinclude, a header
  is then known as include/Pixels.h, and .clang-tidy's HeaderFilterRegex - which looks
  for /platform/ - quietly skipped every header the backend owns;
- so are the defines that only exist for a build system. PICO_PLATFORM=rp2350-arm-s is
  the Pico SDK's CMake variable passed on as a macro, read by no source, and as a macro it
  is an unparenthesised subtraction that bugprone-macro-parentheses reports against every
  translation unit - with no file to put a NOLINT in.

The checks are .clang-tidy's, the same ones the simulator is read with.

Usage:
    tools/cross-clang-tidy.py build-avr 'firmware/|platform/avr-dx/src'
    tools/cross-clang-tidy.py platform/esp32 'firmware/|platform/esp32/src'
"""

import argparse
import json
import os
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

GCC_ONLY_FLAGS = {
    '-fno-shrink-wrap',
    '-fno-tree-switch-conversion',
    '-fstrict-volatile-bitfields',
    '-mdisable-hardware-atomics',
    '-mlongcalls',
    '-mtext-section-literals',
}

UNREAD_DEFINES = {
    'PICO_PLATFORM',
}


def arguments_of(entry):
    return entry['arguments'] if 'arguments' in entry else shlex.split(entry['command'])


def cxx_compiler(database):
    """The C++ compiler, by full path. PlatformIO names the C compiler bare and the C++ one
    with its directory, and only the latter can be run from here."""
    for entry in database:
        compiler = arguments_of(entry)[0]
        if compiler.endswith('g++') and os.path.isabs(compiler):
            return compiler
    raise SystemExit('cross-clang-tidy.py: no C++ compiler with a full path in the database')


def clang_target(compiler):
    """xtensa-esp32s3-elf-g++ -> xtensa: clang knows the architecture, not Espressif's
    name for one chip of it. The others are triples clang takes as they are."""
    prefix = os.path.basename(compiler).removesuffix('-g++')
    return 'xtensa' if prefix.startswith('xtensa') else prefix


def system_includes(compiler):
    """Exactly the list GCC prints for -v, in its order: the C++ library first and the C
    library last, which is the order #include_next depends on."""
    output = subprocess.run([compiler, '-xc++', '-E', '-v', '-'], input='',
                            capture_output=True, text=True, check=True).stderr.splitlines()
    start = output.index('#include <...> search starts here:') + 1
    end = output.index('End of search list.')
    return [os.path.normpath(line.strip()) for line in output[start:end]]


def is_dropped(argument):
    if argument in GCC_ONLY_FLAGS:
        return True
    return argument.startswith('-D') and argument[2:].split('=')[0] in UNREAD_DEFINES


def absolute_includes(arguments, directory):
    """-Iinclude and -I include alike; -isystem and the rest are the framework's, absolute
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
    """-nostdlibinc rather than -nostdinc: it drops the host's /usr/include and keeps
    clang's own builtin headers, which GCC's include directory relies on for stdarg and
    friends the same way clang's does."""
    kept = [argument for argument in arguments[1:] if not is_dropped(argument)]
    flags = [flag for directory in includes for flag in ('-isystem', directory)]
    return [arguments[0], f'--target={target}', '-nostdlibinc', '-nostdinc++'] + flags + kept


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('build', type=Path, help='the directory holding compile_commands.json')
    parser.add_argument('files', help="which translation units to read, as run-clang-tidy's regex")
    arguments = parser.parse_args()

    database = json.loads((arguments.build / 'compile_commands.json').read_text())
    compiler = cxx_compiler(database)
    target = clang_target(compiler)
    includes = system_includes(compiler)
    print(f'cross-clang-tidy.py: {compiler} as --target={target}', flush=True)

    for entry in database:
        entry['arguments'] = clang_arguments(absolute_includes(arguments_of(entry), entry['directory']),
                                             target, includes)
        entry.pop('command', None)

    with tempfile.TemporaryDirectory() as directory:
        (Path(directory) / 'compile_commands.json').write_text(json.dumps(database))
        return subprocess.run(['run-clang-tidy', '-p', directory, '-quiet', '-j', str(os.cpu_count()),
                               arguments.files]).returncode


if __name__ == '__main__':
    sys.exit(main())
