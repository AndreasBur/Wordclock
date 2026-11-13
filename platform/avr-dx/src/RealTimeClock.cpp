// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The time the core reads, kept by the clock chip, see RealTimeClock.h.
 */

#include "RealTimeClock.h"

#include "Arduino.h"

/*
 * Takes the time from the chip, if it has one to give. What the chip refuses to answer is
 * left alone rather than replaced: a board with no chip fitted, or one whose battery ran
 * out, keeps the default date and shows an obviously wrong time - which is what tells a
 * first bring-up apart from a display that is simply dark.
 */
StdReturnType RealTimeClock::ReadChip() {
  ClockDateTime value;

  if (chip_.GetDateTime(value) == kENotOk) {
    return kENotOk;
  }

  date_time_ = value;
  return kEOk;
}

/*
 * Puts what was just set into the chip. The return code is dropped on purpose: a clock with
 * no chip has to keep what a command set it to, and there is nobody here to report a
 * failure to - the command's own answer has already been decided by the parser that called
 * this.
 */
void RealTimeClock::WriteChip() {
  chip_.SetDateTime(date_time_);

  /*
   * Restarted, so the value just written is not read back over in the same second - the
   * chip zeroes nothing, but a read landing between the write and the next second would
   * hand back a time one second behind what the answer reported.
   */
  chip_read_countdown_ = kChipReadInterval;
}

void RealTimeClock::Task() {
  if (chip_read_countdown_ > 0u) {
    chip_read_countdown_--;
    return;
  }

  chip_read_countdown_ = kChipReadInterval;
  ReadChip();
}

void RealTimeClock::SetDateTime(ClockDateTime date_time) {
  date_time_ = date_time;
  WriteChip();
}

void RealTimeClock::SetTime(ClockTime time) {
  date_time_.SetTime(time);
  WriteChip();
}

void RealTimeClock::SetDate(ClockDate date) {
  date_time_.SetDate(date);
  WriteChip();
}
