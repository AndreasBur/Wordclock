// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_PICOOTA_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_PICOOTA_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <string>
/*
 * Stand-in for the core's PicoOTA, which on the target writes the command page the OTA
 * loader reads on the next boot. Nothing here writes flash, so what the stub keeps is the
 * one fact a test can check: which file the loader was told to install, and only once the
 * commit succeeded. Empty therefore means "this clock will boot what it is running", which
 * is what every failing case has to leave behind.
 *
 * The two switches are for the cases that cannot be provoked otherwise: a filesystem that
 * loses the image between writing and reading it, and a command page that will not write.
 */
struct PicoOtaStub {
  bool AddSucceeds{true};
  bool CommitSucceeds{true};
  std::string Named;
  std::string Committed;

  void begin() {
    Named.clear();
    Committed.clear();
  }

  bool addFile(char const* Path) {
    if (!AddSucceeds) {
      return false;
    }
    Named = Path;
    return true;
  }

  bool commit() {
    if (!CommitSucceeds || Named.empty()) {
      return false;
    }
    Committed = Named;
    return true;
  }
};

extern PicoOtaStub picoOTA;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_PICOOTA_H_
