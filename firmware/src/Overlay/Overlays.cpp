// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the overlays do as a whole.
 */

#define _OVERLAYS_SOURCE_

#include "Overlays.h"
#include "Animations.h"

// Disabled is what is left over when none of the overlays has anything to do, which is
// why it needs no test of its own.
Overlays::StateType Overlays::GetState() const {
    if(IsShow()) { return OverlayType::StateType::kShow; }
    if(IsIdle()) { return OverlayType::StateType::kIdle; }
    return OverlayType::StateType::kDisabled;
}

void Overlays::Task() {
    ClockDate date = RealTimeClock::GetInstance().GetDate();
    ClockTime time = RealTimeClock::GetInstance().GetTime();

    if(last_second_ != time.GetSecond()) {
        last_second_ = time.GetSecond();
        if(IsShow()) { TaskShow(date, time); }
        else {
            // The timer counts a show and nothing else; with none showing it is 0, however the
            // last one ended. An overlay switched off while it showed left it standing, and no
            // overlay started by its period again.
            show_timer_in_seconds_ = 0u;
            TaskIdle(date, time);
            if(IsShow()) { AbortAnimation(); }
        }
    }
}

StdReturnType Overlays::Abort() {
    if(!IsShow()) { return kENotOk; }

    ClockDate date = RealTimeClock::GetInstance().GetDate();
    ClockTime time = RealTimeClock::GetInstance().GetTime();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(date_.GetState() == OverlayDate::StateType::kShow) { date_.StopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(temperature_.GetState() == OverlayTemperature::StateType::kShow) { temperature_.StopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(text_.GetState() == OverlayText::StateType::kShow) { text_.StopShow(date, time); }
#endif

    /*
     * The timer is cleared here rather than left to run out: it is what TaskIdle() reads
     * to decide whether an overlay may start, and only TaskShow() counts it down, so a
     * leftover value would keep every overlay from starting in its period until a "show
     * now" call replaced it.
     */
    show_timer_in_seconds_ = 0u;
    return kEOk;
}

// In the order date, temperature, text: the first overlay that starts sets the timer, and
// a timer that is not 0 keeps the ones behind it from starting in the same second.
void Overlays::TaskIdle(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if((date_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ = date_.Task(show_timer_in_seconds_, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if((temperature_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ =  temperature_.Task(show_timer_in_seconds_, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if((text_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ =  text_.Task(show_timer_in_seconds_, date, time);
    }
#endif
}

void Overlays::TaskShow(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(date_.GetState() == OverlayDate::StateType::kShow) { show_timer_in_seconds_ = date_.Task(show_timer_in_seconds_, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(temperature_.GetState() == OverlayTemperature::StateType::kShow) { show_timer_in_seconds_ =  temperature_.Task(show_timer_in_seconds_, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(text_.GetState() == OverlayText::StateType::kShow) { show_timer_in_seconds_ =  text_.Task(show_timer_in_seconds_, date, time); }
#endif
}

/*
 * An overlay that starts ends a running animation: both draw on the same display, and an
 * animation that ran past second 30 - Fade at its default speed takes some 51 seconds, any of
 * them minutes at a low one - drew its pixels into the scrolling text. The overlay holds the
 * display, as DisplayManager already has it; once it ends, DisplayManager draws the time anew.
 */
void Overlays::AbortAnimation()
{
    Animations::GetInstance().Abort();
}
