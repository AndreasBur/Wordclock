# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Wordclock's own targets, which the Makefile includes; the template wrote this file once, and
# `copier update` leaves it alone. A target with a `## ` comment is listed by `make`, and one
# added to PROJECT_CHECKS is part of `make check`, after the template's own.
#
# What the template's targets do not reach: the simulator's window and its two web pages, the
# backends' host suites - built by each backend's test/run.sh against stand-ins of its framework,
# by g++, by clang and under the sanitizers - their names, the web pages' linters, and the sizes
# the documentation states. And each backend's image on its own, as CI's firmware jobs build and
# keep them: `make examples` builds all three at once.

# Three minutes a fuzzer in CI, where the template gives one: the corpus a run leaves in the cache
# is what the next goes on from. Set here, after the template's default and before its rules; a
# FUZZ_SECONDS on the command line wins over both.
FUZZ_SECONDS = 180

# The flags cmake/checks.cmake gives the sanitize preset, once more for the host suites, which
# are not CMake builds and take theirs through HOST_TESTS_CXXFLAGS.
SANITIZE := -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer

HOST_SUITES := platform/esp32/.pio/host-tests platform/rp2350/.pio/host-tests

# Cheap first, as the template's check orders its own.
PROJECT_CHECKS += web-lint host-esp32 host-rp2350 host-clang host-sanitize tidy-host-suites \
                  web documented-sizes

.PHONY: run web web-lint host-esp32 host-rp2350 host-clang host-sanitize tidy-host-suites \
        documented-sizes avr esp32 rp2350 clean-all

## Wordclock's own

run: ## Build the simulator and start it - the window, and the pages on localhost:8080
	cmake --preset $(PRESET)default
	cmake --build --preset $(PRESET)default --target Wordclock
	./build/host/bin/Wordclock

web: ## Both web pages in headless Chromium, against the simulator
	cmake --preset $(PRESET)default
	cmake --build --preset $(PRESET)default --target Wordclock
	$(UV_RUN) python tools/web-smoke.py build/host/bin/Wordclock

# html-validate over both pages as whole documents, and eslint over the scripts inline in them;
# what each checks and leaves out is in .htmlvalidate.mjs and eslint.config.mjs. The packages
# are the ones package-lock.json pins, installed by npm ci only when the lock has changed since:
# node_modules/.package-lock.json is the last thing an install writes.
web-lint: node_modules/.package-lock.json ## html-validate and eslint over the two web pages
	node_modules/.bin/html-validate web/app.html web/index.html
	node_modules/.bin/eslint web/

node_modules/.package-lock.json: package.json package-lock.json
	npm ci

host-esp32: ## The ESP32 backend's host tests, with g++
	platform/esp32/test/run.sh

host-rp2350: ## The RP2350 backend's host tests, with g++
	platform/rp2350/test/run.sh

host-clang: ## Both backends' host tests compiled by clang
	CXX=clang++ platform/esp32/test/run.sh
	CXX=clang++ platform/rp2350/test/run.sh

host-sanitize: ## Both backends' host tests under ASan and UBSan
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/esp32/test/run.sh
	UBSAN_OPTIONS=print_stacktrace=1 HOST_TESTS_CXXFLAGS="$(SANITIZE)" platform/rp2350/test/run.sh

# The backends' host tests are compiled by their own run.sh and by nothing CMake or PlatformIO
# sees - TIDY_SKIP in tools/project.sh - so their names are read through the compile database
# run.sh writes. That takes python3 and nothing else: the stubs stand in for the frameworks.
# The names alone, so -Wno-error: run.sh compiles with -Werror, and the warnings .clang-tidy adds
# with ExtraArgs would otherwise each stop the run as an error no check list switches off.
tidy-host-suites: ## The names in both backends' host tests, by clang-tidy
	for backend in esp32 rp2350; do \
	    "platform/$$backend/test/run.sh" compiledb; \
	    $(UV_RUN) run-clang-tidy.py -p "platform/$$backend/.pio/host-tests" -quiet \
	        -j "$$(nproc)" -checks='-*,readability-identifier-naming' -extra-arg=-Wno-error \
	        'platform/'; \
	done

# Every size the documentation states, against the images the template's `make examples` built
# last, into build/targets/: one flag per target, see CLAUDE.md "Sizes in prose".
documented-sizes: ## The sizes the documentation states, against the images of make examples
	python3 tools/documented-sizes.py \
	    --elf build/targets/avr128da48/default/programs/Wordclock.elf \
	    --esp32-bin build/targets/esp32/default/wordclock/firmware.bin \
	    --esp32-elf build/targets/esp32/default/wordclock/firmware.elf \
	    --rp2350-elf build/targets/rp2350/default/wordclock/firmware.elf

## Each backend's image on its own, as CI's firmware jobs keep them

# Not the workflow preset: --verbose, so that the size the build printed sits next to the one the
# documentation states.
avr: ## The AVR128DA48 image and its documented sizes
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

# The template's clean takes build/; these lie beside it.
clean-all: clean ## And the AVR preset's build, the backends' own images and host suites' builds
	rm -rf build-avr platform/esp32/.pio/build platform/rp2350/.pio/build $(HOST_SUITES)
