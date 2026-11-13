// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The options the date, temperature and text overlay commands share, as a parameter table.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_BASE_MSGCMDBASEOVERLAYPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_BASE_MSGCMDBASEOVERLAYPARSER_H_

#include "Arduino.h"
#include "MsgParameterParser.h"

/*! \brief The number of options in the parameter table of the overlay commands. */
#define MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE 9u

/*!
 * \brief The options of the date, temperature and text overlay commands: their short names and the
 *        parameter table MsgCmdOverlayParser reads them by.
 *
 * The three commands take the same nine options, so they are named here once:
 * - `-P<period>`, uint8: the period, in minutes.
 * - `-E<endurance>`, uint8: the endurance, in seconds.
 * - `-M<month>`, uint8: the month the overlay is limited to, 0 for none.
 * - `-D<day>`, uint8: the day of the month the overlay is limited to, 0 for none.
 * - `-V<days>`, uint8: the valid-in-days, on how many days after its date the overlay goes on
 *   starting.
 * - `-A<0|1>`, uint8: whether the overlay is switched on.
 * - `-T<text>`, string: the text, which only the text overlay takes.
 * - `-S<speed>`, uint8: the speed its text scrolls at.
 * - `-F<id>`, uint8: the font its text is drawn in.
 *
 * What each option does is described at MsgCmdOverlayParser.
 */
class MsgCmdBaseOverlayParser {
 public:
  /*! \brief The parameter parser whose table type the overlay commands use. */
  using MsgParameterBaseParserType =
      MsgParameterParser<MsgCmdBaseOverlayParser, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
  /*! \brief The parameter table, MsgParameterParser::ParameterTableType. */
  using ParameterTableType = typename MsgParameterBaseParserType::ParameterTableType;
  /*! \brief An option of the parameter table, MsgParameterParser::ParameterTableElementType. */
  using ParameterTableElementType = typename MsgParameterBaseParserType::ParameterTableElementType;

 protected:
  static constexpr char kPeriodOptionShortName{'P'};    /*!< The short name of the period. */
  static constexpr char kEnduranceOptionShortName{'E'}; /*!< The short name of the endurance. */
  static constexpr char kMonthOptionShortName{'M'};     /*!< The short name of the month. */
  static constexpr char kDayOptionShortName{'D'};       /*!< The short name of the day. */
  static constexpr char kValidOptionShortName{'V'};     /*!< The short name of the valid-in-days. */
  static constexpr char kActiveOptionShortName{'A'};    /*!< The short name of the switch. */
  static constexpr char kTextOptionShortName{'T'};      /*!< The short name of the text. */
  static constexpr char kSpeedOptionShortName{'S'};     /*!< The short name of the speed. */
  static constexpr char kFontOptionShortName{'F'};      /*!< The short name of the font. */

  /*! \brief The parameter table of the overlay commands: their nine options, with their types. */
  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kPeriodOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kEnduranceOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kMonthOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kDayOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kValidOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kActiveOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kTextOptionShortName, MsgParameter::ArgumentTypeType::kString),
      ParameterTableElementType(kSpeedOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kFontOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
  };

  /*! \brief Makes the options of an overlay parser. */
  constexpr MsgCmdBaseOverlayParser() {}

  /*! \brief Destroys the options of an overlay parser. */
  ~MsgCmdBaseOverlayParser() = default;

 public:
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_BASE_MSGCMDBASEOVERLAYPARSER_H_
