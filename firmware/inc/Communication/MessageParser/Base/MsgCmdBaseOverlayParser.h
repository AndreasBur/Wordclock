// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The options the date, temperature and text overlay commands share, as a parameter table.
 */

#ifndef _MSG_CMD_BASE_OVERLAY_PARSER_H_
#define _MSG_CMD_BASE_OVERLAY_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* MsgCmdBaseOverlayParser configuration parameter */

/* MsgCmdBaseOverlayParser parameter */
#define MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE           9u

class MsgCmdBaseOverlayParser
{
  public:
    using MsgParameterBaseParserType = MsgParameterParser<MsgCmdBaseOverlayParser, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
    using ParameterTableType = typename MsgParameterBaseParserType::ParameterTableType;
    using ParameterTableElementType = typename MsgParameterBaseParserType::ParameterTableElementType;

  protected:
    static constexpr char kPeriodOptionShortName{'P'};
    static constexpr char kEnduranceOptionShortName{'E'};
    static constexpr char kMonthOptionShortName{'M'};
    static constexpr char kDayOptionShortName{'D'};
    static constexpr char kValidOptionShortName{'V'};
    static constexpr char kActiveOptionShortName{'A'};
	static constexpr char kTextOptionShortName{'T'};
	static constexpr char kSpeedOptionShortName{'S'};
	static constexpr char kFontOptionShortName{'F'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
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

    constexpr MsgCmdBaseOverlayParser() { }
    ~MsgCmdBaseOverlayParser() { }

  private:

  public:

	// get methods

	// set methods

	// methods

};

#endif
