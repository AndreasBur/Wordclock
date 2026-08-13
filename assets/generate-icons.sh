#!/usr/bin/env bash
#
# Regenerates the icon files from the SVG masters next to this script.
#
# The icon is deliberately drawn three times rather than scaled from one image,
# because eleven columns of letters do not survive being made small: at 16 pixels a
# column is 1.5 pixels wide. Each master therefore serves the sizes it can carry,
# and all three keep the same lit words - "ES IST HALB ZWOELF", taken from the real
# word table - so the silhouette stays recognisable across every size:
#
#   wordclock-icon.svg        128, 256   the grid with its letters
#   wordclock-icon-dots.svg    48,  64   the same grid, letterforms dropped
#   wordclock-icon-small.svg   16,  32   only the lit words, as bars
#
# Rasterising needs ImageMagick, and the letter master needs DejaVu Sans Bold - both
# are in the dev container. Everything this writes is checked in, so the script only
# has to run when the design changes.
#
# Usage: assets/generate-icons.sh

set -euo pipefail

ASSETS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$ASSETS/.." && pwd)"

SIMULATOR="$ROOT/platform/simulator"
DOCS="$ROOT/docs/images"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

rasterise() {   # master size outfile
    magick "$ASSETS/$1" -background none -resize "${2}x${2}" "png32:$WORK/$3"
}

rasterise wordclock-icon-small.svg  16 16.png
rasterise wordclock-icon-small.svg  32 32.png
rasterise wordclock-icon-dots.svg   48 48.png
rasterise wordclock-icon-dots.svg   64 64.png
rasterise wordclock-icon.svg       128 128.png
rasterise wordclock-icon.svg       256 256.png

# One .ico carrying every size, so Windows picks the master meant for the slot it is
# filling instead of downscaling the largest one.
magick "$WORK/16.png" "$WORK/32.png" "$WORK/48.png" "$WORK/64.png" "$WORK/128.png" "$WORK/256.png" \
    "$SIMULATOR/Wordclock.ico"

# The XPM is what wxWidgets uses on GTK and macOS, and SetIcon() takes a single image -
# so this is a compromise size. 48 is where the grid still reads and a title bar still
# has something to shrink. The array name has to stay WordclockIcon_xpm, which is what
# the wxICON(WordclockIcon) in PixelsFrame.cpp expands to.
magick "$WORK/48.png" -colors 255 +dither "$WORK/icon.xpm"
sed 's/^static char \*icon\[\]/static const char * WordclockIcon_xpm[]/; s/^static const char \*icon\[\]/static const char * WordclockIcon_xpm[]/' \
    "$WORK/icon.xpm" > "$SIMULATOR/WordclockIcon.xpm"

if ! grep -q 'WordclockIcon_xpm' "$SIMULATOR/WordclockIcon.xpm"; then
    echo "generate-icons.sh: the XPM array is not named WordclockIcon_xpm - wxICON() will not find it" >&2
    exit 1
fi

mkdir -p "$DOCS"
cp "$WORK/256.png" "$DOCS/logo.png"

echo "wrote:"
echo "  $SIMULATOR/Wordclock.ico"
echo "  $SIMULATOR/WordclockIcon.xpm"
echo "  $DOCS/logo.png"
