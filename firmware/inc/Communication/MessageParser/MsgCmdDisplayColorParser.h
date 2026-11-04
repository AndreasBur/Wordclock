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

/*! \brief The number of options in the parameter table of the display colour command. */
#define MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE           3u

/*!
 * \brief The parser of command 2, the display colour command: sets the colour the words are lit in.
 *
 * Its options:
 * - `-R<red>`, uint8: the red value of the colour, 0 to 255.
 * - `-G<green>`, uint8: the green value, 0 to 255.
 * - `-B<blue>`, uint8: the blue value, 0 to 255.
 *
 * Each value takes effect at once: the lit pixels are rewritten in the new colour. While the
 * colour cycle of command 15 runs, the words keep the colour of the cycle, and the colour set here
 * comes back when it stops. An option the command does not carry is left alone, so `2` alone is a
 * query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies it at once. MsgCmdParser makes the parser and calls Parse(),
 * Process() and SendAnswer() in this order, so an error answered for an option stands in the line
 * before the fields of the answer.
 */
class MsgCmdDisplayColorParser : public MsgParameterParser<MsgCmdDisplayColorParser, MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    /*! \brief A red, green or blue value, as the options take it: Pixel::ColorType. */
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
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdDisplayColorParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdDisplayColorParser() { }

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `R=<red> G=<green> B=<blue>`: the colour set here, not the one of a running colour
     * cycle. A space follows each field but the last; MsgCmdParser ends the line.
     */
    static void SendAnswer()
    {
        SendAnswerRed(true);
        SendAnswerGreen(true);
        SendAnswerBlue(false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It marks the display's buffer to be sent, through Display::Show(). A refusal would be
     * answered as an error of ErrorMessage::ApiType::kDisplayShow, but no backend refuses.
     */
    void Process() const { Show(); }

};

#endif
