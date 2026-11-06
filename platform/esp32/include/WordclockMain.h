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

/*!
 * \brief The firmware as the ESP32 runs it: brought up once by Init(), then ticked by Task().
 *
 * setup() in main.cpp calls Init() once, and loop() calls Task() once per tick, every
 * Scheduler::GetTaskIntervalMs() milliseconds.
 */
class WordclockMain
{
    public:
        /*! \brief Makes the firmware's runner; it brings nothing up until Init(). */
        WordclockMain() { }
        /*! \brief Destroys the firmware's runner. */
        ~WordclockMain() = default;

        /*!
         * \brief Brings the supply and the display up, and restores the stored settings with
         *        Persistence.
         *
         * The supply switch first, because the display's Init() claims the strip's data line, and
         * a frame reaching it before the 5 V is up pushes current into a dead rail. The settings
         * after the display, because restoring the brightness recalculates what reaches the LEDs,
         * so the strip's first frame already carries the stored colour.
         *
         * The display's Init() is what claims the RMT channel for the strip's data pin. The
         * simulator gets away without this call because its stand-ins need no setting up, which
         * is why nothing in the core makes it; on hardware, without it every Show() is quietly
         * dropped.
         *
         * The light sensor is not initialised here on purpose: Illuminance keeps its BH1750
         * private and has no defined Init(), so the driver sets itself up from its first Task().
         */
        void Init();

        /*!
         * \brief Runs one tick: the real time clock's task, then the scheduler's.
         *
         * The clock before the scheduler, so that every task of this pass reads the same time.
         */
        void Task();

    private:
        Scheduler wc_scheduler_;
};

#endif // _WORDCLOCK_MAIN_H_
