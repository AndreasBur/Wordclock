// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the temperature overlay command.
 */

#ifndef _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_
#define _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"

/* MsgCmdOverlayTemperatureParser configuration parameter */

/* MsgCmdOverlayTemperatureParser parameter */

class MsgCmdOverlayTemperatureParser : public MsgCmdOverlayParser<MsgCmdOverlayTemperatureParser>
{
  public:

  private:
    friend class MsgCmdOverlayParser;

  public:
    constexpr MsgCmdOverlayTemperatureParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }
    ~MsgCmdOverlayTemperatureParser() { }

    // get methods

    // set methods

    // methods
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetTemperaturePeriodInMinutes(); }
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetTemperatureEnduranceInSeconds(); }
    static MonthType GetMonth() { return Overlays::GetInstance().GetTemperatureMonth(); }
    static DayType GetDay() { return Overlays::GetInstance().GetTemperatureDay(); }
    static DayType GetValidInDays() { return Overlays::GetInstance().GetTemperatureValidInDays(); }
    static bool GetIsActive() { return Overlays::GetInstance().GetTemperatureIsActive(); }
    static byte GetSpeed() { return Overlays::GetInstance().GetTemperatureSpeed(); }
    static FontType GetFont() {return Overlays::GetInstance().GetTemperatureFont(); }

    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetTemperaturePeriodInMinutes(period_in_minutes); }
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) {return Overlays::GetInstance().SetTemperatureEnduranceInSeconds(endurance_in_seconds); }
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetTemperatureMonth(month); }
    static void SetDay(DayType day) { Overlays::GetInstance().SetTemperatureDay(day); }
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetTemperatureValidInDays(valid_in_days); }
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetTemperatureSpeed(speed); }
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetTemperatureFont(font); }
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetTemperatureIsActive(is_active); }
    static void SetText(const char* text, LengthType length) { UNUSED(text); UNUSED(length); }

    static void SendAnswerText(bool append_space) { UNUSED(append_space); }
    static void SendAnswerSpeedText(bool append_space) { UNUSED(append_space); }

};

#endif
