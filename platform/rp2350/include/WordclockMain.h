// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Adapts the ESP32 application to the firmware core.
 *
 * The counterpart of the simulator's WordclockMain, and deliberately as thin: it takes the time
 * from the real time clock and drives the scheduler. What reaches the display is the
 * DisplayManager's decision, so nothing of that lives here.
 *
 * Unlike on the simulator there is nothing to seed - the real time clock reads the system clock,
 * which SNTP sets.
 */

#ifndef _WORDCLOCK_MAIN_H_
#define _WORDCLOCK_MAIN_H_

#include "Scheduler.h"

class WordclockMain
{
    public:
        WordclockMain() { }
        ~WordclockMain() { }

        void Init();
        void Task();

    private:
        Scheduler wc_scheduler_;
};

#endif // _WORDCLOCK_MAIN_H_
