#!/usr/bin/env python3
"""Loads the two pages a clock serves in a real browser, against the simulator, and checks them.

The panel at / and the console at /console were checked by hand until now, by somebody running
Playwright the way CLAUDE.md describes. The bugs that turned up that way were found by measuring
rather than by looking - a card wider than the phone, a page that scrolls sideways at 390 px - so
what is measured there is measured here, on every pull request:

- each page at a phone's width and at a desktop's, the one in the dark scheme and the other in
  the light one, loads, receives its first frame and logs no error to the console;
- nothing on it is wider than the window, which is a page that scrolls sideways;
- on the panel, the plate shows lit letters - /display answered and the socket delivered a frame;
- on the console, a command typed into the folded "Commands" section and sent comes back
  answered: status command 12, whose answer carries V=, so the whole way from a browser through
  the firmware and back has been taken.

The simulator is the firmware behind the pages. It is started here, on an X server of its own
through xvfb-run, so that its window opens nowhere a person would see it, and in a directory of
its own, because it keeps its settings in a file in the working directory. It is stopped again
whatever the outcome.

Run with a Python that has Playwright and its Chromium - the dev container's /opt/playwright:

    /opt/playwright/bin/python tools/web-smoke.py build/bin/Wordclock
"""

import argparse
import os
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path

from playwright.sync_api import sync_playwright

URL = 'http://localhost:8080'
START_SECONDS = 30
# The first load after the server starts can miss even a long wait on the selector, and every
# load after it be instant - CLAUDE.md's note on driving the pages. One retry, not a loop.
LOAD_ATTEMPTS = 2
# Every field fills from the catalog and a round of answers over the socket after the plate has
# appeared, so a check made the moment it appears reads defaults.
SETTLE_MS = 2000

PAGES = (
    ('panel', '/', '#plate:not([hidden])'),
    ('console', '/console', '#panel:not([hidden])'),
)
VIEWS = (
    ('390 px, dark', {'width': 390, 'height': 900}, 'dark'),
    ('1280 px, light', {'width': 1280, 'height': 900}, 'light'),
)

# What sticks out past the right edge of the window, by its id or its tag and class - the element
# to look at, rather than only the fact that the page scrolls.
OVERFLOWING = '''() => {
    const width = document.documentElement.clientWidth;
    const found = [];
    for (const element of document.querySelectorAll('body *')) {
        const box = element.getBoundingClientRect();
        if (box.width > 0 && box.right > width + 1) {
            found.push(element.id ? '#' + element.id
                                  : element.tagName.toLowerCase() + (element.className ? '.' + element.className : ''));
        }
    }
    return {scrolls: document.documentElement.scrollWidth > width, elements: found.slice(0, 8)};
}'''


def wait_for_server(process):
    deadline = time.monotonic() + START_SECONDS
    while time.monotonic() < deadline:
        if process.poll() is not None:
            return False
        try:
            with urllib.request.urlopen(URL + '/', timeout=2):
                return True
        except OSError:
            time.sleep(0.5)
    return False


def start_simulator(binary, directory):
    """The simulator under an X server of its own, in a session of its own so it can be stopped
    whole: xvfb-run, the X server and the simulator. GDK_BACKEND because the WSLg containers set
    WAYLAND_DISPLAY, and GTK would rather draw there than on the X server it was given."""
    xvfb_run = shutil.which('xvfb-run')
    if xvfb_run is None:
        raise SystemExit('web-smoke.py: xvfb-run is not installed (Debian and Ubuntu: xvfb)')
    environment = dict(os.environ, GDK_BACKEND='x11')
    environment.pop('WAYLAND_DISPLAY', None)
    log = open(directory / 'simulator.log', 'w')
    return subprocess.Popen([xvfb_run, '-a', str(binary)], cwd=directory, env=environment,
                            stdout=log, stderr=subprocess.STDOUT, start_new_session=True)


def stop(process):
    if process.poll() is None:
        os.killpg(process.pid, signal.SIGTERM)
        try:
            process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)


def load(page, path, ready):
    for attempt in range(LOAD_ATTEMPTS):
        try:
            # Never networkidle: both pages hold a web socket open, so neither ever goes idle.
            page.goto(URL + path, wait_until='load')
            page.wait_for_selector(ready, timeout=20000)
            return True
        except Exception:
            if attempt + 1 == LOAD_ATTEMPTS:
                return False
    return False


def check_page(browser, name, path, ready, view, viewport, scheme):
    """The findings on one page in one view, as lines; none means it passed."""
    findings = []
    page = browser.new_page(viewport=viewport, color_scheme=scheme)
    errors = []
    page.on('console', lambda message: errors.append(message.text) if message.type == 'error' else None)
    page.on('pageerror', lambda error: errors.append(str(error)))

    if not load(page, path, ready):
        page.close()
        return [f'{name}, {view}: {ready} never appeared']
    page.wait_for_timeout(SETTLE_MS)

    overflow = page.evaluate(OVERFLOWING)
    if overflow['scrolls']:
        findings.append(f'{name}, {view}: scrolls sideways; past the edge: {", ".join(overflow["elements"])}')

    if name == 'panel' and page.locator('#plate .lit').count() == 0:
        findings.append(f'{name}, {view}: the plate shows no lit letter')

    if name == 'console':
        # The command line sits in the folded "Commands" section, opened the way a reader would.
        try:
            if not page.eval_on_selector('#consoleBox', 'box => box.open'):
                page.click('#consoleBox > summary')
            page.fill('#command', '12', timeout=5000)
            page.press('#command', 'Enter')
            page.wait_for_function("document.getElementById('log').textContent.includes('V=')", timeout=5000)
        except Exception as error:
            findings.append(f'{name}, {view}: status command 12 got no answer in the log ({error.__class__.__name__})')

    findings += [f'{name}, {view}: console error: {text}' for text in errors]
    page.close()
    return findings


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('simulator', type=Path, help='the simulator binary, build/bin/Wordclock')
    args = parser.parse_args()
    binary = args.simulator.resolve()
    if not binary.is_file():
        raise SystemExit(f'web-smoke.py: no simulator at {binary} - build it first')

    with tempfile.TemporaryDirectory(prefix='web-smoke-') as scratch:
        directory = Path(scratch)
        process = start_simulator(binary, directory)
        try:
            if not wait_for_server(process):
                print((directory / 'simulator.log').read_text(), file=sys.stderr)
                raise SystemExit(f'web-smoke.py: the simulator did not answer on {URL} within {START_SECONDS} s')

            findings = []
            with sync_playwright() as playwright:
                browser = playwright.chromium.launch()
                for name, path, ready in PAGES:
                    for view, viewport, scheme in VIEWS:
                        found = check_page(browser, name, path, ready, view, viewport, scheme)
                        print(f'{name:8s} {view:15s} {"ok" if not found else "FAIL"}')
                        findings += found
                browser.close()
        finally:
            stop(process)

    if findings:
        print('\n' + '\n'.join(findings), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
