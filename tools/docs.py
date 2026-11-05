#!/usr/bin/env python3
"""Reads the Doxygen blocks of the firmware and of every backend, and fails on a warning.

Doxygen runs once per backend, over firmware/ and platform/<backend>: the backends each have a
Pixels, a System and a Storage of their own, which one run over all of them would merge into one
class. What the firmware's warnings say is the same in every run, so each is reported once.

Every warning fails but two kinds. A missing \\return: a one-line block whose \\brief says what
comes back, "Returns the width of the grid.", needs none, while every parameter needs its
\\param. And a missing block in a module of PENDING: those still have the blocks of before
docs/documentation.md, at the definitions in the .cpp, and are written anew one module at a time.
A module comes off the list in the pull request that documents it.

The reference is written into build/doxygen/<backend>/html, to be read there; it is not
published.

Usage:
    tools/docs.py
"""

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BACKENDS = ('simulator', 'avr-dx', 'esp32', 'rp2350')

# Where a member may still be undocumented, until its module is written in the new style.
PENDING = (
    'platform/avr-dx/',
    'platform/esp32/',
    'platform/rp2350/',
)

WARNING = re.compile(r'^(?P<path>[^:\s]+):\d+: warning: (?P<text>.*)$')
MISSING = re.compile(r'is not documented\.?$|^parameters of member .* are not documented$'
                     r'|^The following parameters? of .* (is|are) not documented:$')
RETURN = re.compile(r'^return type of member .* is not documented$')


def run(backend):
    """Returns the warnings of one backend's run, each with the lines that continue it."""
    out = ROOT / 'build' / 'doxygen' / backend
    out.mkdir(parents=True, exist_ok=True)
    log = out / 'warnings.log'
    config = (ROOT / 'Doxyfile').read_text() + (
        f'\nINPUT = firmware platform/{backend}\n'
        f'OUTPUT_DIRECTORY = {out}\n'
        f'WARN_LOGFILE = {log}\n')
    subprocess.run(['doxygen', '-'], input=config, text=True, cwd=ROOT, check=True)
    warnings = []
    for line in log.read_text().splitlines():
        line = line.replace(f'{ROOT}/', '')
        if WARNING.match(line) or not warnings:
            warnings.append(line)
        else:
            warnings[-1] += '\n' + line
    return warnings


def passes(warning):
    match = WARNING.match(warning.split('\n', 1)[0])
    if not match:
        return False
    if RETURN.search(match['text']):
        return True
    return bool(MISSING.search(match['text'])) and match['path'].startswith(PENDING)


def main():
    seen = []
    for backend in BACKENDS:
        seen += [warning for warning in run(backend) if warning not in seen]
    failing = [warning for warning in seen if not passes(warning)]
    for warning in failing:
        print(warning)
    print(f'docs.py: {len(failing)} warnings, {len(seen) - len(failing)} blocks still to be written'
          ' or returns of one-line blocks')
    return 1 if failing else 0


if __name__ == '__main__':
    sys.exit(main())
