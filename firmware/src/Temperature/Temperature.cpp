// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How a reading is written as text.
 */

#define _TEMPERATURE_SOURCE_

#include "Temperature.h"

// Both public forms come through here, so the digits cannot drift apart between what
// is shown and what is reported - only the degree sign differs.
StdReturnType Temperature::AppendReading(char* string, bool with_degree_sign) const
{
    TemperatureType tenths{0};

    string[0u] = STD_NULL_CHARACTER;
    if(GetTemperature(tenths) == kENotOk) { return kENotOk; }

    char* position = string;
    position = AppendSign(tenths, position);
    position = AppendNumber(WholeDegrees(tenths), position);
    position = AppendChar(kDecimalPoint, position);
    position = AppendNumber(TenthOfDegree(tenths), position);
    position = AppendDegreeSign(with_degree_sign, position);
    position = AppendChar(kDegreeUnit, position);
    *position = STD_NULL_CHARACTER;
    return kEOk;
}
