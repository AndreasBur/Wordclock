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
#   wordclock-icon.svg        128, 192, 256, 512   the grid with its letters
#   wordclock-icon-dots.svg    48,  64              the same grid, letterforms dropped
#   wordclock-icon-small.svg   16,  32              only the lit words, as bars
#
# Rasterising needs ImageMagick *and* librsvg2-bin, which is the part that is easy to
# miss: ImageMagick does not draw SVG itself, it hands the file to rsvg-convert. The
# letter master also needs DejaVu Sans Bold. All three are in the dev container.
# Everything this writes is checked in, so the script only has to run when the design
# changes - and running it twice in a row now yields the same bytes, which is what makes
# such a diff mean something.
#
# It yields the same bytes *here*. What comes out still depends on the versions of
# ImageMagick and librsvg in front of it - the checked-in files were regenerated when this
# was found precisely because an older pair had produced them - which is the other reason
# the outputs live in the repository rather than being built on demand.
#
# Usage: assets/generate-icons.sh

set -euo pipefail

ASSETS="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$ASSETS/.." && pwd)"

SIMULATOR="$ROOT/platform/simulator"
DOCS="$ROOT/docs/images"
WEB="$ROOT/web"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# -strip, and it is not cosmetic: without it ImageMagick writes the wall-clock time into
# three tEXt chunks - date:create, date:modify and date:timestamp - so every run of this
# script produced a different logo.png for the same drawing. The outputs are checked in, so
# that turned "the design changed" and "somebody ran the script" into the same diff.
rasterise() {   # master size outfile
    magick "$ASSETS/$1" -background none -resize "${2}x${2}" -strip "png32:$WORK/$3"
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

# The home screen icon, in the two sizes a browser asks a web app manifest for. These are
# the only outputs that end up in the firmware's flash, which is why they are the only ones
# written with a palette: at 512 the picture is two colours and some smoothed edges, and
# 32-bit RGBA spends 65 kB of a clock's flash on that where 16 colours spend 11 kB. Both
# come from the letter master - a home screen icon is never small enough to need the others.
pwa_icon() {    # size outfile
    magick "$ASSETS/wordclock-icon.svg" -background none -resize "${1}x${1}" \
        -colors 16 -strip -define png:compression-level=9 "PNG8:$WEB/$2"
}

pwa_icon 192 icon-192.png
pwa_icon 512 icon-512.png

echo "wrote:"
echo "  $SIMULATOR/Wordclock.ico"
echo "  $SIMULATOR/WordclockIcon.xpm"
echo "  $DOCS/logo.png"
echo "  $WEB/icon-192.png"
echo "  $WEB/icon-512.png"
