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

# The VS Code server derives XDG_RUNTIME_DIR from /tmp/user/$(id -u) whenever the
# devcontainer.json variant does not set one itself, as the Linux/X11 variant
# does not (the WSLg variants point it at /mnt/wslg/runtime-dir). It creates that
# path under `umask 077` and gives up if the parent is missing on a host whose
# /tmp it cannot write to. Pre-create the parent with the permissions systemd
# gives /run/user: world-writable so each user can add their own directory,
# sticky so none can remove another's. install -d, because a plain mkdir would
# inherit the build umask rather than the mode spelled out here.
RUN install -d -m 1777 /tmp/user

ENV TZ=Europe/Berlin

# Extensions, editor settings and lifecycle commands live in the local feature
# in shared/, which every devcontainer.json variant references. A
# devcontainer.metadata label set here would not work: clients read image
# metadata from the image the final FROM points at, then overwrite the label on
# the image they build from this file.
