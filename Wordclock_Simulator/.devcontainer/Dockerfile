FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

# Toolchain + wxWidgets development libraries.
# Ubuntu 24.04 ships wxWidgets 3.2 (GTK build), which satisfies the
# find_package(wxWidgets ...) call in CMakeLists.txt.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        gdb \
        libwxgtk3.2-dev \
        wx-common && \
    apt-get clean && rm -rf /var/lib/apt/lists/*
