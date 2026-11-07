#!/usr/bin/env python3
"""Reports which lines of the firmware the tests run, from what gcov left behind.

The firmware is tested by three suites that build it three ways - the core tests through the
simulator's CMake build, and each networked backend's host tests through its own run.sh -
and none of them alone says much: the core tests never reach the command line, which the host
suites drive through the web socket, and the host suites leave out what only the core tests
check. So the report is of all of them at once. A line counts as run if any suite ran it.

Each suite is built with --coverage and run first, which leaves a .gcda beside every object:

    cmake --workflow --preset coverage
    HOST_TESTS_CXXFLAGS="--coverage -O0" platform/esp32/test/run.sh
    HOST_TESTS_CXXFLAGS="--coverage -O0" platform/rp2350/test/run.sh
    tools/coverage.py build-cov platform/esp32/.pio/host-tests platform/rp2350/.pio/host-tests

-O0 because an optimised build folds lines together and inlines functions into their callers,
which leaves gcov's count of a line hard to read; at -O0 a line is the code written on it.

It reports and does not judge: there is no threshold, and it fails only when it found no data at
all, which means a suite was built without --coverage. A threshold would be a number to keep
above rather than a picture of where the tests are thin; the files at the top of the table are
the point. Only firmware/ is reported. The backends, the simulator and the stubs are what the
tests run on, not what they are for.
"""

import argparse
import collections
import concurrent.futures
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FIRMWARE = ROOT / 'firmware'


def data_files(directories):
    """Every .gcda under the directories. Walked by hand rather than globbed: the host suites'
    object cache is under .pio/, and a glob passes over a directory whose name starts with a
    dot."""
    for directory in directories:
        for path, _, names in os.walk(directory):
            for name in names:
                if name.endswith('.gcda'):
                    yield Path(path, name).resolve()


def lines_of(gcov, data_file):
    """{source: {line: count}} for one .gcda, as gcov's JSON gives it. A .gcda left over from
    an earlier build of a different object makes gcov complain on stderr and give nothing; that
    is a stale file and not a finding, so it is not reported."""
    result = subprocess.run([gcov, '--json-format', '--stdout', str(data_file)],
                            capture_output=True, text=True, cwd=ROOT, check=False)
    found = collections.defaultdict(dict)
    for document in result.stdout.splitlines():
        if not document.startswith('{'):
            continue
        report = json.loads(document)
        directory = report.get('current_working_directory', '')
        for source in report['files']:
            path = Path(os.path.normpath(os.path.join(directory, source['file'])))
            for line in source['lines']:
                found[path][line['line_number']] = line['count']
    return found


def merge(reports):
    """The highest count of each line over every suite and every object that compiled it. A
    header's inline function is compiled into many objects, and run from only some of them."""
    merged = collections.defaultdict(dict)
    for report in reports:
        for path, lines in report.items():
            if not path.is_relative_to(FIRMWARE):
                continue
            for number, count in lines.items():
                merged[path][number] = max(merged[path].get(number, 0), count)
    return merged


def rows_of(merged):
    """(path, lines run, lines) per file, the least covered first - the gaps are what a reader
    of the report is looking for - and alphabetical among equals."""
    rows = []
    for path, lines in merged.items():
        if lines:
            run = sum(1 for count in lines.values() if count > 0)
            rows.append((path.relative_to(ROOT).as_posix(), run, len(lines)))
    return sorted(rows, key=lambda row: (row[1] / row[2], row[0]))


def percent(run, total):
    return f'{100.0 * run / total:5.1f} %'


def write_text(rows, total_run, total, out):
    for path, run, lines in rows:
        print(f'{percent(run, lines)} {run:5d}/{lines:<5d} {path}', file=out)
    print(f'{percent(total_run, total)} {total_run:5d}/{total:<5d} firmware/ in all', file=out)


def write_markdown(rows, total_run, total, suites, out):
    print('## Line coverage of firmware/', file=out)
    print('', file=out)
    print(f'**{percent(total_run, total).strip()}**, {total_run} of {total} lines, over '
          f'{", ".join(suites)}. A line counts as run if any of them ran it.', file=out)
    print('', file=out)
    print('<details><summary>Per file, the least covered first</summary>', file=out)
    print('', file=out)
    print('| Covered | Lines | File |', file=out)
    print('|---:|---:|---|', file=out)
    for path, run, lines in rows:
        print(f'| {percent(run, lines).strip()} | {run}/{lines} | `{path}` |', file=out)
    print('', file=out)
    print('</details>', file=out)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('directories', nargs='+', type=Path,
                        help='where the suites left their .gcda files, one per suite')
    parser.add_argument('--gcov', default='gcov',
                        help='the gcov of the compiler the suites were built with')
    parser.add_argument('--summary', type=Path,
                        help='append the report as Markdown to this file as well, '
                             'such as $GITHUB_STEP_SUMMARY')
    arguments = parser.parse_args()

    files = list(data_files(arguments.directories))
    if not files:
        sys.exit('coverage.py: no .gcda found - were the suites built with --coverage and run?')

    with concurrent.futures.ThreadPoolExecutor() as pool:
        merged = merge(pool.map(lambda data_file: lines_of(arguments.gcov, data_file), files))

    rows = rows_of(merged)
    if not rows:
        sys.exit('coverage.py: the .gcda files name no source under firmware/')
    total_run = sum(row[1] for row in rows)
    total = sum(row[2] for row in rows)

    write_text(rows, total_run, total, sys.stdout)
    if arguments.summary:
        suites = [directory.as_posix() for directory in arguments.directories]
        with open(arguments.summary, 'a', encoding='utf-8') as summary:
            write_markdown(rows, total_run, total, suites, summary)


if __name__ == '__main__':
    main()
