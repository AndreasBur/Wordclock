// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Entry point of the AVR Dx application.
 *
 * The counterpart of the simulator's WordclockApp: bring the runtime up, bring the firmware up,
 * then tick it at the scheduler's interval.
 *
 * The interval is read from the scheduler rather than repeated here. Every module's task cycle
 * counts in it - animation speeds, the sensor's sampling rate, the console's poll rate - so a tick
 * at another rate would rescale all of them at once, and silently.
 */

#include "Arduino.h"
#include "Pixels.h"
#include "Scheduler.h"
#include "System.h"
#include "WordclockConfiguration.h"
#include "WordclockMain.h"

namespace {
WordclockMain wordclock;

}  // namespace

/*!
 * \brief Brings the clock up and runs it, one tick every Scheduler::GetTaskIntervalMs()
 *        milliseconds.
 *
 * A tick runs the firmware's tasks, sends the buffer to the strip if it is marked, and carries
 * out a restart a command asked for, in that order.
 *
 * Timed against an absolute wake-up rather than a delay after the work, so that what a tick costs
 * does not stretch the interval. A frame takes about 3.3 ms of the ten, and adding the two would
 * run the clock a third slow.
 *
 * \return Never: the loop runs until the controller is reset.
 */
int main() {
  InitRuntime();

  Serial.begin(WORDCLOCK_SERIAL_BAUDRATE);
  Serial.println(F("Wordclock"));

  wordclock.Init();

  uint32_t next_tick = millis();

  for (;;) {
    // Subtraction rather than a comparison of the two, so that the wrap of the
    // millisecond counter passes without a tick that never comes due.
    if (static_cast<int32_t>(millis() - next_tick) < 0) {
      continue;
    }

    next_tick += Scheduler::GetTaskIntervalMs();

    wordclock.Task();

    // After the firmware, not from inside its pixel writes: the strip gets whatever the
    // buffer holds once the whole pass over the tasks is done.
    Pixels::GetInstance().Render();

    // Last, so a restart asked for by a command takes place once that command's answer
    // has reached the wire.
    System::GetInstance().PerformPendingRestart();
  }
}
