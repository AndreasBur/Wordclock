#!/bin/sh
# Links the workspace's .venv to the image's tool environment, so that `uv run` there uses the
# environment the image built - the one on PATH, with the proxy's certificates in its certifi -
# rather than making a second one beside it. And puts the image's uv on PATH if the container
# has none of its own.
#
# A link rather than UV_PROJECT_ENVIRONMENT: that variable names the environment of every uv
# project in the container, the Python servers other features install included, and they synced
# themselves into it until neither they nor this repository's tools were whole.
#
# Run from the workspace, as a postCreateCommand is. A .venv that is a directory is somebody's
# own and is left alone, with a note.
set -e

TOOLS=/opt/wordclock-env
UV=/opt/uv-pip/bin

# Only now, when every feature is installed: the image keeps its uv off PATH, because a feature
# that brings its own uv with the configuration it needs installs neither when it finds one.
if ! command -v uv >/dev/null 2>&1 && [ -x "$UV/uv" ]; then
    if sudo -n ln -sf "$UV/uv" "$UV/uvx" /usr/local/bin/ 2>/dev/null; then
        echo "wordclock-tool-env: uv on PATH from $UV"
    else
        echo "wordclock-tool-env: no uv on PATH and no sudo to link $UV/uv; call it by its path" >&2
    fi
fi

[ -d "$TOOLS" ] || exit 0
if ! grep -qs '^name = "wordclock-tools"' pyproject.toml; then
    echo "wordclock-tool-env: not run from the Wordclock workspace; .venv left as it is" >&2
    exit 0
fi

if [ -L .venv ] || [ ! -e .venv ]; then
    ln -sfn "$TOOLS" .venv
    echo "wordclock-tool-env: .venv links to $TOOLS"
else
    echo "wordclock-tool-env: .venv is a directory of its own, so uv run uses it rather than $TOOLS" >&2
fi
