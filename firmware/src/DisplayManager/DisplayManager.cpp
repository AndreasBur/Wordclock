// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Decides what the display shows.
 */

#define _DISPLAY_MANAGER_SOURCE_

#include "DisplayManager.h"

/*! \brief          Decides what the display is showing */
void DisplayManager::Task()
{
    Display::GetInstance().ApplyBrightness();

    ClockTime time = RealTimeClock::GetInstance().GetTime();

    // An overlay owns the display while it shows: Text draws its letters there and the
    // clock must not write over them.
    if(IsOverlayShowing()) {
        overlay_was_showing_ = true;
        return;
    }

    /*
     * The overlay has ended. Nothing else brings the clock back — Overlay's transition
     * to idle only stops the Text — so without this the grid would keep the last
     * overlay frame until the next word change, which can be nearly five minutes away.
     * Updating last_clock_words_ keeps that change from drawing a second time.
     */
    if(overlay_was_showing_) {
        overlay_was_showing_ = false;
        UpdateClockWords(time.GetHour(), time.GetMinute());
        ShowClock(time.GetHour(), time.GetMinute());
        return;
    }

    if(UpdateClockWords(time.GetHour(), time.GetMinute())) {
        TaskClockWordsChange(time.GetHour(), time.GetMinute());
    }
}
