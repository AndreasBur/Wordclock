// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Adapts the ESP32 application to the firmware core.
 */

#include "WordclockMain.h"

#include "Display.h"
#include "Persistence.h"
#include "PowerSwitch.h"
#include "RealTimeClock.h"

/*
 * An instance method, though it reads no member: it is part of the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void WordclockMain::Init() {
  /*
   * Before the display, and that order is the hardware's rather than a preference: the call
   * below claims the strip's data line, and a frame reaching DIN before the 5 V is up pushes
   * current into a dead rail through the LED's own protection diode. The reset leaves the
   * supply off, so this is the call that asks for it.
   */
  PowerSwitch::GetInstance().Init();
  Display::GetInstance().Init();
  /*
   * After the display, because restoring the brightness recalculates what reaches the
   * LEDs, and before the first task, so the strip's first frame already carries the
   * stored colour.
   */
  Persistence::GetInstance().Load();
}
// NOLINTEND(readability-convert-member-functions-to-static)

void WordclockMain::Task() {
  RealTimeClock::GetInstance().Task();
  wc_scheduler_.Task();
}
