#!/bin/sh
# The editor metadata in devcontainer-feature.json is the bulk of this feature; the
# toolchain itself is installed by ../Dockerfile. What is installed here are the scripts that
# have to run in the container rather than at image build: one fixes up what lives in a volume,
# one links the workspace to the image's tool environment, and one answers for the image the
# container was made from.
set -e

install -m 0755 "$(dirname "$0")/platformio-trust-store.sh" /usr/local/bin/platformio-trust-store
install -m 0755 "$(dirname "$0")/toolchain-check.sh" /usr/local/bin/wordclock-toolchain-check
install -m 0755 "$(dirname "$0")/tool-env-link.sh" /usr/local/bin/wordclock-tool-env
echo "Wordclock shared configuration: installed platformio-trust-store, wordclock-tool-env and wordclock-toolchain-check."
