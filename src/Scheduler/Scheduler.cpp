// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The scheduler's tick and the order in which it calls the module tasks.
 */

#include "Scheduler.h"
#include "StandardTypes.h"
#include "Animations.h"
#include "DisplayManager.h"
#include "Illuminance.h"
#include "Communication.h"
#include "Overlays.h"
#include "Persistence.h"
#include "NightSwitch.h"
#include "ColorCycle.h"
#include "Power.h"
#include "Uptime.h"
#include "Temperature.h"
#include "Text.h"

void Scheduler::Task()
{
    TriggerTasks();
}

/*
 * Tells whether a task is due on this tick - true if it is to run - and counts it down if not.
 * A cycle of zero switches the task off. Its remaining ticks are cleared along with it, so
 * switching it back on runs it right away instead of somewhere inside a period left over from
 * before.
 */
bool Scheduler::IsDue(TaskIdType task_id, byte cycle) {
    if(cycle == 0u) { remaining_ticks_[ToUnderlying(task_id)] = 0u; return false; }
    if(remaining_ticks_[ToUnderlying(task_id)] > 1u) { remaining_ticks_[ToUnderlying(task_id)]--; return false; }

    // Reloading here rather than on the previous run is what makes a cycle that changed
    // in between take effect. Starting from zero, every task is due on the first tick.
    remaining_ticks_[ToUnderlying(task_id)] = cycle;
    return true;
}

void Scheduler::TriggerTasks()
{
    if(IsDue(TaskIdType::kIlluminance, Illuminance::GetInstance().GetTaskCycle())) { Illuminance::GetInstance().Task(); }
    // Beside the light sensor because it is one too, and nothing downstream is timed
    // against it: the overlay reads whatever the last pass left.
    if(IsDue(TaskIdType::kTemperature, Temperature::GetInstance().GetTaskCycle())) { Temperature::GetInstance().Task(); }
    /* after the sensor, so the brightness automatic works on a fresh reading */
    if(IsDue(TaskIdType::kDisplayManager, DisplayManager::GetInstance().GetTaskCycle())) { DisplayManager::GetInstance().Task(); }
    if(IsDue(TaskIdType::kAnimations, Animations::GetInstance().GetTaskCycle())) { Animations::GetInstance().Task(true); }
    if(IsDue(TaskIdType::kCommunication, Communication::GetInstance().GetTaskCycle())) { Communication::GetInstance().Task(); }
    if(IsDue(TaskIdType::kOverlays, Overlays::GetInstance().GetTaskCycle())) { Overlays::GetInstance().Task(); }
    if(IsDue(TaskIdType::kText, Text::GetInstance().GetTaskCycle())) { Text::GetInstance().Task(true); }
    // last, so what it compares against the store is a settled state rather than one from
    // the middle of this tick's pass
    if(IsDue(TaskIdType::kPersistence, Persistence::GetInstance().GetTaskCycle())) { Persistence::GetInstance().Task(); }
    if(IsDue(TaskIdType::kUptime, Uptime::GetInstance().GetTaskCycle())) { Uptime::GetInstance().Task(); }
    if(IsDue(TaskIdType::kNightSwitch, NightSwitch::GetInstance().GetTaskCycle())) { NightSwitch::GetInstance().Task(); }
    /*
     * After the night switch, which is what may have switched the display off this tick: a
     * cycle stepping after that would mark the buffer for a display that is meant to be
     * dark, and the supply switch below waits for a buffer that has settled.
     */
    if(IsDue(TaskIdType::kColorCycle, ColorCycle::GetInstance().GetTaskCycle())) { ColorCycle::GetInstance().Task(); }
    /*
     * Last of all, and after the night switch in particular: everything above may still
     * write pixels in this pass, and what this one waits for is the pass being over - a
     * supply cut before the last writer would leave the strip holding a frame nobody
     * finished. The application renders after this returns, which is the transmission this
     * task is watching for on its next run.
     */
    if(IsDue(TaskIdType::kPower, Power::GetInstance().GetTaskCycle())) { Power::GetInstance().Task(); }
}
