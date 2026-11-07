#!/bin/sh
# Appends the container's trust store to every certifi bundle under ~/.platformio and in the
# tool environment.
#
# The tool environment's is handled at build time, but a `uv run` after uv.lock changed can
# replace certifi there, and the platform packages build a second virtual environment under
# ~/.platformio at first use, which lives in a volume rather than in the image. Without this,
# a download from either fails behind a TLS-intercepting proxy with CERTIFICATE_VERIFY_FAILED -
# on a URL curl and git fetch without complaint, because PlatformIO hands requests its own
# bundle and ignores REQUESTS_CA_BUNDLE.
#
# Idempotent: the marker keeps a restart from appending the same certificates again.
set -e

MARKER='# wordclock-devcontainer: container trust store appended'
STORE=/etc/ssl/certs/ca-certificates.crt

[ -r "$STORE" ] || exit 0

for root in "$HOME/.platformio" /opt/wordclock-env; do
    [ -d "$root" ] && find "$root" -name cacert.pem -type f 2>/dev/null
done | while read -r bundle; do
    grep -qF "$MARKER" "$bundle" && continue

    printf '\n%s\n' "$MARKER" >> "$bundle"
    cat "$STORE" >> "$bundle"
    echo "trust store appended to $bundle"
done
