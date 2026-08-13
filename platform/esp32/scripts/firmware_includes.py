"""Puts the firmware core's header directories on the include path, and tells this
platform's Arduino.h where the Arduino core's own Arduino.h lives.

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


def core_arduino_header(env):
    """Absolute path of the Arduino core's Arduino.h.

    This platform's include/Arduino.h shadows that name to rebind Serial, and has to
    include the original - which it cannot do with include_next, see the note in that
    file. Looked up in the installed framework rather than hardcoded, so a framework
    update moves it without an edit here.
    """
    framework = Path(env.subst("$PROJECT_PACKAGES_DIR")) / "framework-arduinoespressif32"
    candidates = sorted(framework.rglob("cores/esp32/Arduino.h"))

    if not candidates:
        raise SystemExit(
            "firmware_includes.py: no cores/esp32/Arduino.h under "
            f"{framework} - is the Arduino framework installed?"
        )

    return candidates[0]


env.Append(CPPPATH=firmware_include_dirs(env.subst("$PROJECT_DIR")))  # noqa: F821
# The escaped quotes are part of the value: `#include WORDCLOCK_CORE_ARDUINO_H` needs the
# macro to expand to a quoted path, so they have to survive the shell into the compiler.
env.Append(  # noqa: F821
    CPPDEFINES=[("WORDCLOCK_CORE_ARDUINO_H", '\\"{}\\"'.format(core_arduino_header(env)))]
)
