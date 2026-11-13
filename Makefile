# Wordclock - the index of what builds and checks this repository.
#
# Nothing is decided here: the CMake builds are presets in CMakePresets.json, the Python tools
# are pinned in uv.lock, and each check is a script under tools/ or a backend's run.sh. This
# file only names them, so that a developer and CI type the same thing - every step of the two
# workflows that builds or checks something is one of these targets. `make` alone lists them.
#
# Warnings are errors in CI and under `make check`, and not otherwise: the ci- presets, chosen
# by STRICT. GitHub Actions sets CI=true in every job, which is where the default comes from.
#
# The tools uv.lock pins - PlatformIO, Playwright, ruff, actionlint - run through `uv run`, which
# finds them in .venv: the dev container links it to the image's environment, and CI's uv-sync
# action creates it; anywhere else the first `uv run` does. --frozen so that a run never relocks,
# which is the business of the pull request that changes pyproject.toml. Everything else is
# expected on PATH, where the dev container and the workflows' apt-install put it.

SHELL := bash
.SHELLFLAGS := -e -o pipefail -c
.DEFAULT_GOAL := help
# One tool at a time: several of these configure the same build directory, and each of them
# runs its compiler or analyser on every core already.
.NOTPARALLEL:

STRICT ?= $(CI)
PRESET = $(if $(STRICT),ci-)
UV_RUN ?= uv run --frozen

# The flags cmake/checks.cmake gives the sanitize preset, once more for the host suites, which
# are not CMake builds and take theirs through HOST_TESTS_CXXFLAGS.
SANITIZE := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

# How long each fuzzer runs; tools/fuzz.sh reads it, and keeps what each found in build/fuzz/work.
export FUZZ_SECONDS ?= 180

HOST_SUITES := platform/esp32/.pio/host-tests platform/rp2350/.pio/host-tests

.PHONY: help check lint run clean \
        simulator clang avr esp32 rp2350 host-esp32 host-rp2350 web \
        tidy headers cppcheck style fmt docs web-lint \
        sanitizers coverage fuzz

help: ## This list
	@awk 'BEGIN { FS = ":.*## " } /^## / { printf "\n%s\n", substr($$0, 4) } \
	      /^[a-z0-9-]+:.*## / { printf "  make %-12s %s\n", $$1, $$2 }' $(MAKEFILE_LIST)
	@echo
	@echo '  STRICT=1 for warnings as errors, FUZZ_SECONDS=n for a shorter fuzzing run.'

## Everything at once

# The order is cheap first, so that a typo fails in seconds rather than after the fuzzers.
check: STRICT := 1
check: ## What CI runs, all of it
check: lint simulator clang avr host-esp32 host-rp2350 web sanitizers coverage \
       esp32 rp2350 tidy headers fuzz
lint: style docs cppcheck web-lint ## The checks that need no build: seconds

## Builds, each with its tests

simulator: ## The simulator and the core tests, into build/host/
	cmake --workflow --preset $(PRESET)default

run: ## Build the simulator and start it - the window, and the pages on localhost:8080
	cmake --preset $(PRESET)default
	cmake --build --preset $(PRESET)default --target Wordclock
	./build/host/bin/Wordclock

clang: ## The simulator, core tests and both host suites compiled by clang
	cmake --workflow --preset $(PRESET)clang
	CXX=clang++ platform/esp32/test/run.sh
	CXX=clang++ platform/rp2350/test/run.sh

# Not the workflow preset: --verbose, so the log shows the paths the stack is made of even when
# it fits, and the size the build printed sits next to the one the documentation states.
avr: ## The AVR128DA48 image, its documented sizes and its stack
	cmake --preset $(PRESET)avr
	cmake --build --preset $(PRESET)avr
	ctest --preset $(PRESET)avr --verbose

esp32: ## The ESP32-S3 image and its documented sizes
	$(UV_RUN) pio run -d platform/esp32
	python3 tools/documented-sizes.py \
	    --esp32-bin platform/esp32/.pio/build/wordclock/firmware.bin \
	    --esp32-elf platform/esp32/.pio/build/wordclock/firmware.elf

rp2350: ## The RP2350 image and its documented sizes
	$(UV_RUN) pio run -d platform/rp2350
	python3 tools/documented-sizes.py --rp2350-elf platform/rp2350/.pio/build/wordclock/firmware.elf

host-esp32: ## The ESP32 backend's host tests, with g++
	platform/esp32/test/run.sh

host-rp2350: ## The RP2350 backend's host tests, with g++
	platform/rp2350/test/run.sh

web: ## Both web pages in headless Chromium, against the simulator
	cmake --preset $(PRESET)default
	cmake --build --preset $(PRESET)default --target Wordclock
	$(UV_RUN) python tools/web-smoke.py build/host/bin/Wordclock

## Analysers

# The template's tools/tidy.sh: each target of tools/project.sh with its own flags - the AVR's
# database from CMake, the ESP32's and the RP2350's from PlatformIO - then the tests and the
# simulator, then the fuzzers. Configured, not built: clang-tidy compiles each translation unit
# itself. Which checks, and why the rest are off, is in .clang-tidy; WarningsAsErrors is set
# there, so a finding fails.
tidy: ## clang-tidy: every target, the tests, the fuzzers and the host suites' names
# The one exception to "not built". WebPage.h holds the two pages as arrays and is generated
# rather than checked in, so a translation unit that includes it does not compile until the rule
# has run - and clang-tidy reports that as the file being missing, which reads like a bad include
# path rather than a build step that was skipped. It is a python script and costs no build time.
	cmake --preset default
	cmake --build --preset default --target WordclockWebPage
	tools/tidy.sh
