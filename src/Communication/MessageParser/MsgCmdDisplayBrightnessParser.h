// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the display brightness command.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYBRIGHTNESSPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYBRIGHTNESSPARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "MsgParameterParser.h"

/*! \brief The number of options in the parameter table of the display brightness command. */
#define MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE 3u

/*!
 * \brief The parser of command 3, the display brightness command: sets the brightness, and whether
 *        it follows the light sensor and is gamma corrected.
 *
 * Its options:
 * - `-B<brightness>`, uint8: sets the brightness asked for, 0 to 255. What reaches the LEDs is
 *   worked out from it, with the gamma correction, the light sensor and the night switch, and the
 *   current limit can hold it lower.
 * - `-A<0|1>`, uint8: with any value but 0, the brightness follows the light sensor; with 0, it
 *   does not.
 * - `-G<0|1>`, uint8: with any value but 0, the brightness is gamma corrected; with 0, it is not.
 *
 * Each option takes effect at once. An option the command does not carry is left alone, so `3`
 * alone is a query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies it at once. MsgCmdParser makes the parser and calls Parse(),
 * Process() and SendAnswer() in this order, so an error answered for an option stands in the line
 * before the fields of the answer.
 */
class MsgCmdDisplayBrightnessParser
    : public MsgParameterParser<MsgCmdDisplayBrightnessParser,
                                MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE> {
 public:
 private:
  friend class MsgParameterParser;
  static constexpr char kBrightnessOptionShortName{'B'};
  static constexpr char kUseAutomaticOptionShortName{'A'};
  static constexpr char kUseGammaCorrectionOptionShortName{'G'};

  static constexpr ParameterTableType kParameterTable PROGMEM{
      MsgParameter(kBrightnessOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      MsgParameter(kUseAutomaticOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      MsgParameter(kUseGammaCorrectionOptionShortName, MsgParameter::ArgumentTypeType::kUint8)};

  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  static void HandleParameter(char parameter_short_name, byte argument) {
    if (parameter_short_name == kBrightnessOptionShortName) {
      Display::GetInstance().SetBrightness(argument);
    }
    if (parameter_short_name == kUseAutomaticOptionShortName) {
      Display::GetInstance().SetBrightnessUseAutomatic(argument);
    }
    if (parameter_short_name == kUseGammaCorrectionOptionShortName) {
      Display::GetInstance().SetBrightnessUseGammaCorrection(argument);
    }
  }

  static void SendAnswerBrightness(bool append_space) {
    SendAnswerParameter(kBrightnessOptionShortName, Display::GetInstance().GetBrightness(),
                        append_space);
  }
  static void SendAnswerAutomatic(bool append_space) {
    SendAnswerParameter(kUseAutomaticOptionShortName,
                        Display::GetInstance().GetBrightnessUseAutomatic(), append_space);
  }
  static void SendAnswerGammaCorrection(bool append_space) {
    SendAnswerParameter(kUseGammaCorrectionOptionShortName,
                        Display::GetInstance().GetBrightnessUseGammaCorrection(), append_space);
  }

  void Show() const {
    StdReturnType return_value = Display::GetInstance().Show();
    error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kDisplayShow, return_value,
                                   ErrorMessage::ErrorType::kDisplayPending);
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  constexpr MsgCmdDisplayBrightnessParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {}

  /*! \brief Destroys the parser. */
  ~MsgCmdDisplayBrightnessParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `B=<brightness> A=<0|1> G=<0|1>`: the brightness asked for, not the one on the
   * LEDs, whether it follows the light sensor, and whether it is gamma corrected. A space follows
   * each field but the last; MsgCmdParser ends the line.
   */
  static void SendAnswer() {
    SendAnswerBrightness(true);
    SendAnswerAutomatic(true);
    SendAnswerGammaCorrection(false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It marks the display's buffer to be sent, through Display::Show(). A refusal would be
   * answered as an error of ErrorMessage::ApiType::kDisplayShow, but no backend refuses.
   */
  void Process() { Show(); }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYBRIGHTNESSPARSER_H_
