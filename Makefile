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

# The flags CMakePresets.json gives the sanitizers preset, once more for the host suites, which
# are not CMake builds and take theirs through HOST_TESTS_CXXFLAGS.
SANITIZE := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
COVERAGE := --coverage -O0

# How long each fuzzer runs, and where the corpus it goes on from is kept.
FUZZ_SECONDS ?= 180
FUZZ_WORK ?= build-fuzz/work

HOST_SUITES := platform/esp32/.pio/host-tests platform/rp2350/.pio/host-tests

.PHONY: help check lint run clean \
        simulator clang avr esp32 rp2350 host-esp32 host-rp2350 web \
        tidy tidy-avr tidy-esp32 tidy-rp2350 cppcheck scripts docs web-lint \
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
check: lint simulator clang avr host-esp32 host-rp2350 web tidy tidy-avr \
       sanitizers coverage esp32 rp2350 tidy-esp32 tidy-rp2350 fuzz
lint: scripts docs cppcheck web-lint ## The checks that need no build: seconds

## Builds, each with its tests

simulator: ## The simulator and the core tests, into build/
	cmake --workflow --preset $(PRESET)simulator

run: ## Build the simulator and start it - the window, and the pages on localhost:8080
	cmake --preset $(PRESET)simulator
	cmake --build --preset $(PRESET)simulator --target Wordclock
	./build/bin/Wordclock

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
	cmake --preset $(PRESET)simulator
	cmake --build --preset $(PRESET)simulator --target Wordclock
	$(UV_RUN) python tools/web-smoke.py build/bin/Wordclock

## Analysers

# Configured, not built: clang-tidy compiles each translation unit itself and needs nothing from
# the build but compile_commands.json, which every preset writes. Which checks, and why the rest
# are off, is in .clang-tidy; WarningsAsErrors is set there, so a finding fails.
tidy: ## clang-tidy and the standalone headers: the core, the simulator and the host tests
	cmake --preset $(PRESET)simulator
# The one exception to "not built". WebPage.h holds the two pages as arrays and is generated
# rather than checked in, so a translation unit that includes it does not compile until the rule
# has run - and clang-tidy reports that as the file being missing, which reads like a bad include
# path rather than a build step that was skipped. It is a python script and costs no build time.
	cmake --build --preset $(PRESET)simulator --target WordclockWebPage
	run-clang-tidy -p build -quiet -j "$$(nproc)" '^$(CURDIR)/src/|platform/simulator/src'
# Every header compiled as the only include of an empty file, by the compiler and with the flags
# the database records. A header that leans on what its includers happen to include first builds
# everywhere until one of them stops - tools/self-contained-headers.py says why that is not a
# finding any analyser above can make. Each backend's target runs it again with its own compiler,
# because the #if branches a header takes are the target's.
	python3 tools/self-contained-headers.py build src platform/simulator/include
# The names are the same everywhere, the tests and the window included, which the run above
# leaves out: what the other checks would say about test scaffolding is not worth answering, but
# a test written in the old names reads as the old code. Seconds, not minutes - one check.
	run-clang-tidy -p build -quiet -j "$$(nproc)" -checks='-*,readability-identifier-naming' \
	    'platform/simulator/(tests|Wordclock)'
# The backends' host tests are compiled by their own run.sh and by nothing CMake or PlatformIO
# sees, so their names are read through the compile database run.sh writes. That takes python3
# and nothing else: no PlatformIO and no cross compiler, the stubs stand in for both.
	for backend in esp32 rp2350; do \
	    "platform/$$backend/test/run.sh" compiledb; \
	    run-clang-tidy -p "platform/$$backend/.pio/host-tests" -quiet -j "$$(nproc)" \
	        -checks='-*,readability-identifier-naming' 'platform/'; \
	done

# Configured, not built, as above. The preset names the toolchain file, which matters here: the
# database records whichever compiler project() found. The headers by avr-g++, which the
# cross-compiling clang-tidy needs installed anyway.
tidy-avr: ## clang-tidy and the standalone headers for the AVR backend
	cmake --preset $(PRESET)avr
	python3 tools/cross-clang-tidy.py build-avr '^$(CURDIR)/src/|platform/avr-dx/src'
	python3 tools/self-contained-headers.py build-avr src platform/avr-dx/include

# compiledb rather than a build: it fetches the platform, runs the pre-scripts - which is what
# writes WebPage.h - and writes the database, and compiles nothing. The headers by the board's own
# compiler out of the PlatformIO packages.
tidy-esp32: ## clang-tidy and the standalone headers for the ESP32 backend
	$(UV_RUN) pio run -d platform/esp32 -t compiledb
	python3 tools/cross-clang-tidy.py platform/esp32 '^$(CURDIR)/src/|platform/esp32/src|platform/arduino/src'
	python3 tools/self-contained-headers.py platform/esp32 src platform/esp32/include \
	    platform/arduino/include

tidy-rp2350: ## clang-tidy and the standalone headers for the RP2350 backend
	$(UV_RUN) pio run -d platform/rp2350 -t compiledb
	python3 tools/cross-clang-tidy.py platform/rp2350 '^$(CURDIR)/src/|platform/rp2350/src|platform/arduino/src'
	python3 tools/self-contained-headers.py platform/rp2350 src platform/rp2350/include \
	    platform/arduino/include

# No cmake, no wxWidgets, no compile database: cppcheck is given the include directories and
# works out the configurations itself, which for the firmware core is enough - it reports the
# same findings with and without compile_commands.json, and this way it still answers when the
# simulator does not build. src/ only, unlike tidy: without wxWidgets' own headers cppcheck
# stops at DECLARE_EVENT_TABLE in the simulator's window class and asks to be told what the macro
# is; the backend is the half clang-tidy covers through the compile database, and this is the
# half that needs no toolchain to read.
#
# --enable: warning and portability, not style or performance. Those two are where "could be
#   static", "could be const&" and "could return a reference" live - 270 of them, about how the
#   code is written rather than what it does, and the same argument .clang-tidy makes for leaving
#   the style checks out.
# --check-level=exhaustive costs nothing measurable at this size.
# --inline-suppr so a one-off can be answered next to the code, with the reason there, rather
#   than by widening .cppcheck-suppressions to a whole file.
#
# The include directories as an array, so each is one argument - a name with a blank included.
cppcheck: ## cppcheck over the firmware core
	mapfile -t includes < <(find src -type d | sed 's/^/-I/'); \
	cppcheck --enable=warning,portability --check-level=exhaustive --std=c++17 \
	    --inline-suppr --suppressions-list=.cppcheck-suppressions --error-exitcode=1 --quiet \
	    "$${includes[@]}" -Iplatform/simulator/include/sim -Iplatform/simulator/include \
	    src

# shellcheck over every .sh, ruff over every .py with the checks pyproject.toml selects, and
# actionlint over .github/workflows with what .github/actionlint.yaml adds - which runs
# shellcheck over every run: script as well, the one on PATH.
scripts: ## shellcheck, ruff and actionlint over the scripts and the workflows
	git ls-files -z '*.sh' | xargs -0 shellcheck
	$(UV_RUN) ruff check
	$(UV_RUN) actionlint

# Once per backend, over src/ and that backend: a \param that names no parameter, a command
# Doxygen does not know, an entity without a block or a block that leaves something out.
# docs/documentation.md says what a block holds. Then the indentation of a line that continues a
# command, which goes under the command's text.
docs: ## The Doxygen blocks, per backend, and their indentation
	python3 tools/docs.py
	python3 tools/doc-indent.py

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
	cmake --workflow --preset sanitizers
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/esp32/test/run.sh
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/rp2350/test/run.sh

# Into the job's summary as well when there is one, which is CI's.
coverage: ## Core tests and both host suites with --coverage, and the report
	cmake --workflow --preset coverage
	HOST_TESTS_CXXFLAGS="$(COVERAGE)" platform/esp32/test/run.sh
	HOST_TESTS_CXXFLAGS="$(COVERAGE)" platform/rp2350/test/run.sh
	python3 tools/coverage.py $(if $(GITHUB_STEP_SUMMARY),--summary "$(GITHUB_STEP_SUMMARY)") \
	    build-cov $(HOST_SUITES)

# The fuzzers' names here and not under tidy: they are in no build but this one.
fuzz: ## The fuzzers, FUZZ_SECONDS each, going on from the corpus in FUZZ_WORK
	cmake --workflow --preset fuzz
	run-clang-tidy -p build-fuzz -quiet -j "$$(nproc)" -checks='-*,readability-identifier-naming' \
	    'platform/simulator/tests/fuzz'
	platform/simulator/tests/fuzz/run.sh build-fuzz $(FUZZ_SECONDS) $(FUZZ_WORK)

## Housekeeping

clean: ## Every build directory, the PlatformIO images and the host suites' builds
	rm -rf build build-avr build-clang build-san build-cov build-fuzz \
	    platform/esp32/.pio/build platform/rp2350/.pio/build $(HOST_SUITES)
