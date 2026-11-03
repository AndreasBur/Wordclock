/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       MsgCmdDateParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_DATE_PARSER_H_
#define _MSG_CMD_DATE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "RealTimeClock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdDateParser configuration parameter */


/* MsgCmdDateParser parameter */
#define MSG_CMD_DATE_PARSER_PARAMETER_TABLE_SIZE           3u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdDateParser : public MsgParameterParser<MsgCmdDateParser, MSG_CMD_DATE_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using YearType = RealTimeClock::YearType;
    using MonthType = RealTimeClock::MonthType;
    using DayType = RealTimeClock::DayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    ClockDate date_;

    static constexpr char kYearOptionShortName{'Y'};
    static constexpr char kMonthOptionShortName{'M'};
    static constexpr char kDayOptionShortName{'D'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kYearOptionShortName, MsgParameter::ArgumentTypeType::kUint16),
        ParameterTableElementType(kMonthOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kDayOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument) {
        if(parameter_short_name == kMonthOptionShortName) { SetMonth(static_cast<MonthType>(argument)); }
        if(parameter_short_name == kDayOptionShortName) { SetDay(argument); }
    }

	void HandleParameter(char parameter_short_name, uint16_t argument) {
		if(parameter_short_name == kYearOptionShortName) { SetYear(argument); }
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

    void SetYear(YearType year)  {
        StdReturnType return_value =  date_.SetYear(year);
        error_.CheckReturnValueAndSend(kYearOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetMonth(MonthType month)  {
        StdReturnType return_value = date_.SetMonth(month);
        error_.CheckReturnValueAndSend(kMonthOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetDay(DayType day)  {
        StdReturnType return_value = date_.SetDay(day);
        error_.CheckReturnValueAndSend(kDayOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    MsgCmdDateParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) {
        date_ = RealTimeClock::GetInstance().GetDate();
    }
    ~MsgCmdDateParser() { }

    // get methods

    // set methods

    // methods
    void SendAnswer() const
    {
        SendAnswerYear(true);
        SendAnswerMonth(true);
        SendAnswerDay(false);
    }

    void Process()
    {
        RealTimeClock::GetInstance().SetDate(date_);
        date_ = RealTimeClock::GetInstance().GetDate();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
