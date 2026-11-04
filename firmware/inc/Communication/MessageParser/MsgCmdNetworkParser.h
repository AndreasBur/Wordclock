// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Which network the clock joins.
 *
 * The credentials used to be compiled in, which made every change of network a reflash. They are
 * stored on the platform now and set through this command, over whichever console can reach the
 * clock - the serial line, or the web console on the access point the clock opens when it has no
 * network to join.
 *
 * The pass phrase goes in and never comes back out: the answer names the network and says whether
 * one is configured at all. A console that echoed it would put it in the scrollback of every
 * browser that watched.
 *
 * Both are taken as they arrive and applied together, because half a pair is a clock that cannot
 * join anything.
 */

#ifndef _MSG_CMD_NETWORK_PARSER_H_
#define _MSG_CMD_NETWORK_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "StringTools.h"
#include "System.h"

#include <string.h>

/*! \brief The number of options in the parameter table of the network command. */
#define MSG_CMD_NETWORK_PARSER_PARAMETER_TABLE_SIZE                 2u

/*!
 * \brief The parser of command 13, the network command: sets the network the clock joins.
 *
 * Its options:
 * - `-S<name>`, string: the name of the network.
 * - `-P<pass phrase>`, string: its pass phrase.
 *
 * Each runs up to the next `-` or the end of the line, so it cannot hold a `-`. It is cut to
 * System::kSsidStringLength - 1 or System::kPasswordStringLength - 1 characters, and its trailing
 * spaces are taken off, so it cannot end in a space either.
 *
 * The two are stored together or not at all. A command with both has System store them and join
 * the network; one with only one of them is refused with `Error=8`, ErrorType::kUnknown, and
 * leaves the stored pair as it was; one with neither is a query. The AVR and the simulator refuse
 * every pair with `Error=8`.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 *
 * \warning An option that ends the line is not read, see MsgParameterParser::Parse(). So an empty
 *          pass phrase at the end is no pass phrase at all: `13 -SOpenNet -P` for an open network
 *          and `13 -S -P` to forget the network are both refused with `Error=8` as half a pair.
 */
class MsgCmdNetworkParser : public MsgParameterParser<MsgCmdNetworkParser, MSG_CMD_NETWORK_PARSER_PARAMETER_TABLE_SIZE>
{
  private:
    friend class MsgParameterParser;

    static constexpr char kSsidShortName{'S'};
    static constexpr char kPasswordShortName{'P'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kSsidShortName, MsgParameter::ArgumentTypeType::kString),
        ParameterTableElementType(kPasswordShortName, MsgParameter::ArgumentTypeType::kString)
    };

    char ssid_[System::kSsidStringLength]{0u};
    char password_[System::kPasswordStringLength]{0u};
    bool ssid_given_{false};
    bool password_given_{false};
    StdReturnType return_value_{kEOk};

    static void HandleParameter(char parameter_short_name, byte argument) { UNUSED(parameter_short_name); UNUSED(argument); }
    void HandleParameter(char parameter_short_name, const char* argument, PositionType length)
    {
        if(parameter_short_name == kSsidShortName) {
            StringTools::StringCopy(ssid_, argument, Minimum(length, sizeof(ssid_)));
            TrimTrailingBlanks(ssid_);
            ssid_given_ = true;
        }
        if(parameter_short_name == kPasswordShortName) {
            StringTools::StringCopy(password_, argument, Minimum(length, sizeof(password_)));
            TrimTrailingBlanks(password_);
            password_given_ = true;
        }
    }

    static constexpr size_t Minimum(size_t left, size_t right) { return (left < right) ? left : right; }

    /*
     * A string argument runs to the next option, so it carries the space that separates
     * them. A network called "Home " is not what anybody typed, and joining it would fail
     * in a way that names nothing - so a trailing blank is taken off here. It also means a
     * name or a pass phrase cannot end in a space over this interface, which is written
     * down in the command reference.
     */
    static void TrimTrailingBlanks(char* string) {
        for(size_t index = strlen(string); (index > 0u) && (string[index - 1u] == ' '); index--) {
            string[index - 1u] = STD_NULL_CHARACTER;
        }
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdNetworkParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdNetworkParser() { }

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `S=<name> Error=<error>`: it names the network and says whether there is one, and
     * never the pass phrase. `S` is the name System has stored, empty for a clock with no network.
     * `Error` is 0, or 8 if the command was refused. A space follows each field but the last;
     * MsgCmdParser ends the line.
     */
    void SendAnswer() const {
        char stored_ssid[System::kSsidStringLength]{};

        // An empty name is a clock with no network, which is the state the access point
        // stands in for - the same empty field the status command answers with.
        System::GetInstance().GetNetworkSsid(stored_ssid);

        SendAnswerParameter(kSsidShortName, stored_ssid);
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        error_.Send(return_value_, false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It stores the name and the pass phrase through System::SetNetworkCredentials() if the
     * command carried both, and keeps what that returned for the answer.
     *
     * Both halves or neither: a command that carried only one of them would either join a
     * network with the previous pass phrase or store a pass phrase for the previous network, and
     * both of those are a clock that goes quiet. So one alone is refused, and the stored pair is
     * left as it was. Without either it does nothing.
     */
    void Process()
    {
        if(!ssid_given_ && !password_given_) { return; }

        if(!ssid_given_ || !password_given_) {
            return_value_ = kENotOk;
            return;
        }

        return_value_ = System::GetInstance().SetNetworkCredentials(ssid_, password_);
    }
};

#endif // _MSG_CMD_NETWORK_PARSER_H_
