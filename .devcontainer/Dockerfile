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

# Settings shared by all devcontainer.json variants. Dev Container clients
# merge this image metadata with the selected platform-specific configuration.
LABEL devcontainer.metadata='[{ \
    "customizations": { \
        "vscode": { \
            "extensions": [ \
                "ms-vscode.cpptools", \
                "ms-vscode.cmake-tools", \
                "EditorConfig.EditorConfig", \
                "eamodio.gitlens", \
                "openai.chatgpt" \
            ], \
            "settings": { \
                "cmake.configureOnOpen": true, \
                "cmake.generator": "Ninja", \
                "cmake.configureSettings": { \
                    "PLATFORM": "simulator" \
                } \
            } \
        } \
    }, \
    "postCreateCommand": "wx-config --version && cmake --version" \
}]'

ENV TZ=Europe/Berlin
