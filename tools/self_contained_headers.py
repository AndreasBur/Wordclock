#!/usr/bin/env python3
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Compiles every header of src/ and platform/ on its own, as the only include of an otherwise
empty file.

A header that uses something it does not include still compiles in the build, as long as every
file that includes it happens to have included that something first. Nothing notices until the
include it leaned on goes, and then what breaks is a file nobody touched; misc-include-cleaner
cannot see it either, as it asks what a file uses, not whether the file compiles without its
neighbours.

So each header is compiled alone, by the compiler and with the flags of a compilation database:
the host's, which CMake writes (g++, gnu++11, against the mock of the core), and each target's,
which tools/tidy.sh writes from arduino-cli's (avr-g++, the core and the board's pins) or names
in a CMake build (avr-g++ of the toolchain file). An #if that only a board takes is so compiled
for that board.

A header of platform/<name>/ is compiled with each database that compiles a source of that
folder, with that source's flags, where it lies under one of their include folders: a firmware
of several boards has a folder for each, which a target compiles with its own compiler, and
what lies outside the include folders, as a backend's stubs of its host tests, is no header of
the target's.

Usage:
    tools/self_contained_headers.py build/host @build/tidy/databases

An argument that starts with @ names a file of further arguments, one per line, as
tools/tidy.sh --database-only writes the list of its databases.
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

# What a compile of one source is told about its output and its input; a syntax check writes
# nothing, and -MF would write a dependency file over the build's own.
WITH_VALUE = {'-o', '-MF', '-MT', '-MQ', '-include', '-x'}
ALONE = {'-c', '-MD', '-MMD'}


def arguments_of(entry):
    return entry['arguments'] if 'arguments' in entry else shlex.split(entry['command'])


def path_of(entry):
    """The file of an entry, relative to the repository; None for one outside it."""
    path = (Path(entry['directory']) / entry['file']).resolve()
    return path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else None


def library_entry(database):
    """The flags the library is compiled with: a source of src/, else the mock of the core,
    which the host's library target compiles with the same flags, else an example: a sketch,
    whose `-include Arduino.h` is dropped below, as a header has to include what it uses itself,
    or a program of a target built by CMake. On the host a library of headers alone without a
    mock has none of these, and its headers are compiled with a test's flags, as C++17; a
    target's database still compiles them as the library is."""
    def under(entry, part, suffix):
        path = path_of(entry)
        return path is not None and path.startswith(part) and path.endswith(suffix)

    for part, suffix in (('src/', '.cpp'), ('test/mock/', '.cpp'), ('examples/', '.ino'),
                         ('examples/', '.cpp'), ('test/', '.cpp')):
        for entry in database:
            if under(entry, part, suffix):
                return entry
    raise SystemExit('self_contained_headers.py: no source of the library in the database')


def syntax_check(arguments, source):
    """The compile of `source` as C++ with the same compiler and flags, writing nothing."""
    result = [arguments[0]]
    skip = False
    for argument in arguments[1:]:
        if skip:
            skip = False
        elif argument in WITH_VALUE:
            skip = True
        elif argument in ALONE or argument.endswith(('.cpp', '.c', '.ino')):
            pass
        elif argument.startswith('-isystem'):
            # tools/tidy.sh makes the core's folders -isystem, so that clang-tidy reports nothing
            # of them; but avr-gcc reads a header there as extern "C", and Arduino.h then
            # conflicts with avr-libc. As arduino-cli gives them: -I.
            result.append('-I' + argument[len('-isystem'):])
        else:
            result.append(argument)
    return result + ['-x', 'c++', '-fsyntax-only', str(source)]


def check(header, arguments, directory, scratch):
    """The first error of compiling `header` alone, or None."""
    name = header.relative_to(ROOT).as_posix().replace('/', '_')
    source = Path(scratch) / f'{name}.cpp'
    source.write_text(f'#include "{header}"\n')
    result = subprocess.run(syntax_check(arguments, source), cwd=directory,
                            capture_output=True, text=True, check=False)
    if result.returncode == 0:
        return None
    errors = [line for line in result.stderr.splitlines() if 'error' in line]
    return errors[0] if errors else result.stderr.strip().splitlines()[-1]


def include_folders(entry):
    """The folders an entry is told to include from, -I and -isystem, as whole paths."""
    folders = []
    arguments = arguments_of(entry)
    for index, argument in enumerate(arguments):
        for flag in ('-I', '-isystem'):
            if argument == flag and index + 1 < len(arguments):
                folders.append(arguments[index + 1])
            elif argument.startswith(flag) and argument != flag:
                folders.append(argument[len(flag):])
    return [(Path(entry['directory']) / folder).resolve() for folder in folders]


def platform_work(database):
    """(entry, headers) for each folder of platform/ the database compiles a source of: the
    entry of its first source, and the headers of the folder under that entry's include
    folders."""
    work = {}
    for entry in database:
        path = path_of(entry)
        parts = path.split('/') if path else []
        if len(parts) > 2 and parts[0] == 'platform' and parts[-1].endswith('.cpp'):
            work.setdefault(parts[1], entry)
    result = []
    for name, entry in sorted(work.items()):
        folders = include_folders(entry)
        # Not in a hidden folder, as the libraries PlatformIO fetches into .pio/: the board's,
        # not the project's.
        headers = sorted(header for header in (ROOT / 'platform' / name).rglob('*.h')
                         if not any(part.startswith('.') for part in
                                    header.relative_to(ROOT).parts)
                         and any(header.is_relative_to(folder) for folder in folders))
        if headers:
            result.append((entry, headers))
    return result


def check_headers(entry, headers, scratch, pool):
    """The headers that do not compile alone with the flags of `entry`, with their errors."""
    arguments = arguments_of(entry)
    results = pool.map(
        lambda header: (header, check(header, arguments, entry['directory'], scratch)), headers)
    return [(header, error) for header, error in results if error]


def check_database(build, headers):
    """The headers of src/ and of platform/ that do not compile alone with the flags of
    `build`, with their errors."""
    database = json.loads((build / 'compile_commands.json').read_text())
    work = [(library_entry(database), headers)] + platform_work(database)
    count = sum(len(headers) for _, headers in work)
    compiler = os.path.basename(arguments_of(work[0][0])[0])
    print(f'self_contained_headers.py: {count} headers by {compiler}, as {build}', flush=True)
    failures = []
    with tempfile.TemporaryDirectory() as scratch, \
            concurrent.futures.ThreadPoolExecutor(os.cpu_count()) as pool:
        for entry, headers_of_entry in work:
            failures += check_headers(entry, headers_of_entry, scratch, pool)
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__, fromfile_prefix_chars='@',
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('builds', nargs='+', type=Path,
                        help='the directories holding a compile_commands.json')
    arguments = parser.parse_args()

    headers = sorted((ROOT / 'src').rglob('*.h'))
    failures = []
    for build in arguments.builds:
        failures += [(build, header, error)
                     for header, error in check_database(build, headers)]
    for build, header, error in failures:
        print(f'{header.relative_to(ROOT)}: does not compile on its own, as {build}\n'
              f'    {error}')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
