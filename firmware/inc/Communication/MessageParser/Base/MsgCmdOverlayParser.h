// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser the date, temperature and text overlay commands share.
 */

#ifndef _MSG_CMD_OVERLAY_PARSER_H_
#define _MSG_CMD_OVERLAY_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "MsgCmdBaseOverlayParser.h"
#include "Overlays.h"

/* MsgCmdOverlayParser configuration parameter */

/* MsgCmdOverlayParser parameter */

template <typename Derived> class MsgCmdOverlayParser
    : public MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>, public MsgCmdBaseOverlayParser
{
  public:
    using MsgParameterParserType = MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
    using PositionType = typename MsgParameterParserType::PositionType;
    using LengthType = StringTools::LengthType;

    using YearType = ClockDate::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;
    using FontType = Text::FontType;

  protected:
    // functions
    /*
     * Protected rather than private with the derived class as a friend, which is what
     * clang-tidy asks for: a friend would open this base's whole private side to it, where
     * protected opens only what is meant for it. What the private constructor would
     * prevent - somebody inheriting this template with another class's Derived - the
     * assertion catches instead, at the first point where Derived is a complete type.
     */
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
    constexpr MsgCmdOverlayParser(const char* parameter) : MsgParameterParserType(kParameterTable, parameter) {
        static_assert(std::is_base_of<MsgCmdOverlayParser, Derived>::value,
                      "MsgCmdOverlayParser is a CRTP base: Derived has to be the class that inherits it");
    }
    ~MsgCmdOverlayParser() { }

  private:
    friend class MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;

    // functions
    Derived& Underlying() { return static_cast<Derived&>(*this); }
    Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

    void HandleParameter(char parameter_short_name, byte argument) {
        if(parameter_short_name == kPeriodOptionShortName) { SetPeriodInMinutes(argument); }
        if(parameter_short_name == kEnduranceOptionShortName) { SetEnduranceInSeconds(argument); }
        if(parameter_short_name == kMonthOptionShortName) { Underlying().SetMonth(argument); }
        if(parameter_short_name == kDayOptionShortName) { Underlying().SetDay(argument); }
        if(parameter_short_name == kValidOptionShortName) { Underlying().SetValidInDays(argument); }
        if(parameter_short_name == kActiveOptionShortName) { Underlying().SetIsActive(static_cast<bool>(argument)); }
        if(parameter_short_name == kSpeedOptionShortName) { Underlying().SetSpeed(argument); }
        if(parameter_short_name == kFontOptionShortName) { Underlying().SetFont(static_cast<FontType>(argument)); }
    }

    void HandleParameter(char parameter_short_name, const char* argument, PositionType length) {
        if(parameter_short_name == kTextOptionShortName) { Underlying().SetText(argument, length); }
    }

    void SetPeriodInMinutes(MinuteType period_in_minutes) {
        StdReturnType return_value = Underlying().SetPeriodInMinutes(period_in_minutes);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kPeriodOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetEnduranceInSeconds(SecondType endurance_in_seconds) {
        StdReturnType return_value = Underlying().SetEnduranceInSeconds(endurance_in_seconds);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kEnduranceOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetFont(FontType font) {
        StdReturnType return_value = Underlying().SetFont(font);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kFontOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);

    }

    void SendAnswerPeriod(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kPeriodOptionShortName,  Underlying().GetPeriodInMinutes(), append_space); }
    void SendAnswerEndurance(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kEnduranceOptionShortName, Underlying().GetEnduranceInSeconds(), append_space); }
    void SendAnswerMonth(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kMonthOptionShortName, Underlying().GetMonth(), append_space); }
    void SendAnswerDay(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kDayOptionShortName, Underlying().GetDay(), append_space); }
    void SendAnswerValid(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kValidOptionShortName, Underlying().GetValidInDays(), append_space); }
    void SendAnswerSpeed(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kSpeedOptionShortName, Underlying().GetSpeed(), append_space); }
    void SendAnswerFont(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kFontOptionShortName, ToUnderlying(Underlying().GetFont()), append_space); }
    void SendAnswerActive(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kActiveOptionShortName, Underlying().GetIsActive(), append_space); }
    void SendAnswerText(bool append_space) const { Underlying().SendAnswerText(append_space); }

  public:

    // get methods

    // set methods

    // methods
    void SendAnswer() const
    {
        SendAnswerPeriod(true);
        SendAnswerEndurance(true);
        SendAnswerMonth(true);
        SendAnswerDay(true);
        SendAnswerValid(true);
        SendAnswerText(true);
        SendAnswerSpeed(true);
        SendAnswerFont(true);
        SendAnswerActive(false);
    }

    static void Process() {  }

};

#endif
