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

And what is checked is checked at a granularity the number actually has. The AVR image is
48 346 bytes here and 48 152 in CI, for the same source: the byte count is a property of the
compiler in front of you, not of this repository, and the first version of this script pinned
it and duly failed on CI. So the flash figure is compared as a *percentage*, which both
toolchains agree on and which is also the question somebody is asking of it - how close to
the ceiling this part is. The page's sizes stay exact, because nothing but the repository's
own bytes decides them.

Usage:
    tools/documented-sizes.py --elf <path>   # check, exit 1 on a difference
    tools/documented-sizes.py --elf <path> --fix
"""

import argparse
import gzip
import importlib.util
import os
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


# What the Pico's sketch has after the filesystem took its megabyte, and the part's RAM. Both
# are what the linker was given rather than what the chip has: the flash region shrank when the
# network update needed room for an image, and a percentage of the whole part would flatter it.
RP2350_FLASH_BYTES = 3141632
RP2350_RAM_BYTES = 512 * 1024


def rp2350_sizes(elf):
    """Flash and RAM as the sections say, which is the only reading this script can defend.

    Not PlatformIO's own two numbers: those come out of its build, this runs on an image, and
    the two disagree by a few hundred bytes for reasons that belong to the builder. So what the
    notes carry is a percentage of each region, measured here, with a point of slack - the same
    shape the AVR's figure has and for the same reason.

    Flash is every section the linker put at an XIP address, RAM the three that live in it.
    """
    size = os.environ.get('RP2350_SIZE') or _find_pico_size()
    out = subprocess.run([size, '-A', str(elf)], capture_output=True, text=True, check=True).stdout

    flash = 0
    ram = 0
    for line in out.splitlines():
        parts = line.split()
        if len(parts) != 3:
            continue
        name, length, address = parts[0], parts[1], parts[2]
        if not length.isdigit() or not address.isdigit():
            continue
        if int(address) >= 0x10000000 and int(address) < 0x20000000:
            flash += int(length)
        elif name in ('.ram_vector_table', '.data', '.bss'):
            ram += int(length)
    return flash, ram


def _find_pico_size():
    """The Pico toolchain's size, wherever PlatformIO put it."""
    candidates = sorted(Path.home().glob('.platformio/packages/toolchain-rp2040-*/bin/arm-none-eabi-size'))
    if not candidates:
        raise SystemExit('documented-sizes.py: no arm-none-eabi-size found; set RP2350_SIZE')
    return str(candidates[-1])


def page_sizes(name):
    """One page as it is served: its source, and what it compresses to.

    Through embed_web.py's own comment stripper, so that "without comments" means here what
    it means in the build rather than something similar.

    Named rather than fixed, because a clock serves two pages now - the one at "/" and the
    console behind it - and a sentence about the flash they cost has to be about both.
    """
    spec = importlib.util.spec_from_file_location('embed_web', ROOT / 'platform' / 'scripts' / 'embed_web.py')
    embed_web = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(embed_web)

    page = ROOT / 'web' / name
    source = page.read_text(encoding='utf-8')
    compressed = gzip.compress(embed_web.strip_comments(source).encode('utf-8'), compresslevel=9, mtime=0)

    return len(source.encode('utf-8')), len(compressed)


def kilobytes(value):
    """The form the prose uses: one decimal, or none once it is past ten."""
    scaled = value / 1024

    return f'{scaled:.0f}' if scaled >= 10 else f'{scaled:.1f}'


class Check:
    """One documented size: where it is written, how to find it, and what it should say.

    `tolerance` is how far the written value may sit from the measured one before it counts
    as stale. A percentage rounded from two toolchains that differ by 0.15 points will agree
    almost always and not quite always, and a check that flaps is a check people learn to
    ignore - so one point of slack is allowed on purpose rather than discovered later.

    `enforce` is what separates a number a build must match from one that is only refreshed.
    The sample of the link rule's output is the second kind: it is real output from whichever
    toolchain ran --fix, so it cannot be enforced on another one - but leaving it to rot is
    what this script exists to stop, so --fix rewrites it and --check leaves it alone.
    """

    def __init__(self, document, pattern, expected, tolerance=(), enforce=True):
        self.document = document
        self.pattern = pattern
        self.expected = expected
        self.tolerance = tolerance or tuple(0 for _ in expected)
        self.enforce = enforce

    def matches(self, written):
        for was, now, slack in zip(written, self.expected, self.tolerance):
            if was == now:
                continue
            if (slack > 0) and abs(float(was) - float(now)) <= slack:
                continue
            return False
        return True


