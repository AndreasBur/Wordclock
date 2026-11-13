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
 * Sent with an empty password, `16 -P`, the command clears it so that the console answers
 * everybody again. That is the way back for a clock whose password was forgotten, and it is why
 * this command is worth having on the serial line rather than only in the console it locks: the
 * console would ask for the very password that is missing.
 *
 * `16` on its own asks and changes nothing, as every other command without options does. It used
 * to clear as well, which made the shorthand a trap: the panel asks for every command it knows by
 * its bare number when its socket opens, so opening the panel took the password off the clock.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDCONSOLEPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDCONSOLEPARSER_H_

#include "Arduino.h"
#include "MsgParameterParser.h"
#include "StandardTypes.h"
#include "System.h"

/*! \brief The number of options in the parameter table of the console command. */
#define MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE 1u

/*!
 * \brief The parser of command 16, the console command: sets the password the web console asks for.
 *
 * Its option:
 * - `-P<password>`, string: the password, up to the next option, without trailing spaces, and cut
 *   to what System::kPasswordStringLength holds. An empty one, `16 -P`, clears it.
 *
 * Process() stores it through System::SetConsolePassword() when `-P` was given, and does nothing
 * without it, so `16` alone is a query. It is stored on the ESP32 and the RP2350; the AVR and the
 * simulator refuse it, and keep no password. A refusal is not answered.
 *
 * The answer says whether a password is set, never which one.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdConsoleParser
    : public MsgParameterParser<MsgCmdConsoleParser, MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE> {
 private:
  friend class MsgParameterParser;

  static constexpr char kPasswordShortName{'P'};
  static constexpr char kProtectedShortName{'A'};

  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kPasswordShortName, MsgParameter::ArgumentTypeType::kString)};

  char password_[System::kPasswordStringLength]{0u};
  /*
   * Whether -P was given at all, which is not the same question as whether the password is
   * empty: `16 -P` clears the password on purpose, and `16` alone must leave it alone. Without
   * this the two cannot be told apart, and the bare number cleared - see the \file block.
   */
  bool password_given_{false};

  static void HandleParameter(char parameter_short_name, byte argument) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
  }
  /*
   * Copied with its length rather than read to the end of the line, as it was: the argument is
   * not terminated in the parameter string, and 16 -Psecret -A1 stored "secret -A1".
   */
  void HandleParameter(char parameter_short_name, char const* argument, PositionType length) {
    if (parameter_short_name == kPasswordShortName) {
      StringTools::StringCopy(password_, argument, Minimum(length, sizeof(password_)));
      TrimTrailingBlanks(password_);
      password_given_ = true;
    }
  }

  static constexpr size_t Minimum(size_t left, size_t right) {
    return (left < right) ? left : right;
  }

  static void TrimTrailingBlanks(char* string) {
    for (size_t index = strlen(string); (index > 0u) && (string[index - 1u] == ' '); index--) {
      string[index - 1u] = STD_NULL_CHARACTER;
    }
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  constexpr MsgCmdConsoleParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {}

  /*! \brief Destroys the parser. */
  ~MsgCmdConsoleParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `A=<0|1>`: whether a password is set, and never which one. A space follows each
   * field but the last; MsgCmdParser ends the line.
   */
  static void SendAnswer() {
    SendAnswerParameter(kProtectedShortName,
                        static_cast<byte>(System::GetInstance().IsConsoleProtected() ? 1u : 0u),
                        false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It stores the password of `-P`, the empty one of `16 -P` included, which clears it. Without
   * `-P` nothing is stored, so `16` alone only asks.
   */
  void Process() const {
    if (password_given_) {
      System::GetInstance().SetConsolePassword(password_);
    }
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDCONSOLEPARSER_H_
