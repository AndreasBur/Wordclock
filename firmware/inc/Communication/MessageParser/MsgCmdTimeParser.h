// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the time command.
 */

#ifndef _MSG_CMD_TIME_PARSER_H_
#define _MSG_CMD_TIME_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "RealTimeClock.h"

/* MsgCmdTimeParser configuration parameter */

/* MsgCmdTimeParser parameter */
#define MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE           3u

class MsgCmdTimeParser : public MsgParameterParser<MsgCmdTimeParser, MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    using HourType = RealTimeClock::HourType;
    using MinuteType = RealTimeClock::MinuteType;
    using SecondType = RealTimeClock::SecondType;

  private:
    friend class MsgParameterParser;
    ClockTime time_;

    static constexpr char kHourOptionShortName{'H'};
    static constexpr char kMinuteOptionShortName{'M'};
    static constexpr char kSecondOptionShortName{'S'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kHourOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kMinuteOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kSecondOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kHourOptionShortName) {
            SetHour(argument);
        }
        if(parameter_short_name == kMinuteOptionShortName) {
            SetMinute(argument);
        }
        if(parameter_short_name == kSecondOptionShortName) {
            SetSecond(argument);
        }
    }

    void SendAnswerHour(bool append_space) const {
        SendAnswerParameter(kHourOptionShortName, time_.GetHour(), append_space);
    }
    void SendAnswerMinute(bool append_space) const {
        SendAnswerParameter(kMinuteOptionShortName, time_.GetMinute(), append_space);
    }
    void SendAnswerSecond(bool append_space) const {
        SendAnswerParameter(kSecondOptionShortName, time_.GetSecond(), append_space);
    }

    void SetHour(HourType hour) {
        StdReturnType return_value = time_.SetHour(hour);
        error_.CheckReturnValueAndSend(kHourOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetMinute(MinuteType minute) {
        StdReturnType return_value = time_.SetMinute(minute);
        error_.CheckReturnValueAndSend(kMinuteOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetSecond(SecondType second) {
        StdReturnType return_value = time_.SetSecond(second);
        error_.CheckReturnValueAndSend(kSecondOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

  public:
    MsgCmdTimeParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) {
        time_ = RealTimeClock::GetInstance().GetTime();
    }
    ~MsgCmdTimeParser() { }

    // get methods

    // set methods

    // methods
    void SendAnswer() const
    {
        SendAnswerHour(true);
        SendAnswerMinute(true);
        SendAnswerSecond(false);
    }

    void Process()
    {
        RealTimeClock::GetInstance().SetTime(time_);
        time_ = RealTimeClock::GetInstance().GetTime();
    }
};

#endif
