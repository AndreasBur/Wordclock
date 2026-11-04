// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the clock mode command.
 */

#ifndef _MSG_CMD_CLOCK_MODE_PARSER_H_
#define _MSG_CMD_CLOCK_MODE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Clock.h"

/* MsgCmdClockModeParser configuration parameter */

/* MsgCmdClockModeParser parameter */
#define MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE           2u

class MsgCmdClockModeParser : public MsgParameterParser<MsgCmdClockModeParser, MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    using ModeType = Clock::ModeType;

  private:
    friend class MsgParameterParser;

    static constexpr char kModeOptionShortName{'M'};
    static constexpr char kShowItIsOptionShortName{'I'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kModeOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kShowItIsOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kModeOptionShortName) {
            SetMode(static_cast<ModeType>(argument));
        }
        if(parameter_short_name == kShowItIsOptionShortName) {
            Clock::GetInstance().SetShowItIsPermanently(argument != 0u);
        }
    }

    void SetMode(ModeType mode) const
    {
        StdReturnType return_value = Clock::GetInstance().SetMode(mode);
        error_.CheckReturnValueAndSend(kModeOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void Show() const
    {
        StdReturnType return_value = Clock::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kClockShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

  public:
    constexpr MsgCmdClockModeParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdClockModeParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer() {
        SendAnswerParameter(kModeOptionShortName, ToUnderlying(Clock::GetInstance().GetMode()));
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        SendAnswerParameter(kShowItIsOptionShortName, static_cast<byte>(Clock::GetInstance().GetShowItIsPermanently() ? 1u : 0u), false);
    }

    void Process() const { Show(); }
};

#endif
