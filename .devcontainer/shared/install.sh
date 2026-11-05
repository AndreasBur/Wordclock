#!/bin/sh
# The editor metadata in devcontainer-feature.json is the bulk of this feature; the
# toolchain itself is installed by ../Dockerfile. What is installed here are the two
# scripts that have to run in the container rather than at image build: one fixes up what
# lives in a volume, the other answers for the image the container was made from.
set -e

install -m 0755 "$(dirname "$0")/platformio-trust-store.sh" /usr/local/bin/platformio-trust-store
install -m 0755 "$(dirname "$0")/toolchain-check.sh" /usr/local/bin/wordclock-toolchain-check
echo "Wordclock shared configuration: installed platformio-trust-store and wordclock-toolchain-check."
