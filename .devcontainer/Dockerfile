FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

# wxWidgets plus the tools used to build and visually inspect the simulator.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
    apt-get install -y --no-install-recommends \
        cmake \
        ninja-build \
        libwxgtk3.2-dev \
        imagemagick \
        xdotool \
        x11-utils && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Seed the Codex volume with the correct ownership on first use.
RUN install -d -o vscode -g vscode -m 0700 /home/vscode/.codex

ENV TZ=Europe/Berlin