# The backends' host tests are compiled by their own run.sh and by nothing CMake or PlatformIO
# sees - TIDY_SKIP in tools/project.sh - so their names are read through the compile database
# run.sh writes. That takes python3 and nothing else: the stubs stand in for the frameworks.
# The names alone, so -Wno-error: run.sh compiles with -Werror, and the warnings .clang-tidy adds
# with ExtraArgs would otherwise each stop the run as an error no check list switches off.
	for backend in esp32 rp2350; do \
	    "platform/$$backend/test/run.sh" compiledb; \
	    $(UV_RUN) run-clang-tidy.py -p "platform/$$backend/.pio/host-tests" -quiet \
	        -j "$$(nproc)" -checks='-*,readability-identifier-naming' -extra-arg=-Wno-error \
	        'platform/'; \
	done

# Every header of src/ and platform/ compiled as the only include of an empty file, by each
# target's compiler and the host's, with the flags its database records:
# tools/self_contained_headers.py says why that is a finding no analyser above can make. A header
# of platform/<name>/ is compiled by the targets that compile that folder.
headers: ## Every header compiled on its own, for the host and for each target
	cmake --preset default >/dev/null
	cmake --build --preset default --target WordclockWebPage
	tools/tidy.sh --database-only >/dev/null
	python3 tools/self_contained_headers.py build/host @build/tidy/databases

# The template's tools/cppcheck.sh. No cmake, no wxWidgets, no compile database: cppcheck is
# given the include folders and works out the configurations itself - each module's folder, the
# simulator's stand-ins for the Arduino core and .cppcheck-suppressions, which cppcheck_flags() of
# tools/project.sh adds. src/ only, unlike tidy: without wxWidgets' own headers cppcheck stops at
# DECLARE_EVENT_TABLE in the simulator's window class; the backend is the half clang-tidy covers
# through the compile database, and this is the half that needs no toolchain to read.
#
# --enable=warning,portability, not style or performance: those two are where "could be static",
# "could be const&" and "could return a reference" live - 270 of them, about how the code is
# written rather than what it does. --inline-suppr, so a one-off is answered next to the code.
cppcheck: ## cppcheck over the firmware core
	tools/cppcheck.sh

# The template's tools/lint.sh: clang-format and the indentation of the Doxygen blocks, cpplint
# with what CPPLINT.cfg filters, shellcheck over tools/, ruff over every .py with the checks
# pyproject.toml selects, and actionlint over the workflows. Then shellcheck over the scripts
# outside tools/, which tools/lint.sh does not reach: the host suites' run.sh above all.
style: ## clang-format, cpplint, shellcheck, ruff and actionlint: tools/lint.sh
	tools/lint.sh
	git ls-files -z '*.sh' ':!tools/*' | xargs -0 $(UV_RUN) shellcheck

fmt: ## Format the C++ sources and the Doxygen blocks in place
	tools/lint.sh --fix

# Once per backend, over src/ and that backend: a \param that names no parameter, a command
# Doxygen does not know, an entity without a block or a block that leaves something out.
# docs/documentation.md says what a block holds.
docs: ## The Doxygen blocks, per backend
	python3 tools/docs.py

# html-validate over both pages as whole documents, and eslint over the scripts inline in them;
# what each checks and leaves out is in .htmlvalidate.mjs and eslint.config.mjs. The packages
# are the ones package-lock.json pins, installed by npm ci only when the lock has changed since:
# node_modules/.package-lock.json is the last thing an install writes.
web-lint: node_modules/.package-lock.json ## html-validate and eslint over the two web pages
	node_modules/.bin/html-validate web/app.html web/index.html
	node_modules/.bin/eslint web/

node_modules/.package-lock.json: package.json package-lock.json
	npm ci

## The three suites, instrumented

sanitizers: ## Core tests and both host suites under ASan and UBSan
	cmake --workflow --preset sanitize
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/esp32/test/run.sh
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/rp2350/test/run.sh

# The host suites through coverage_suites() of tools/project.sh, counted with the core tests
# against COVERAGE_MINIMUM. Into the job's summary as well when there is one, which is CI's.
coverage: ## Core tests and both host suites with --coverage, against COVERAGE_MINIMUM
	tools/coverage.sh
	if [[ -n "$${GITHUB_STEP_SUMMARY:-}" ]]; then \
	    { echo '```text'; cat build/coverage/coverage.txt; echo '```'; } >> "$$GITHUB_STEP_SUMMARY"; \
	fi

# The fuzzers' names here and not under tidy: they are in no build but this one.
fuzz: ## The fuzzers, FUZZ_SECONDS each, going on from the corpus in build/fuzz/work
	cmake --workflow --preset fuzz
	run-clang-tidy -p build/fuzz -quiet -j "$$(nproc)" -checks='-*,readability-identifier-naming' \
	    'test/fuzz'
	tools/fuzz.sh

## Housekeeping

clean: ## Every build directory, the PlatformIO images and the host suites' builds
	rm -rf build build-avr \
	    platform/esp32/.pio/build platform/rp2350/.pio/build $(HOST_SUITES)
