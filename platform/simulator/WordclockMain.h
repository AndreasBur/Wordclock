// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the simulator runs the firmware with: the host's time and the scheduler.
 */

#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H

#include "Scheduler.h"

/*
 * Adapts the wxWidgets side to the firmware: it hands the host's time to the real time
 * clock and drives the scheduler. What reaches the display is the DisplayManager's
 * decision, so nothing of that lives here.
 */
class WordclockMain
{
    public:
        WordclockMain();
        ~WordclockMain();

        void Task();

    protected:

    private:
        Scheduler wc_scheduler_;
        enum {
            kTimerId = 1500
        };
};

#endif // WORDCLOCKMAIN_H
