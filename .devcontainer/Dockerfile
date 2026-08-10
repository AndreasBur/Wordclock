# Base image, overridable via build.args in a devcontainer.json variant so that
# site-specific base images can be used without touching this file.
ARG BASE_IMAGE=mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04
FROM ${BASE_IMAGE}

# Toolchain + wxWidgets development libraries.
# Ubuntu 24.04 ships wxWidgets 3.2 (GTK build), which satisfies the
# find_package(wxWidgets ...) call in CMakeLists.txt.
# imagemagick, xdotool and x11-utils are not needed to build or run the simulator.
# They let an agent screenshot the running window to check a layout change instead of
# having to ask; see "Screenshotting the simulator" in CLAUDE.md.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        gdb \
        libwxgtk3.2-dev \
        wx-common \
        imagemagick \
        xdotool \
        x11-utils && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# VS Code creates /tmp/user/<uid> before postCreateCommand runs. Provide its
# shared parent so the non-root remote user can create the per-user directory.
RUN chmod 1777 /tmp && install -d -m 1777 /tmp/user

ENV TZ=Europe/Berlin

# Extensions, editor settings and lifecycle commands live in the local feature
# in shared/, which every devcontainer.json variant references. A
# devcontainer.metadata label set here would not work: clients read image
# metadata from the image the final FROM points at, then overwrite the label on
# the image they build from this file.
