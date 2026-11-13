#!/usr/bin/env python3
"""Compiles every firmware header on its own, as the only include of an otherwise empty file.

A header that uses something it does not include still compiles in the build, as long as
every file that includes it happens to have included that something first. Nothing notices
until the include it leaned on goes - and then what breaks is a file nobody touched.
StringTools.h called strncpy for months on the <string.h> its includers brought along;
a037fc2a dropped its Arduino.h as unused, which it was by every check there is, and the
build stayed green only because every includer still pulled the C library in first.
misc-include-cleaner cannot see this, because it asks what a file uses, not whether the
file compiles without its neighbours.

So each header is compiled alone, with the flags the build gives a firmware source and by
the compiler the build uses: g++ or clang for the simulator, the cross compiler for each
board. Taking both from compile_commands.json is what makes the four runs differ where the
targets do - an #if branch only the AVR selects is only compiled for the AVR - and it needs
no translation of GCC's flags, since nothing but GCC ever reads them.

The core is checked by every target and each backend's headers by its own, so a run names
the backend's include directory beside src:

    tools/self-contained-headers.py build src platform/simulator/include
    tools/self-contained-headers.py build-avr src platform/avr-dx/include
    tools/self-contained-headers.py platform/esp32 src platform/esp32/include platform/arduino/include
"""

import argparse
import concurrent.futures
import json
import os
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# What a compile of one source is told about its output. A syntax check writes nothing, and
# -MF would have it write a dependency file over the build's own.
OUTPUT_FLAGS_WITH_VALUE = {'-o', '-MF', '-MT', '-MQ'}
OUTPUT_FLAGS = {'-c', '-MD', '-MMD'}


def arguments_of(entry):
    return entry['arguments'] if 'arguments' in entry else shlex.split(entry['command'])


def firmware_entry(database):
    """The flags of a firmware source. They are the same for every source under src/:
    a target compiles its core with one set of defines and include directories."""
    for entry in database:
        # By its whole path: '/src/' alone would take a source of platform/<backend>/src.
        file = Path(entry['directory'], entry['file']).resolve()
        if file.is_relative_to(ROOT / 'src') and file.suffix == '.cpp':
            return entry
    raise SystemExit('self-contained-headers.py: no firmware source in the database')


def syntax_check_arguments(arguments, source):
    """The compile of `source` with the same compiler and flags, checking and writing nothing."""
    result = [arguments[0]]
    skip = False
    for argument in arguments[1:]:
        if skip:
            skip = False
        elif argument in OUTPUT_FLAGS_WITH_VALUE:
            skip = True
        elif argument in OUTPUT_FLAGS or argument.endswith(('.cpp', '.c')):
            pass
        else:
            result.append(argument)
    return result + ['-fsyntax-only', str(source)]


def check(header, arguments, directory, scratch):
    """The first error of compiling `header` alone, or None."""
    source = Path(scratch) / (header.relative_to(ROOT).as_posix().replace('/', '_') + '.cpp')
    source.write_text(f'#include "{header}"\n')
    result = subprocess.run(syntax_check_arguments(arguments, source), cwd=directory,
                            capture_output=True, text=True)
    if result.returncode == 0:
        return None
    errors = [line for line in result.stderr.splitlines() if 'error' in line]
    return errors[0] if errors else result.stderr.strip().splitlines()[-1]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('build', type=Path, help='the directory holding compile_commands.json')
    parser.add_argument('headers', nargs='*', type=Path, default=[ROOT / 'src'],
                        help='the directories whose headers are checked (default: src)')
    arguments = parser.parse_args()

    database = json.loads((arguments.build / 'compile_commands.json').read_text())
    entry = firmware_entry(database)
    compile_arguments = arguments_of(entry)
    headers = sorted(header for directory in arguments.headers for header in directory.resolve().rglob('*.h'))
    print(f'self-contained-headers.py: {len(headers)} headers by {os.path.basename(compile_arguments[0])}',
          flush=True)

    with tempfile.TemporaryDirectory() as scratch, concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
        results = pool.map(lambda header: (header, check(header, compile_arguments, entry['directory'], scratch)),
                           headers)
        failures = [(header, error) for header, error in results if error]

    for header, error in failures:
        print(f'{header.relative_to(ROOT)}: does not compile on its own\n    {error}')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
