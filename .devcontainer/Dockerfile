# Base image, overridable via build.args in a devcontainer.json variant so that
# site-specific base images can be used without touching this file.
#
# 26.04 rather than 24.04, and the reason is the AVR Dx backend: 24.04 carries avr-gcc 7.3,
# which cannot build it twice over - no device-specs for avr128da48, and, once handed those
# from a Microchip pack, a refusal of the core's constexpr constructors that GCC 12 onwards
# accepts. A container on 24.04 could build three of this project's four platforms.
#
# Note the tag has no hyphen. devcontainers/cpp publishes ubuntu-24.04 and ubuntu24.04 both,
# but from 26.04 only the unhyphenated form exists, so the older spelling looks like the
# newest release when it is not.
ARG BASE_IMAGE=mcr.microsoft.com/devcontainers/cpp:ubuntu26.04
FROM ${BASE_IMAGE}

# wxWidgets development libraries. Ubuntu 26.04 still ships wxWidgets 3.2 (3.2.9, GTK
# build), which satisfies the find_package(wxWidgets ...) call in CMakeLists.txt. Neither
# build-essential nor gdb is listed: the base image carries both — which is also where
# the ESP32 host-side test harness gets its g++. Its python3 and node come from the
# PlatformIO block below, which installs them for the same two jobs. wx-common is not
# listed either, but for a different reason — libwxgtk3.2-dev depends on it, and it
# is what puts the wx-config that find_package needs on the PATH.
# xvfb gives the simulator an X server nobody sees, for tools/web-smoke.py: the pages need the
# simulator behind them, and the simulator opens a window wherever it runs.
# imagemagick, xdotool and x11-utils are not needed to build or run the simulator.
# They let an agent screenshot the running window to check a layout change instead of
# having to ask; see "Screenshotting the simulator" in CLAUDE.md.
# librsvg2-bin is what assets/generate-icons.sh actually rasterises with. ImageMagick does
# not draw SVG itself - it hands the file to rsvg-convert, which is a separate package that
# librsvg2-2 does not bring - so without this the script fails with "delegate failed" and a
# missing temporary file, which reads like a broken script rather than a missing program.
# clang-tidy comes with clangd because the checks it runs are the ones CI blocks a pull
# request on; running them here is what keeps that from being a surprise. run-clang-tidy,
# which walks the compile database in parallel, is part of the same package.
# cppcheck is the second analyser of that same workflow, and is here for the same reason.
# doxygen is what tools/docs.py reads the Doxygen blocks with, a job of that workflow as well.
# shellcheck reads the shell scripts, in the "Scripts" job of that workflow; ruff and actionlint,
# the job's other two, come with the tool environment below.
# clang, libclang-rt-dev and llvm build and read the fuzzers of that workflow's fuzz job. clang
# alone is not enough, and fails late: it compiles a fuzzer and then cannot link it, because
# libFuzzer and the sanitizers' runtimes are libclang-rt-dev. llvm brings llvm-symbolizer,
# without which a finding is a stack of bare addresses.
# CI uses the same Ubuntu release for both analysers; rebuild this image to pick up
# package updates when a newer CI image reports a finding that is not reproducible locally.
# gcc-avr and avr-libc cross-compile the AVR Dx backend, which nothing else here can build:
# the simulator uses the host compiler and the ESP32 brings its own through PlatformIO. They
# are only worth installing on a base new enough for them to work - see the note on the tag
# above - and platform/avr-dx/README.md says how to ask a given one whether it is.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        cmake \
        clang \
        clangd \
        clang-tidy \
        libclang-rt-dev \
        llvm \
        cppcheck \
        doxygen \
        shellcheck \
        gcc-avr \
        avr-libc \
        ninja-build \
        libwxgtk3.2-dev \
        imagemagick \
        librsvg2-bin \
        xdotool \
        xvfb \
        x11-utils && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# gh, for the same reason as the screenshot tools: it turns branch and pull request
