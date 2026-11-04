// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the display colour command.
 */

#ifndef _MSG_CMD_DISPLAY_COLOR_PARSER_H_
#define _MSG_CMD_DISPLAY_COLOR_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "MsgParameterParser.h"

/* MsgCmdDisplayColorParser configuration parameter */

/* MsgCmdDisplayColorParser parameter */
#define MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE           3u

class MsgCmdDisplayColorParser : public MsgParameterParser<MsgCmdDisplayColorParser, MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    using ColorType = Pixel::ColorType;

  private:
    friend class MsgParameterParser;
    static constexpr char kRedOptionShortName{'R'};
    static constexpr char kGreenOptionShortName{'G'};
    static constexpr char kBlueOptionShortName{'B'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kRedOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kGreenOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kBlueOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    static void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kRedOptionShortName) { Display::GetInstance().SetColorRed(argument); }
        if(parameter_short_name == kGreenOptionShortName) { Display::GetInstance().SetColorGreen(argument); }
        if(parameter_short_name == kBlueOptionShortName) { Display::GetInstance().SetColorBlue(argument); }
    }

    static void SendAnswerRed(bool append_space) { SendAnswerParameter(kRedOptionShortName, Display::GetInstance().GetColorRed(), append_space); }
    static void SendAnswerGreen(bool append_space) { SendAnswerParameter(kGreenOptionShortName, Display::GetInstance().GetColorGreen(), append_space); }
    static void SendAnswerBlue(bool append_space) { SendAnswerParameter(kBlueOptionShortName, Display::GetInstance().GetColorBlue(), append_space); }

    void Show() const
    {
        StdReturnType return_value = Display::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kDisplayShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

  public:
    constexpr MsgCmdDisplayColorParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdDisplayColorParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer()
    {
        SendAnswerRed(true);
        SendAnswerGreen(true);
        SendAnswerBlue(false);
    }

    void Process() const { Show(); }

};

#endif
