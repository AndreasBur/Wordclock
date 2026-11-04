// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the display brightness command.
 */

#ifndef _MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_H_
#define _MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixel.h"
#include "Display.h"
#include "MsgParameterParser.h"

/* MsgCmdDisplayBrightnessParser configuration parameter */

/* MsgCmdDisplayBrightnessParser parameter */
#define MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE           3u

class MsgCmdDisplayBrightnessParser : public MsgParameterParser<MsgCmdDisplayBrightnessParser, MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE>
{
  public:

  private:
    friend class MsgParameterParser;
    static constexpr char kBrightnessOptionShortName{'B'};
    static constexpr char kUseAutomaticOptionShortName{'A'};
    static constexpr char kUseGammaCorrectionOptionShortName{'G'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        MsgParameter(kBrightnessOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        MsgParameter(kUseAutomaticOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        MsgParameter(kUseGammaCorrectionOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    static void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kBrightnessOptionShortName) {
            Display::GetInstance().SetBrightness(argument);
        }
        if(parameter_short_name == kUseAutomaticOptionShortName) {
            Display::GetInstance().SetBrightnessUseAutomatic(argument);
        }
        if(parameter_short_name == kUseGammaCorrectionOptionShortName) {
            Display::GetInstance().SetBrightnessUseGammaCorrection(argument);
        }
    }

     static void SendAnswerBrightness(bool append_space) { SendAnswerParameter(kBrightnessOptionShortName, Display::GetInstance().GetBrightness(), append_space); }
     static void SendAnswerAutomatic(bool append_space) { SendAnswerParameter(kUseAutomaticOptionShortName, Display::GetInstance().GetBrightnessUseAutomatic(), append_space); }
     static void SendAnswerGammaCorrection(bool append_space) { SendAnswerParameter(kUseGammaCorrectionOptionShortName, Display::GetInstance().GetBrightnessUseGammaCorrection(), append_space); }

    void Show() const
    {
        StdReturnType return_value = Display::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kDisplayShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

  public:
    constexpr MsgCmdDisplayBrightnessParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdDisplayBrightnessParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer()
    {
        SendAnswerBrightness(true);
        SendAnswerAutomatic(true);
        SendAnswerGammaCorrection(false);

    }

    void Process() { Show(); }

};

#endif
