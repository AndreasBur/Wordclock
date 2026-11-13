// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the night window does at its edges, and how it reads the time.
 */

#include "NightSwitch.h"
#include "Display.h"
#include "Power.h"
#include "RealTimeClock.h"

/*
 * Power is asked rather than the port written: it darkens the strip over the data line
 * first and waits for that frame to leave the wire, so the rail is still up when this
 * returns and the sequence finishes on the ticks after it.
 *
 * Where no switch is fitted the display is darkened the way it always was, and the strip
 * keeps its current because on that board it cannot do otherwise - which is the same
 * answer the procedures give, rather than a night that quietly does less than it says.
 */
void NightSwitch::CutSupplyForTheNight()
{
    if constexpr (!Power::IsSwitchFitted()) { Display::GetInstance().Disable(); return; }

    Power::GetInstance().SwitchSupplyOff();
    supply_cut_for_night_ = true;
}

// Ahead of the enable and not after it, because the rail has to be up before a frame
// goes out - Power keeps the data line gated until it is, and releases it a tick later.
void NightSwitch::RestoreSupplyAfterTheNight()
{
    if(!supply_cut_for_night_) { return; }

    Power::GetInstance().SwitchSupplyOn();
    supply_cut_for_night_ = false;
}

void NightSwitch::ApplyNight()
{
    if(IsDisplayOffAtNight(night_brightness_)) { CutSupplyForTheNight(); }
    else                                     { Display::GetInstance().SetBrightnessNightLevel(night_brightness_); }
}

/*
 * Both undone rather than only the one that was applied: the night brightness may have
 * been changed while the window was open, and then the morning has to put back
 * whichever of the two the evening happened to use.
 */
void NightSwitch::ApplyDay()
{
    Display::GetInstance().ClearBrightnessNight();
    RestoreSupplyAfterTheNight();
    Display::GetInstance().Enable();
}

/*
 * The one thing watched as a state rather than as an edge, and only because a cut rail
 * makes it a different question. Between two crossings the display is whatever anybody
 * last made it - but where the night took the supply away, switching the clock on reaches
 * a gated data line and an unpowered strip, so the wall stays dark and the switch reads
 * as broken. Giving the rail back is what makes that hand mean something at two in the
 * morning; it is the crossing rule kept rather than an exception to it.
 *
 * Nothing here cuts the supply again. A display switched off by hand a minute later keeps
 * its rail until the window's next edge, for the same reason the rest of this module acts
 * on crossings: a clock that undid what a hand just did would read as a fault.
 */
void NightSwitch::FollowTheDisplayBackOn()
{
    if(!supply_cut_for_night_) { return; }
    if(!Display::GetInstance().IsEnabled()) { return; }

    RestoreSupplyAfterTheNight();
}

bool NightSwitch::IsNight() const
{
    const ClockTime now = RealTimeClock::GetInstance().GetTime();

    return IsWithinWindow(ToTimeOfDay(now.GetHour(), now.GetMinute()),
                          ToTimeOfDay(start_hour_, start_minute_),
                          ToTimeOfDay(end_hour_, end_minute_));
}

void NightSwitch::SetIsActive(bool is_active)
{
    is_active_ = is_active;
    if(!is_active_) { ApplyDay(); }
    was_night_ = false;
}

void NightSwitch::ResetToDefaults()
{
    SetIsActive(kIsActiveInitValue);
    start_hour_ = kStartHourInitValue;
    start_minute_ = kStartMinuteInitValue;
    end_hour_ = kEndHourInitValue;
    end_minute_ = kEndMinuteInitValue;
    night_brightness_ = kNightBrightnessInitValue;
}

void NightSwitch::Task()
{
    if(!is_active_) { return; }

    const bool is_night_now = IsNight();

    if(is_night_now == was_night_) { FollowTheDisplayBackOn(); return; }
    was_night_ = is_night_now;

    if(is_night_now) { ApplyNight(); }
    else           { ApplyDay(); }
}