# housekeeping into something doable from the container rather than by hand in a browser.
# A pinned release .deb, not the universe package — that one is 2.46, whose `gh pr edit`
# still asks for the retired Projects-classic `projectCards` field and therefore fails
# outright, which is half of what it would be here for. The upstream apt repository would
# fix that as well, at the price of a second source and its signing key behind a
# TLS-intercepting proxy; one file with its published checksum keeps the trust anchored to
# something visible in this file, at the cost of bumping the version by hand.
# dpkg rather than apt because the package Depends on git alone, which the base image
# carries — so this needs no package lists and the block above can keep discarding them.
ARG GH_VERSION=2.97.0
ARG GH_SHA256_AMD64=7c7fa3bb890db0934baf65910d97b8c0fa437b2e590f7f7daf6bdf82c5c486d7
ARG GH_SHA256_ARM64=0ba7a76739c865d82ebde24667d875d9b8caa55db47c7597c24accdd4defd2bb
RUN ARCH="$(dpkg --print-architecture)" && \
    case "${ARCH}" in \
        amd64) SHA256="${GH_SHA256_AMD64}" ;; \
        arm64) SHA256="${GH_SHA256_ARM64}" ;; \
        *) echo "gh: no pinned checksum for ${ARCH}; add one or drop gh" >&2; exit 1 ;; \
    esac && \
    curl -fsSL -o /tmp/gh.deb \
        "https://github.com/cli/cli/releases/download/v${GH_VERSION}/gh_${GH_VERSION}_linux_${ARCH}.deb" && \
    echo "${SHA256}  /tmp/gh.deb" | sha256sum -c - && \
    dpkg -i /tmp/gh.deb && \
    rm /tmp/gh.deb

# python3 for the tool environment below and for what the scripts in tools/ and test/run.sh
# need - the page embedding among them - and python3-venv for the environment uv is installed
# into. node and npm for the web pages' linters, which `make web-lint` installs from
# package-lock.json on first use: Ubuntu packages npm apart from nodejs.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        python3 \
        python3-venv \
        nodejs \
        npm && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# uv, which makes the environment below from pyproject.toml and uv.lock. This ARG is the one
# place its version is written: .github/actions/uv-sync reads it for CI. From PyPI, into an
# environment of its own as pip will not write into the system's.
#
# Not on PATH, and called by its path below. Features installed after this layer may bring a uv
# of their own, and install it - with the configuration their own servers depend on - only when
# they find none: with this one on PATH, the Vector base image's features skipped both, and its
# ALM server failed to install. The shared feature puts this uv on PATH when the container is
# created, if nothing has brought one by then.
ARG UV_VERSION=0.12.0
RUN python3 -m venv /opt/uv-pip && \
    /opt/uv-pip/bin/pip install --no-cache-dir "uv==${UV_VERSION}"

# The repository's Python tools - PlatformIO, Playwright, ruff, actionlint - in one environment,
# at the versions uv.lock pins, which are CI's as well. Under /opt so it belongs to the image
# rather than to a home directory a volume may replace, and on PATH, so `pio` and the rest work as
# they did when each had an environment of its own. It belongs to the remote user, because a
# changed uv.lock is synced into it by the next `uv run`; the shared feature links the
# workspace's .venv to it, which is how `uv run` finds it.
#
# UV_PROJECT_ENVIRONMENT is set for this one sync and nowhere else. Set as an ENV it named the
# environment of every uv project in the image: the features installed after this layer synced
# their own Python servers into it, each removing what the one before had put there, and the
# image came out with neither our tools nor all of theirs.
#
# PlatformIO downloads with requests and hands it certifi's bundle, so it ignores
# REQUESTS_CA_BUNDLE and fails behind a TLS-intercepting proxy with CERTIFICATE_VERIFY_FAILED on
# URLs that curl and git fetch without complaint. Appending the image's own trust store keeps
# verification on rather than turning it off. The platform packages build a second environment
# under ~/.platformio at first use, and a sync can replace certifi here; the shared feature's
# postStartCommand appends to both.
#
# Playwright's Chromium is what looks at the two pages - its own, because apt on this base offers
# `chromium-browser` as a snap stub and snap does not run in a container. PLAYWRIGHT_BROWSERS_PATH
# keeps it out of ~/.cache, which a volume replaces. --with-deps pulls the ~30 libraries a headless
# Chromium links against, so this layer is the expensive one in the image: about 650 MB.
# At the end of PATH, not the front: the tools here are found either way, as nothing else carries
# them, but at the front anything the environment brings would shadow the system's - a uv a
# package depends on would stand in front of the container's own.
ENV PLAYWRIGHT_BROWSERS_PATH=/opt/ms-playwright \
    PATH="${PATH}:/opt/wordclock-env/bin"
