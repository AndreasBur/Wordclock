"""Puts the firmware core's header directories on the include path.

The core's modules include each other by bare file name ("Display.h", "Scheduler.h"),
so every directory under firmware/inc has to be on the path. They are globbed rather
than listed for the same reason the CMake build globs them: a list would have to be
edited whenever a module is added, and the omission shows up as a missing header in an
unrelated file.

Unlike the CMake glob this one recurses, so the nested Communication/MessageParser
directories need no separate entry.
"""

from pathlib import Path

Import("env")  # noqa: F821 - injected by PlatformIO


def firmware_include_dirs(project_dir):
    # platform/esp32 -> repository root -> firmware/inc
    firmware_inc = Path(project_dir).resolve().parents[1] / "firmware" / "inc"

    if not firmware_inc.is_dir():
        raise SystemExit(f"firmware_includes.py: {firmware_inc} not found")

    directories = [firmware_inc]
    directories += sorted(path for path in firmware_inc.rglob("*") if path.is_dir())
    return [str(path) for path in directories]


env.Append(CPPPATH=firmware_include_dirs(env.subst("$PROJECT_DIR")))  # noqa: F821
