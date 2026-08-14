#!/bin/sh
# The editor metadata in devcontainer-feature.json is the bulk of this feature; the
# toolchain itself is installed by ../Dockerfile. What is installed here is the one script
# that has to run at container start rather than at image build, because what it fixes up
# lives in a volume.
set -e

install -m 0755 "$(dirname "$0")/platformio-trust-store.sh" /usr/local/bin/platformio-trust-store
echo "Wordclock shared configuration: installed platformio-trust-store."
