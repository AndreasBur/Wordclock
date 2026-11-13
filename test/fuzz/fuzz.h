// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * What both fuzzers do around their input: put the clock back to a known state before it, and
 * run the firmware's ticks after it.
 *
 * Back to a known state because a fuzzer runs millions of inputs in one process, against
 * singletons that keep whatever the previous input left. A crash that needs a hundred inputs
 * before it cannot be replayed from the one file libFuzzer saves, so every input starts from
 * the settings a new clock has and from the same date and time. What Persistence::Reset() does
 * not reach - an animation that is half way through, an overlay that is showing - still carries
 * over, and is ended here as well.
 *
 * The ticks are the scheduler's alone. WordclockMain::Task() also runs the real time clock's
 * task, which counts on from the host's own clock and would make every run a little different;
 * the date and time are set instead.
 */
#ifndef WORDCLOCK_TEST_FUZZ_FUZZ_H_
#define WORDCLOCK_TEST_FUZZ_FUZZ_H_

#include "Arduino.h"
#include "ClockDateTime.h"
#include "DisplayManager.h"
#include "Overlays.h"
#include "Persistence.h"
#include "RealTimeClock.h"
#include "Scheduler.h"

inline Scheduler& GetScheduler() {
  static Scheduler scheduler;
  return scheduler;
}

inline ClockDateTime GetFixedDateTime() {
  ClockDateTime date_time;
  date_time.SetDateYear(2026u);
  date_time.SetDateMonth(11u);
  date_time.SetDateDay(6u);
  date_time.SetTimeHour(14u);
  date_time.SetTimeMinute(29u);
  date_time.SetTimeSecond(50u);
  return date_time;
}

inline void StartFromANewClock() {
  (void)Overlays::GetInstance().Abort();
  (void)DisplayManager::GetInstance().AbortAnimation();
  (void)Persistence::Reset();
  RealTimeClock::GetInstance().SetDateTime(GetFixedDateTime());
}

inline void RunTicks(unsigned ticks) {
  for (unsigned tick = 0u; tick < ticks; tick++) {
    GetScheduler().Task();
  }
}

/*
 * Until the port has nothing left to read, and one tick more: Communication takes a line off the
 * port in one tick and parses it in the next. Bounded by the input, because every tick in which
 * the port holds something reads at least one character of it.
 */
inline void RunUntilThePortIsRead() {
  while (Serial.available()) {
    RunTicks(1u);
  }
  RunTicks(1u);
}

#endif  // WORDCLOCK_TEST_FUZZ_FUZZ_H_
