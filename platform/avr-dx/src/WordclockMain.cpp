// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The firmware, brought up and ticked, see WordclockMain.h.
 */

#include "WordclockMain.h"

#include "Arduino.h"
#include "Display.h"
#include "Persistence.h"
#include "PowerSwitch.h"
#include "RealTimeClock.h"

/*
 * An instance method, though it reads no member: main() brings the firmware up through
 * the same object it ticks with Task(), which reads one. Whether this backend's
 * implementation happens to need a member is its own detail, and answering the check
 * would carry that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void WordclockMain::Init() {
  PowerSwitch::GetInstance().Init();
  Display::GetInstance().Init();
  Persistence::GetInstance().Load();
}
// NOLINTEND(readability-convert-member-functions-to-static)

void WordclockMain::Task() {
  RealTimeClock::GetInstance().Task();
  wc_scheduler_.Task();
}
