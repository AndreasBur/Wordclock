// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The firmware, brought up and ticked.
 *
 * The same shape as the simulator's and the ESP32's: what has to be initialised, and what has to
 * happen once per tick. Keeping the three alike is what lets a change to the start-up order be made
 * in one place and checked in the simulator.
 */

#ifndef _WORDCLOCK_MAIN_H_
#define _WORDCLOCK_MAIN_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Scheduler.h"

class WordclockMain
{
  private:
    Scheduler wc_scheduler_;

  public:
    WordclockMain() { }
    ~WordclockMain() { }

    void Init();
    void Task();
};

#endif // _WORDCLOCK_MAIN_H_
