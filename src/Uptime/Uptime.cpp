// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The count of the time since the start.
 */

#include "Uptime.h"

/*
 * One place of the count: steps it and says whether it rolled into the next one. The
 * carry is the whole of the arithmetic here, so it is worth its own name - Task() then
 * reads as the three places it applies to.
 */
bool Uptime::Carry(byte& value, byte limit)
{
    value++;
    if(value < limit) { return false; }

    value = 0u;
    return true;
}

void Uptime::Task()
{
    if(!Carry(seconds_, kSecondsPerMinute)) { return; }
    if(!Carry(minutes_, kMinutesPerHour)) { return; }
    if(!Carry(hours_, kHoursPerDay)) { return; }

    if(days_ < kDaysMaximum) { days_++; }
}
