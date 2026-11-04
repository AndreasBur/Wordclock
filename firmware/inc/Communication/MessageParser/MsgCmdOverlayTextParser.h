// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the text overlay command.
 */

#ifndef _MSG_CMD_OVERLAY_TEXT_PARSER_H_
#define _MSG_CMD_OVERLAY_TEXT_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"

/* MsgCmdOverlayTextParser configuration parameter */

/* MsgCmdOverlayTextParser parameter */

class MsgCmdOverlayTextParser : public MsgCmdOverlayParser<MsgCmdOverlayTextParser>
{
  public:

  private:
    friend class MsgCmdOverlayParser;

    // functions

  public:
    constexpr MsgCmdOverlayTextParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }
    ~MsgCmdOverlayTextParser() { }

    // get methods

    // set methods

    // methods
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetTextPeriodInMinutes(); }
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetTextEnduranceInSeconds(); }
    static MonthType GetMonth() { return Overlays::GetInstance().GetTextMonth(); }
    static DayType GetDay() { return Overlays::GetInstance().GetTextDay(); }
    static DayType GetValidInDays() { return Overlays::GetInstance().GetTextValidInDays(); }
    static byte GetSpeed() { return Overlays::GetInstance().GetTextSpeed(); }
    static FontType GetFont() {return Overlays::GetInstance().GetTextFont(); }
    static bool GetIsActive() { return Overlays::GetInstance().GetTextIsActive(); }
    static const char* GetText() { return Overlays::GetInstance().GetTextText(); }

    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetTextPeriodInMinutes(period_in_minutes); }
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) { return Overlays::GetInstance().SetTextEnduranceInSeconds(endurance_in_seconds); }
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetTextMonth(month); }
    static void SetDay(DayType day) { Overlays::GetInstance().SetTextDay(day); }
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetTextValidInDays(valid_in_days); }
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetTextSpeed(speed); }
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetTextFont(font); }
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetTextIsActive(is_active); }
    static void SetText(const char* text, LengthType length) { Overlays::GetInstance().SetTextText(text, length); }

    static void SendAnswerText(bool append_space) { SendAnswerParameter(kTextOptionShortName, GetText(), append_space); }
    static void SendAnswerSpeed(bool append_space) { SendAnswerParameter(kSpeedOptionShortName, GetSpeed(), append_space); }

};

#endif