COPY pyproject.toml uv.lock /opt/wordclock-tools/
RUN cd /opt/wordclock-tools && \
    UV_PROJECT_ENVIRONMENT=/opt/wordclock-env UV_LINK_MODE=copy UV_NO_CACHE=1 /opt/uv-pip/bin/uv sync --frozen && \
    cat /etc/ssl/certs/ca-certificates.crt \
        >> "$(/opt/wordclock-env/bin/python -c 'import certifi; print(certifi.where())')" && \
    /opt/wordclock-env/bin/playwright install --with-deps chromium && \
    chmod -R a+rX /opt/ms-playwright && \
    chown -R vscode:vscode /opt/wordclock-env && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# The headers of LLVM, clang and clang-tidy that tools/lib/tidy_module.sh compiles the template's
# checks of its own against, as a module that the clang-tidy of uv.lock loads. LLVM_MAJOR is that
# clang-tidy's major version: tools/lib/tidy_module.sh fails, saying so, when uv.lock moves on and
# this does not. CI's action llvm-headers reads it from here, so there is one place that says
# which LLVM. llvm-headers.sh, the template's, says where they come from.
ARG LLVM_MAJOR=23
COPY .devcontainer/llvm-headers.sh /tmp/llvm-headers.sh
RUN /tmp/llvm-headers.sh "${LLVM_MAJOR}" "/opt/llvm-${LLVM_MAJOR}/include" && \
    rm /tmp/llvm-headers.sh

# The package cache is a named volume, and an empty one takes its ownership from the path
# it covers - so the path has to exist and belong to the remote user first, or PlatformIO
# cannot write the ~1.5 GB of toolchain into it.
RUN install -d -o vscode -g vscode -m 0755 /home/vscode/.platformio

# The same arrangement for the two logins a rebuild would otherwise throw away: Codex, whose
# extension the shared feature installs into every variant, and gh, which every push and
# every pull request goes through. Mode 0700 rather than 0755, because what these hold is a
# credential and not a cache - and gh refuses to read a config directory others can enter.
RUN install -d -o vscode -g vscode -m 0700 /home/vscode/.codex && \
    install -d -o vscode -g vscode -m 0700 /home/vscode/.config/gh

# GitHub's SSH host key, so that the first git operation in a fresh container verifies
# against something rather than failing with "Host key verification failed" - which is
# what it does today, because ~/.ssh belongs to the container and is empty after every
# rebuild. System-wide rather than in the remote user's home for the same reason.
#
# Written out rather than fetched with ssh-keyscan at build time: a scan believes whatever
# answers it, which is precisely what a known host key is there to prevent, and the answer
# here would come through a proxy. What makes this line trustworthy instead is the
# fingerprint below - GitHub publishes it under "GitHub's SSH key fingerprints", and
# `ssh-keygen -lf /etc/ssh/ssh_known_hosts` prints it back out of the file.
#
# Three names for one key. github.com is the ordinary host; ssh.github.com serves SSH on
# 443 for networks that block 22, which is the port the remotes here use; and the bracketed
# form is the same host again, because an entry is keyed by host *and* port.
ARG GITHUB_HOST_KEY="ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIOMqqnkVzrm0SdG6UOoqKLsabgH5C9okWi0dh2l9GKJl"
RUN printf '# SHA256:+DiY3wvvV6TuJJhbpZisF/zLDA0zPMSvHdkr4UvCOqU\n%s %s\n%s %s\n%s %s\n' \
        'github.com' "${GITHUB_HOST_KEY}" \
        'ssh.github.com' "${GITHUB_HOST_KEY}" \
        '[ssh.github.com]:443' "${GITHUB_HOST_KEY}" \
        >> /etc/ssh/ssh_known_hosts && \
    chmod 0644 /etc/ssh/ssh_known_hosts

ENV TZ=Europe/Berlin

# Extensions, editor settings and lifecycle commands live in the local feature
# in shared/, which every devcontainer.json variant references. A
# devcontainer.metadata label set here would not work: clients read image
# metadata from the image the final FROM points at, then overwrite the label on
# the image they build from this file.
