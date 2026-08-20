"""Compresses what is under web/ and emits it as C++ arrays the firmware can serve.

The page and everything it asks for are part of the firmware rather than files on a
second partition. That way `pio run -t upload` ships them together with the code and
their versions cannot drift apart - which is the failure a filesystem invites: firmware
updated, page stale, and nothing says so.

The page is compressed here and served with `Content-Encoding: gzip`, so the controller
only pushes bytes out of flash and the browser does the work. Nothing else is: a PNG is
already deflated, and the manifest is 485 bytes, where the 228 gzip saves buys a header
on the wire and an inflate in every reader that wants to look at it.

The generated header goes into the build directory, never into the source tree. A
checked-in generated file rots the moment someone edits the page and forgets to
regenerate it; as a build product that cannot happen.
"""

import gzip
import re
import sys
from collections import namedtuple
from pathlib import Path

# Called two ways: by PlatformIO as an extra script, where SCons injects Import, and from
# the command line by test/run.sh, which needs the same header to compile the backend on
# the host. One generator either way - a second one would drift from this.
#
# And one copy for both backends, which is why this lives above them rather than in either
# one's scripts/ directory. It was two byte-identical files until they were shared; nothing
# in here names a platform, and what the header holds is the same on both.
try:
    Import("env")  # noqa: F821
    FROM_PLATFORMIO = True
except NameError:
    FROM_PLATFORMIO = False


HEADER_NAME = "WebPage.h"

# What ends up in flash, and under which name. The symbol is written out in full rather
# than derived from the file, because it is what the backends spell in their handlers - a
# rule that turned icon-192.png into a symbol would put the naming of both C++ files in
# here, where nobody reading them would look for it.
#
# `gzip` is per asset for the reason in the docstring, and `strip` only applies to the page:
# it is the one asset with comments worth having in the source and not worth shipping.
# `type` is not read by anything that runs - the handlers set their own - and is here to put
# it in the generated header, where it is the only thing saying what a blob of bytes is for.
Asset = namedtuple("Asset", "source symbol type gzip strip")

ASSETS = (
    Asset("index.html", "WebPage", "text/html", gzip=True, strip=True),
    Asset("manifest.webmanifest", "WebManifest", "application/manifest+json", gzip=False, strip=False),
    Asset("icon-192.png", "WebIcon192", "image/png", gzip=False, strip=False),
    Asset("icon-512.png", "WebIcon512", "image/png", gzip=False, strip=False),
)


def strip_comments(text):
    """Takes the page's comments out on the way into flash, leaving them in the source.

    They are half of index.html and, compressed, 10 KiB of the 16.5 KiB the page would come
    to with them - which is 0.3% of the ESP32's flash and worth nothing there, but the page is
    also the place where the reasoning is worth the most, because CSS decisions read as
    arbitrary without it. So the explanation stays where it is read and stops being shipped.

    The numbers move with the page, and two of them were wrong here for a while; what does
    not move is the ratio. Anything that says a size in this repository is worth measuring
    before it is trusted.

    Only block comments, and that is what makes this safe rather than clever: the page uses
    /* */ throughout, which is this project's style everywhere, so there is no // form to
    remove - and // is exactly what a naive stripper would take out of ws:// and http://.
    The check below is what keeps that assumption from going quiet if it ever stops holding.
    """
    if re.search(r"^\s*//", text, flags=re.M):
        raise SystemExit(
            "embed_web.py: index.html has a // comment. Only /* */ is removed here, because "
            "// cannot be told from the one in ws:// without parsing the page. Use /* */."
        )

    text = re.sub(r"<!--.*?-->", "", text, flags=re.S)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    # The blank lines the comments leave behind, and the trailing spaces with them.
    text = re.sub(r"[ \t]+$", "", text, flags=re.M)
    return re.sub(r"\n{3,}", "\n\n", text)


def render_bytes(symbol, blob):
    lines = [f"static const uint8_t {symbol}[] = {{"]

    for offset in range(0, len(blob), 16):
        chunk = blob[offset:offset + 16]
        lines.append("    " + " ".join(f"0x{byte:02x}," for byte in chunk))

    lines += [
        "};",
        "",
        f"static constexpr size_t {symbol}Size{{{len(blob)}u}};",
        "",
    ]
    return lines


def render(blobs):
    """One header for every asset, in the order ASSETS lists them.

    The gzipped ones keep the `Gzip` in their symbol, so a handler that forgets the
    Content-Encoding header reads wrong at the call rather than only in the browser.
    """
    lines = [
        "/* Generated by scripts/embed_web.py from web/ - do not edit. */",
        "#ifndef _WEB_PAGE_H_",
        "#define _WEB_PAGE_H_",
        "",
        "#include <stddef.h>",
        "#include <stdint.h>",
        "",
    ]

    for asset, blob in zip(ASSETS, blobs):
        suffix = "Gzip" if asset.gzip else ""
        served = f"{asset.type}, gzip-compressed" if asset.gzip else asset.type
        lines.append(f"/* {asset.source}: served as {served}. */")
        lines += render_bytes(asset.symbol + suffix, blob)

    lines += [
        "#endif // _WEB_PAGE_H_",
        "",
    ]
    return "\n".join(lines)


def read_asset(asset, web_dir):
    """One asset as the bytes that go into flash, and a line saying what became of it."""
    path = Path(web_dir) / asset.source

    if not path.is_file():
        raise SystemExit(f"embed_web.py: {path} not found")

    if not asset.gzip:
        blob = path.read_bytes()
        return blob, f"{asset.source}: {len(blob)} bytes as they are"

    text = path.read_text(encoding="utf-8")
    stripped = strip_comments(text) if asset.strip else text
    # mtime=0 so the same input always yields the same bytes, which keeps a rebuild from
    # looking like a change.
    blob = gzip.compress(stripped.encode("utf-8"), compresslevel=9, mtime=0)
    without = f" -> {len(stripped.encode('utf-8'))} without comments" if asset.strip else ""

    return blob, f"{asset.source}: {path.stat().st_size}{without} -> {len(blob)} gzipped"


def embed(web_dir, build_dir):
    reports = []
    blobs = []

    for asset in ASSETS:
        blob, report = read_asset(asset, web_dir)
        blobs.append(blob)
        reports.append(report)

    target = Path(build_dir) / HEADER_NAME
    target.parent.mkdir(parents=True, exist_ok=True)
    rendered = render(blobs)

    # Only rewrite when it differs, so an unchanged page does not force a recompile of
    # everything that includes the header.
    if not target.is_file() or target.read_text() != rendered:
        target.write_text(rendered)

    print(f"embed_web.py: {Path(web_dir).name}/ -> {target}")
    for report in reports:
        print(f"  {report}")
    return target.parent


if FROM_PLATFORMIO:
    build_dir = embed(
        # The page lives at the repository root, not under this platform: both backends
        # serve the same one, and a second copy would have to be kept in step by hand.
        Path(env.subst("$PROJECT_DIR")).resolve().parents[1] / "web",  # noqa: F821
        env.subst("$BUILD_DIR"),  # noqa: F821
    )
    env.Append(CPPPATH=[str(build_dir)])  # noqa: F821
elif __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit(f"usage: {Path(sys.argv[0]).name} <web directory> <output directory>")

    embed(Path(sys.argv[1]), sys.argv[2])
