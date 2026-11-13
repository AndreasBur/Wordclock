// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Configures when the clock takes the night off.
 *
 * Five numbers and a switch: the two ends of the window, and how bright the display is inside it,
 * where zero means off altogether.
 *
 * Every option is optional, and what is not sent is left as it was - so the end alone can be moved
 * without repeating the start. The answer reports the whole window whatever was sent, which is what
 * makes a half-remembered setting checkable in one line.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDNIGHTSWITCHPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDNIGHTSWITCHPARSER_H_

#include "Arduino.h"
#include "MsgParameterParser.h"
#include "NightSwitch.h"
#include "StandardTypes.h"

/*! \brief The number of options in the parameter table of the night switch command. */
#define MSG_CMD_NIGHT_SWITCH_PARSER_PARAMETER_TABLE_SIZE 6u

/*!
 * \brief The parser of command 14, the night switch command: sets the window of the night, and what
 *        the night does to the display.
 *
 * Its options:
 * - `-A<0|1>`, uint8: with any value but 0, switches the night switch on; with 0, switches it off
 *   and gives the display back.
 * - `-H<hour>` and `-M<minute>`, uint8: the time the night starts at.
 * - `-E<hour>` and `-N<minute>`, uint8: the time the night ends at. The window may cross midnight.
 * - `-B<brightness>`, uint8: what the night does to the display: 0 switches it off, any other value
 *   dims it to that level. It takes effect at the next start of the night.
 *
 * None of the values is checked: an hour above 23 or a minute above 59 is stored as it comes. An
 * option the command does not carry is left as it was, so `14` alone is a query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies `-H`, `-M`, `-E`, `-N` and `-B` at once and keeps `-A` for
 * Process(). MsgCmdParser makes the parser and calls Parse(), Process() and SendAnswer() in this
 * order, so an error answered for an option stands in the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdNightSwitchParser
    : public MsgParameterParser<MsgCmdNightSwitchParser,
                                MSG_CMD_NIGHT_SWITCH_PARSER_PARAMETER_TABLE_SIZE> {
 private:
  friend class MsgParameterParser;

  static constexpr char kActiveShortName{'A'};
  static constexpr char kStartHourShortName{'H'};
  static constexpr char kStartMinuteShortName{'M'};
  static constexpr char kEndHourShortName{'E'};
  static constexpr char kEndMinuteShortName{'N'};
  static constexpr char kBrightnessShortName{'B'};

  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kActiveShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kStartHourShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kStartMinuteShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kEndHourShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kEndMinuteShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kBrightnessShortName, MsgParameter::ArgumentTypeType::kUint8)};

  /*
   * Collected rather than applied as they arrive, and the switch applied last: switching
   * it on is what makes the next tick an edge, and an edge acting on half a window would
   * use one end from this command and the other from the one before it.
   */
  bool active_{false};
  bool active_given_{false};

  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  void HandleParameter(char parameter_short_name, byte argument) {
    NightSwitch& night_switch = NightSwitch::GetInstance();

    if (parameter_short_name == kActiveShortName) {
      active_ = (argument != 0u);
      active_given_ = true;
    }
    if (parameter_short_name == kStartHourShortName) {
      night_switch.SetStartHour(argument);
    }
    if (parameter_short_name == kStartMinuteShortName) {
      night_switch.SetStartMinute(argument);
    }
    if (parameter_short_name == kEndHourShortName) {
      night_switch.SetEndHour(argument);
    }
    if (parameter_short_name == kEndMinuteShortName) {
      night_switch.SetEndMinute(argument);
    }
    if (parameter_short_name == kBrightnessShortName) {
      night_switch.SetNightBrightness(argument);
    }
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  constexpr MsgCmdNightSwitchParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {}

  /*! \brief Destroys the parser. */
  ~MsgCmdNightSwitchParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `A=<0|1> H=<hour> M=<minute> E=<hour> N=<minute> B=<brightness>`: whether the night
   * switch is on, the start and the end of the window, and the night brightness, whatever the
   * command carried. A space follows each field but the last; MsgCmdParser ends the line.
   */
  static void SendAnswer() {
    NightSwitch const& night_switch = NightSwitch::GetInstance();

    SendAnswerParameter(kActiveShortName, static_cast<byte>(night_switch.GetIsActive() ? 1u : 0u));
    SendAnswerParameter(kStartHourShortName, night_switch.GetStartHour());
    SendAnswerParameter(kStartMinuteShortName, night_switch.GetStartMinute());
    SendAnswerParameter(kEndHourShortName, night_switch.GetEndHour());
    SendAnswerParameter(kEndMinuteShortName, night_switch.GetEndMinute());
    // Last field before the command parser's terminating println(), so no trailing
    // separator space.
    SendAnswerParameter(kBrightnessShortName, night_switch.GetNightBrightness(), false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It switches the night switch on or off as `-A` asks, after the window and the brightness of
   * the same command have been applied. Without `-A` it does nothing.
   */
  void Process() const {
    if (active_given_) {
      NightSwitch::GetInstance().SetIsActive(active_);
    }
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDNIGHTSWITCHPARSER_H_
