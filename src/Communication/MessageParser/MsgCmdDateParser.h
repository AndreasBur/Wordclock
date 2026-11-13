// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the date command.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDATEPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDATEPARSER_H_

#include "Arduino.h"
#include "MsgParameterParser.h"
#include "RealTimeClock.h"
#include "StandardTypes.h"

/*! \brief The number of options in the parameter table of the date command. */
#define MSG_CMD_DATE_PARSER_PARAMETER_TABLE_SIZE 3u

/*!
 * \brief The parser of command 11, the date command: sets the date of the real-time clock.
 *
 * Its options:
 * - `-Y<year>`, uint16: the year, 2000 to 2099.
 * - `-M<month>`, uint8: the month, 1 to 12.
 * - `-D<day>`, uint8: the day of the month, 1 to 31; it is checked against 31, not against the
 *   days of the month.
 *
 * A value out of its range is answered with `Error=4:<option>`, and that part of the date is left
 * as it was. An option the command does not carry is left as it was too.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdDateParser
    : public MsgParameterParser<MsgCmdDateParser, MSG_CMD_DATE_PARSER_PARAMETER_TABLE_SIZE> {
 public:
  /*! \brief A year in full, as `-Y` takes it: RealTimeClock::YearType. */
  using YearType = RealTimeClock::YearType;
  /*! \brief A month, as `-M` takes it: RealTimeClock::MonthType. */
  using MonthType = RealTimeClock::MonthType;
  /*! \brief A day of the month, as `-D` takes it: RealTimeClock::DayType. */
  using DayType = RealTimeClock::DayType;

 private:
  friend class MsgParameterParser;
  ClockDate date_;

  static constexpr char kYearOptionShortName{'Y'};
  static constexpr char kMonthOptionShortName{'M'};
  static constexpr char kDayOptionShortName{'D'};

  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kYearOptionShortName, MsgParameter::ArgumentTypeType::kUint16),
      ParameterTableElementType(kMonthOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kDayOptionShortName, MsgParameter::ArgumentTypeType::kUint8)};

  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  void HandleParameter(char parameter_short_name, byte argument) {
    if (parameter_short_name == kMonthOptionShortName) {
      SetMonth(static_cast<MonthType>(argument));
    }
    if (parameter_short_name == kDayOptionShortName) {
      SetDay(argument);
    }
  }

  void HandleParameter(char parameter_short_name, uint16_t argument) {
    if (parameter_short_name == kYearOptionShortName) {
      SetYear(argument);
    }
  }

  void SendAnswerYear(bool append_space) const {
    SendAnswerParameter(kYearOptionShortName, date_.GetYear(), append_space);
  }
  void SendAnswerMonth(bool append_space) const {
    SendAnswerParameter(kMonthOptionShortName, date_.GetMonth(), append_space);
  }
  void SendAnswerDay(bool append_space) const {
    SendAnswerParameter(kDayOptionShortName, date_.GetDay(), append_space);
  }

  void SetYear(YearType year) {
    StdReturnType return_value = date_.SetYear(year);
    error_.CheckReturnValueAndSend(kYearOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }
  void SetMonth(MonthType month) {
    StdReturnType return_value = date_.SetMonth(month);
    error_.CheckReturnValueAndSend(kMonthOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }
  void SetDay(DayType day) {
    StdReturnType return_value = date_.SetDay(day);
    error_.CheckReturnValueAndSend(kDayOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * It takes the date of the real-time clock as it is now, which the options change part by
   * part.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  explicit MsgCmdDateParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter) {
    date_ = RealTimeClock::GetInstance().GetDate();
  }

  /*! \brief Destroys the parser. */
  ~MsgCmdDateParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `Y=<year> M=<month> D=<day>`: the date of the real-time clock, as Process() has read
   * it back. A space follows each field but the last; MsgCmdParser ends the line.
   */
  void SendAnswer() const {
    SendAnswerYear(true);
    SendAnswerMonth(true);
    SendAnswerDay(false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It writes the date to the real-time clock, and reads it back for the answer. It writes even
   * when the command carried no option, the date the parser was made with.
   */
  void Process() {
    RealTimeClock::GetInstance().SetDate(date_);
    date_ = RealTimeClock::GetInstance().GetDate();
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDDATEPARSER_H_
