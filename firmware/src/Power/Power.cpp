// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The steps of switching the strip's supply off and on.
 */

#define _POWER_SOURCE_

#include "Power.h"
#include "Display.h"

namespace {
Display& GetDisplay() { return Display::GetInstance(); }
}

/*
 * Both waits, as one question. A buffer still marked dirty means the blanked frame has
 * not been handed to the strip yet; a frame on the wire means it has and is still going
 * out. Only when neither holds is the strip dark and idle.
 *
 * Anything still drawing - an animation that is running out, the clock changing minute -
 * marks the buffer again and makes this wait another tick. That is left as it is rather
 * than cut short: the display is dark from the first step onwards, so what a longer wait
 * postpones is the current the switch saves, not the darkness somebody asked for.
 */
bool Power::IsStripDarkAndIdle()
{
    if(GetDisplay().IsDirty()) { return false; }

    return !GetDisplay().IsFrameOnTheWire();
}

// The gate before the port, so that no tick can slip a frame in between the two.
void Power::CutSupply()
{
    GetDisplay().SuspendOutput();
    GetSwitch().SwitchSupplyOff();
    state_ = StateType::kSupplyOff;
}

// The port before the gate, for the same reason read the other way round.
void Power::ReleaseSupply()
{
    GetDisplay().ResumeOutput();
    GetDisplay().Enable();
    state_ = StateType::kSupplyOn;
}

StdReturnType Power::SwitchSupplyOn()
{
    if constexpr (!IsSwitchFitted()) { return kENotOk; }

    switch(state_) {
        case StateType::kSupplyOff :
            // The port first; the data line follows on the next task.
            GetSwitch().SwitchSupplyOn();
            state_ = StateType::kSettling;
            break;
        // Never got as far as cutting it, so there is nothing to bring up and nothing
        // to settle - only the darkness to undo.
        case StateType::kBlanking :
            ReleaseSupply();
            break;
        case StateType::kSupplyOn :
        case StateType::kSettling :
            break;
    }

    return kEOk;
}

StdReturnType Power::SwitchSupplyOff()
{
    if constexpr (!IsSwitchFitted()) { return kENotOk; }

    switch(state_) {
        case StateType::kSupplyOn :
            // Over the data line, not by cutting the supply: this is the frame the
            // strip has to be holding when its rail goes.
            GetDisplay().Disable();
            state_ = StateType::kBlanking;
            break;
        // Asked while the supply is coming up. The data line is still gated, so the
        // strip is dark already and the port can go straight back down.
        case StateType::kSettling :
            GetSwitch().SwitchSupplyOff();
            state_ = StateType::kSupplyOff;
            break;
        case StateType::kBlanking :
        case StateType::kSupplyOff :
            break;
    }

    return kEOk;
}

void Power::Task()
{
    switch(state_) {
        case StateType::kBlanking :
            if(IsStripDarkAndIdle()) { CutSupply(); }
            break;
        case StateType::kSettling :
            ReleaseSupply();
            break;
        case StateType::kSupplyOn :
        case StateType::kSupplyOff :
            break;
    }
}
