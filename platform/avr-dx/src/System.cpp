// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The machine the clock runs on, see System.h.
 */

#include "System.h"

#include <avr/io.h>

#include "Arduino.h"

namespace {
/*
 * Where the linker put the end of the statically allocated data, and where the allocator
 * has grown to if it was ever used. Declared by avr-libc rather than defined here, so the
 * names are avr-libc's.
 */
extern "C" char __heap_start;  // NOLINT(readability-identifier-naming,bugprone-reserved-identifier)
extern "C" char* __brkval;     // NOLINT(readability-identifier-naming,bugprone-reserved-identifier)

constexpr uint16_t kBytesPerKibibyte{1024u};

}  // namespace

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 * The ones it would make const drive the hardware, which a const would deny.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
// NOLINTBEGIN(readability-make-member-function-const)
StdReturnType System::GetFreeMemoryInKibibytes(uint16_t& free_memory) const {
  char stack_mark;
  /*
   * As addresses rather than pointers: the stack and the heap are not one array, and
   * comparing or subtracting pointers into two different objects is undefined - an
   * optimiser is entitled to fold the comparison away. Integers have no such rule.
   */
  uintptr_t const stack_address = reinterpret_cast<uintptr_t>(&stack_mark);
  uintptr_t const lower_edge =
      reinterpret_cast<uintptr_t>((__brkval == nullptr) ? &__heap_start : __brkval);

  if (stack_address < lower_edge) {
    return kENotOk;
  }

  free_memory =
      static_cast<uint16_t>(static_cast<uint16_t>(stack_address - lower_edge) / kBytesPerKibibyte);

  return kEOk;
}

void System::PerformPendingRestart() {
  if (!restart_pending_) {
    return;
  }

  Serial.flush();

  // The software reset request is protected, like every register that can change how the
  // device runs.
  _PROTECTED_WRITE(RSTCTRL.SWRR, RSTCTRL_SWRST_bm);
}
// NOLINTEND(readability-make-member-function-const)
// NOLINTEND(readability-convert-member-functions-to-static)
