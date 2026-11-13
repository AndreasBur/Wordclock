// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the display pixel command.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYPIXELPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYPIXELPARSER_H_

#include "Arduino.h"
#include "Display.h"
#include "MsgParameterParser.h"
#include "StandardTypes.h"

/*! \brief The number of options in the parameter table of the display pixel command. */
#define MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE 2u

#if (WS2812_NUMBER_OF_LEDS < 255u)
/*!
 * \brief The argument type of `-I`: uint8 while WS2812_NUMBER_OF_LEDS lies below 255, uint16
 *        otherwise.
 *
 * WS2812_NUMBER_OF_LEDS is defined nowhere, so the preprocessor takes it as 0, and the type is
 * uint8 whatever the number of LEDs.
 */
#define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE MsgParameter::ArgumentTypeType::kUint8
#else
#define MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE MsgParameter::ArgumentTypeType::kUint16
#endif

/*!
 * \brief The parser of command 4, the display pixel command: lights a single pixel or turns it off.
 *
 * Its options:
 * - `-I<index>`, uint8: the index of the pixel, row * DISPLAY_NUMBER_OF_COLUMNS + column; without
 *   `-I` the answer is about pixel 0. An index off the display is answered with `Error=4:I`, and
 *   nothing is written.
 * - `-S<0|1>`, uint8: with any value but 0, lights the pixel of `-I` in the colour of the display;
 *   with 0, turns it off. It writes only beside an `-I` that was taken: `4 -S1` alone writes
 *   nothing, as `4 -I300 -S1` does not. Without `-S` the pixel is written as it is.
 *
 * The pixel stands until something draws over it. Process() marks the display's buffer to be sent
 * in any case.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * `-S` sets a pixel only together with an `-I` that was taken: `4 -I300 -S1` answers `Error=4:I`
 * and sets nothing, and `4 -S1` alone sets nothing either.
 */
class MsgCmdDisplayPixelParser
    : public MsgParameterParser<MsgCmdDisplayPixelParser,
                                MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE> {
 public:
  /*! \brief What the command writes to the pixel, as `-S` asks for it or `-I` finds it. */
  enum class StateType {
    kNone, /*!< Nothing to write: no `-S`, and no `-I` of a pixel on the display. */
    kOff,  /*!< The pixel is to be turned off. */
    kOn    /*!< The pixel is to be lit. */
  };

  /*! \brief The index of a pixel, Display::IndexType. */
  using IndexType = Display::IndexType;

 private:
  friend class MsgParameterParser;
  byte index_{0u};
  // Whether an -I was taken; -S sets a pixel only then.
  bool index_given_{false};
  StateType state_{StateType::kNone};

  static constexpr char kIndexOptionShortName{'I'};
  static constexpr char kStateOptionShortName{'S'};

  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kIndexOptionShortName,
                                MSG_CMD_DISPLAY_PIXEL_PARSER_INDEX_ARGUMENT_TYPE),
      ParameterTableElementType(kStateOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
  };

  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  void HandleParameter(char parameter_short_name, byte argument) {
    if (parameter_short_name == kIndexOptionShortName) {
      HandleParameterIndex(argument);
    }
    if (parameter_short_name == kStateOptionShortName) {
      HandleParameterState(argument);
    }
  }

  void HandleParameterIndex(byte argument) {
    index_ = argument;
    index_given_ = true;
    if (state_ == StateType::kNone) {
      state_ = GetPixelState(index_);
    }
  }

  void HandleParameterState(byte argument) {
    bool pixel_value = argument;
    state_ = GetPixelState(pixel_value);
  }

  static StateType GetPixelState(byte index) {
    /*
     * Asked before reading rather than read and then judged by a return code: the pixel
     * itself is what this answers with, and an index that is not on the display has no
     * state rather than a false one.
     */
    if (!Display::GetInstance().IsIndexValid(index)) {
      return StateType::kNone;
    }

    return GetPixelState(Display::GetInstance().GetPixel(index));
  }

  static StateType GetPixelState(bool pixel_value) {
    return pixel_value ? StateType::kOn : StateType::kOff;
  }
  static bool GetPixelValue(StateType pixel_state) { return pixel_state == StateType::kOn; }

  void SendAnswerIndex(bool append_space) const {
    SendAnswerParameter(kIndexOptionShortName, index_, append_space);
  }
  void SendAnswerState(bool append_space) const {
    SendAnswerParameter(kStateOptionShortName, Display::GetInstance().GetPixel(index_),
                        append_space);
  }

  /*
   * Only for an index that was given and taken. Without one, a refused -I left the index at 0,
   * and 4 -I300 -S1 lit pixel 0.
   */
  void SetPixel() const {
    if (index_given_ && (state_ != StateType::kNone)) {
      Display::GetInstance().WritePixel(index_, GetPixelValue(state_));
    }
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
  explicit constexpr MsgCmdDisplayPixelParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {}

  /*! \brief Destroys the parser. */
  ~MsgCmdDisplayPixelParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `I=<index> S=<0|1>`: the index, and whether the pixel is lit now. For an index off
   * the display it sends no field. A space follows each field but the last; MsgCmdParser ends the
   * line.
   */
  void SendAnswer() const {
    if (Display::GetInstance().IsIndexValid(index_)) {
      SendAnswerIndex(true);
      SendAnswerState(false);
    }
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It writes the pixel as `-S` asks, or answers `Error=4:I` for an index off the display. Then
   * it marks the display's buffer to be sent, through Display::Show(). A refusal would be
   * answered as an error of ErrorMessage::ApiType::kDisplayShow, but no backend refuses.
   */
  void Process() {
    if (!Display::GetInstance().IsIndexValid(index_)) {
      error_.Send(kIndexOptionShortName, ErrorMessage::ErrorType::kValueOutOfBounds, false);
    } else {
      SetPixel();
    }
    Show();
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDISPLAYPIXELPARSER_H_
