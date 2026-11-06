// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How the simulator starts the firmware and runs a task of it.
 */

#include "WordclockMain.h"

#include "Persistence.h"
#include "sim/PowerSwitch.h"
#include "sim/RealTimeClock.h"

WordclockMain::WordclockMain()
{
    RealTimeClock::GetInstance().SetDateTime(RealTimeClock::ToClockDateTime(wxDateTime::Now()));
    /*
     * Nothing to set up here - there is no port - but the call is made anyway, so that the
     * one backend the others are mirrored from shows where it belongs. On hardware it is what
     * brings the strip's 5 V up, and it has to come before the data line is claimed.
     */
    PowerSwitch::GetInstance().Init();
    // Before the first task, so the window's first paint already shows the restored
    // colour and mode rather than the defaults for a tick.
    Persistence::GetInstance().Load();
}

WordclockMain::~WordclockMain()
{
}

void WordclockMain::Task()
{
    RealTimeClock::GetInstance().Task();
    wc_scheduler_.Task();
}
