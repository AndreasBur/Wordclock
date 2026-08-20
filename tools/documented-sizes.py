#!/usr/bin/env python3
"""Checks the sizes written in the documentation against the ones a build produces.

Every size in this repository was wrong at least once, and always the same way: the code
grew and the sentence did not. Correcting them by hand does not scale - two of them were
corrected and then made wrong again by the next change, within a day.

So this is a check rather than a rewriter by default. A stale number becomes a failing CI
job on the pull request that caused it, which is the only moment anybody can fix it
cheaply. `--fix` writes the measured values in, for when that moment has arrived.

What is checked is deliberately narrow: numbers that describe **the artefact as it is
now**. Numbers that describe a past change - "82 bytes of flash for the night switch" -
are history and must not be updated, and comparative claims are written as a delta
("worth 10 KiB") so that there is nothing exact to rot.

Usage:
    tools/documented-sizes.py --elf <path>   # check, exit 1 on a difference
    tools/documented-sizes.py --elf <path> --fix
"""

import argparse
import gzip
import importlib.util
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
AVR_FLASH_BYTES = 128 * 1024
AVR_RAM_BYTES = 16 * 1024


def avr_sizes(elf):
    """Program and data exactly as the link rule prints them - by asking the way it asks.

    --format=avr rather than adding up sections, which is how this was written first and was
    wrong by six bytes: Program is .text + .data + .bootloader, and the third of those is not
    something to reconstruct from the outside. The link rule in platform/avr-dx/CMakeLists.txt
    runs the same command, so what a reader sees after a build is what is compared here.
    """
    out = subprocess.run(['avr-size', '--format=avr', '--mcu=avr128da48', str(elf)],
                         capture_output=True, text=True, check=True).stdout
    program = re.search(r'Program:\s+(\d+) bytes', out)
    data = re.search(r'Data:\s+(\d+) bytes', out)

    if (program is None) or (data is None):
        raise SystemExit(f'documented-sizes.py: avr-size said something unexpected:\n{out}')

    return int(program.group(1)), int(data.group(1))


def page_sizes():
    """The console page as it is served: its source, and what it compresses to.

    Through embed_web.py's own comment stripper, so that "without comments" means here what
    it means in the build rather than something similar.
    """
    spec = importlib.util.spec_from_file_location('embed_web', ROOT / 'platform' / 'scripts' / 'embed_web.py')
    embed_web = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(embed_web)

    page = ROOT / 'web' / 'index.html'
    source = page.read_text(encoding='utf-8')
    compressed = gzip.compress(embed_web.strip_comments(source).encode('utf-8'), compresslevel=9, mtime=0)

    return len(source.encode('utf-8')), len(compressed)


def kilobytes(value):
    """The form the prose uses: one decimal, or none once it is past ten."""
    scaled = value / 1024

    return f'{scaled:.0f}' if scaled >= 10 else f'{scaled:.1f}'


def checks(elf):
    """Every documented size, with the pattern that finds it and the value it should be.

    A pattern must capture exactly what is to be replaced, and nothing that moves for
    another reason - which is why the percentages are captured beside their byte counts
    rather than found on their own.
    """
    found = []

    if elf is not None:
        program, data = avr_sizes(elf)
        found += [
            ('platform/avr-dx/README.md',
             r'Program:\s+(\d+) bytes \((\d+)%\)',
             (str(program), f'{round(100 * program / AVR_FLASH_BYTES)}')),
            ('platform/avr-dx/README.md',
             r'Data:\s+(\d+) bytes \((\d+)%\)',
             (str(data), f'{round(100 * data / AVR_RAM_BYTES)}')),
        ]

    source, compressed = page_sizes()
    found.append(('platform/esp32/README.md',
                  r'page is (\d+) KB of source and ([\d.]+) KB compressed',
                  (kilobytes(source), kilobytes(compressed))))

    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--elf', type=Path, help='an AVR image to measure; its checks are skipped without one')
    parser.add_argument('--fix', action='store_true', help='write the measured values into the documentation')
    arguments = parser.parse_args()

    stale = 0
    for name, pattern, expected in checks(arguments.elf):
        path = ROOT / name
        text = path.read_text(encoding='utf-8')
        match = re.search(pattern, text)

        if match is None:
            print(f'{name}: nothing matches /{pattern}/ - the sentence moved, so this check has to move with it')
            stale += 1
            continue

        if tuple(match.groups()) == expected:
            print(f'{name}: {" ".join(expected)} - as documented')
            continue

        stale += 1
        print(f'{name}: documented {" ".join(match.groups())}, measured {" ".join(expected)}')

        if arguments.fix:
            fixed = match.group(0)
            for was, now in zip(match.groups(), expected):
                fixed = fixed.replace(was, now, 1)
            path.write_text(text[:match.start()] + fixed + text[match.end():], encoding='utf-8')
            print(f'  written: {fixed}')

    if stale and not arguments.fix:
        print(f'\n{stale} documented size(s) no longer measure that way. tools/documented-sizes.py --fix writes them in.')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
