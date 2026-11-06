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

/*!
 * \brief The firmware as the AVR runs it: brought up once by Init(), then ticked by Task().
 *
 * main() keeps the one instance, calls Init() once and then Task() once per tick, every
 * Scheduler::GetTaskIntervalMs() milliseconds.
 */
class WordclockMain
{
  private:
    Scheduler wc_scheduler_;

  public:
    /*! \brief Makes the firmware's runner; it brings nothing up until Init(). */
    WordclockMain() { }
    /*! \brief Destroys the firmware's runner. */
    ~WordclockMain() = default;

    /*!
     * \brief Brings the supply and the display up, and restores the stored settings with
     *        Persistence.
     *
     * The supply switch before the display, and that order is the hardware's rather than a
     * preference: the display's Init() claims the strip's data line, and a frame reaching DIN
     * before the 5 V is up pushes current into a dead rail through the LED's own protection
     * diode. The reset leaves the supply off, so this is the call that asks for it.
     *
     * The display next, because it is what claims the strip; the settings after it, because
     * restoring the brightness recalculates what reaches the LEDs, and before the first task, so
     * the strip's first frame already carries the stored colour.
     *
     * The light sensor and the clock chip are not initialised here: the core keeps both inside
     * singletons of its own with no defined Init(), and each driver sets itself up from its first
     * task.
     */
    void Init();

    /*!
     * \brief Runs one tick: the real time clock's task, then the scheduler's.
     *
     * The clock before the scheduler, so that everything the tasks read in this pass sees one and
     * the same time rather than one that changes half way through it.
     */
    void Task();
};

#endif // _WORDCLOCK_MAIN_H_
