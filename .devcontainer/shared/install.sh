#!/bin/sh
# This feature only carries devcontainer.json metadata (extensions, settings and
# the postCreateCommand); the toolchain itself is installed by ../Dockerfile.
# A feature must still provide an install entrypoint, so this one does nothing.
set -e
echo "Wordclock shared editor configuration: metadata only, nothing to install."