def checks(elf, rp2350Elf=None):
    """Every documented size, with the pattern that finds it and the value it should be.

    A pattern must capture exactly what is to be replaced, and nothing that moves for
    another reason - which is why the percentages are captured beside their byte counts
    rather than found on their own.
    """
    found = []

    if elf is not None:
        program, data = avr_sizes(elf)
        found.append(Check('platform/avr-dx/README.md',
                           r'\*\*(\d+) % of the flash and (\d+) % of the RAM\*\*',
                           (f'{round(100 * program / AVR_FLASH_BYTES)}',
                            f'{round(100 * data / AVR_RAM_BYTES)}'),
                           tolerance=(1, 1)))
        found.append(Check('platform/avr-dx/README.md',
                           r'Program:\s+(\d+) bytes \((\d+)%\)\nData:\s+(\d+) bytes \((\d+)%\)',
                           (str(program), f'{round(100 * program / AVR_FLASH_BYTES)}',
                            str(data), f'{round(100 * data / AVR_RAM_BYTES)}'),
                           enforce=False))

    if rp2350Elf is not None:
        flash, ram = rp2350_sizes(rp2350Elf)
        found.append(Check('platform/rp2350/README.md',
                           r'RAM:\s+(\d+) %\s+of 512 KB\nFlash:\s+(\d+) %\s+of the 3 MB',
                           (f'{round(100 * ram / RP2350_RAM_BYTES)}',
                            f'{round(100 * flash / RP2350_FLASH_BYTES)}'),
                           tolerance=(1, 1)))

    appSource, appCompressed = page_sizes('app.html')
    consoleSource, consoleCompressed = page_sizes('index.html')
    found.append(Check('platform/esp32/README.md',
                       r'panel is (\d+) KB of source and ([\d.]+) KB compressed, the console '
                       r'(\d+) and ([\d.]+)',
                       (kilobytes(appSource), kilobytes(appCompressed),
                        kilobytes(consoleSource), kilobytes(consoleCompressed))))

    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--elf', type=Path, help='an AVR image to measure; its checks are skipped without one')
    parser.add_argument('--rp2350-elf', type=Path, dest='rp2350Elf',
                        help='an RP2350 image to measure; its checks are skipped without one')
    parser.add_argument('--fix', action='store_true', help='write the measured values into the documentation')
    arguments = parser.parse_args()

    stale = 0
    for check in checks(arguments.elf, arguments.rp2350Elf):
        path = ROOT / check.document
        text = path.read_text(encoding='utf-8')
        match = re.search(check.pattern, text)
        label = check.document if check.enforce else f'{check.document} (sample)'

        if match is None:
            print(f'{label}: nothing matches /{check.pattern}/ - the sentence moved, so this check has to move with it')
            stale += 1
            continue

        if check.matches(match.groups()):
            print(f'{label}: {" ".join(match.groups())} - as documented')
            continue

        if check.enforce:
            stale += 1
        print(f'{label}: documented {" ".join(match.groups())}, measured {" ".join(check.expected)}')

        if arguments.fix:
            fixed = match.group(0)
            for was, now in zip(match.groups(), check.expected):
                fixed = fixed.replace(was, now, 1)
            path.write_text(text[:match.start()] + fixed + text[match.end():], encoding='utf-8')
            print(f'  written: {fixed.splitlines()[0]}')

    if stale and not arguments.fix:
        print(f'\n{stale} documented size(s) no longer measure that way. tools/documented-sizes.py --fix writes them in.')
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
