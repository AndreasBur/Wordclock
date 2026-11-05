// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the simulator runs the firmware with: the host's time and the scheduler.
 */

#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H

#include "Scheduler.h"

/*!
 * \brief The firmware as the simulator runs it: the real time clock and the scheduler.
 *
 * Adapts the wxWidgets side to the firmware: it hands the host's time to the real time clock and
 * drives the scheduler. What reaches the display is the DisplayManager's decision, so nothing of
 * that lives here.
 */
class WordclockMain
{
    public:
        /*!
         * \brief Starts the firmware: sets the real time clock to the host's local time,
         *        initialises the power switch and restores the stored settings with Persistence.
         *
         * The host's time is read this once; from here the real time clock keeps it.
         */
        WordclockMain();
        /*! \brief Destroys the firmware's runner. */
        ~WordclockMain();

        /*! \brief Runs one tick: the real time clock's task, then the scheduler's. */
        void Task();

    protected:

    private:
        Scheduler wc_scheduler_;
        enum {
            kTimerId = 1500
        };
};

#endif // WORDCLOCKMAIN_H
