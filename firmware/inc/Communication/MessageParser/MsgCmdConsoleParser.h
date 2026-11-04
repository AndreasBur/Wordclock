// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Sets the password the web console asks for, or takes it away.
 *
 * One option in, one answer out, and they are not the same field. The password goes in and is never
 * read back, exactly as the WiFi pass phrase is not - what comes back is whether one is set at all,
 * which is the only thing somebody needs to know from the outside.
 *
 * Sent without a password, the command is meant to clear it so that the console answers everybody
 * again. That is the way back for a clock whose password was forgotten, and it is why this command
 * is worth having on the serial line rather than only in the console it locks: the console would
 * ask for the very password that is missing. As the parser stands, no command line clears it; see
 * MsgCmdConsoleParser.
 */

#ifndef _MSG_CMD_CONSOLE_PARSER_H_
#define _MSG_CMD_CONSOLE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "System.h"

/*! \brief The number of options in the parameter table of the console command. */
#define MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE             1u

/*!
 * \brief The parser of command 16, the console command: sets the password the web console asks for.
 *
 * Its option:
 * - `-P<password>`, string: stores the password through System::SetConsolePassword(). It is
 *   stored on the ESP32 and the RP2350; the AVR and the simulator refuse it, and keep no password.
 *   A refusal is not answered.
 *
 * The answer says whether a password is set, never which one.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies it at once. MsgCmdParser makes the parser and calls Parse(),
 * Process() and SendAnswer() in this order, so an error answered for an option stands in the line
 * before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 *
 * \warning The password is stored from the first character after `-P` to the end of the line, not
 *          to the next option: `16 -Psecret -A1` stores `secret -A1`, and answers `Error=3:A` as
 *          well. And no command line clears the password, though the panel sends `16 -P` for an
 *          empty password field: an option that ends the line is not read, see
 *          MsgParameterParser::Parse(), and `16` alone carries no option.
 */
class MsgCmdConsoleParser : public MsgParameterParser<MsgCmdConsoleParser, MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE>
{
  private:
    friend class MsgParameterParser;

    static constexpr char kPasswordShortName{'P'};
    static constexpr char kProtectedShortName{'A'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kPasswordShortName, MsgParameter::ArgumentTypeType::kString)
    };

    static void HandleParameter(char parameter_short_name, byte argument) {
        UNUSED(parameter_short_name); UNUSED(argument);
    }
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length)
    {
        UNUSED(length);

        if(parameter_short_name == kPasswordShortName) { System::GetInstance().SetConsolePassword(argument); }
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdConsoleParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdConsoleParser() { }

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `A=<0|1>`: whether a password is set, and never which one. A space follows each
     * field but the last; MsgCmdParser ends the line.
     */
    static void SendAnswer() {
        SendAnswerParameter(kProtectedShortName,
                            static_cast<byte>(System::GetInstance().IsConsoleProtected() ? 1u : 0u), false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * Nothing is left to do: every option is applied as Parse() reads it. Nothing is deferred, as
     * the password has no second field it has to agree with.
     */
    static void Process() { }
};

#endif // _MSG_CMD_CONSOLE_PARSER_H_
