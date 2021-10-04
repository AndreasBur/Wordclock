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
    using YearType = RealTimeClock::HourType;
    using MonthType = RealTimeClock::MonthType;
    using DayType = RealTimeClock::DayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char YearOptionShortName{'Y'};
    static constexpr char MonthOptionShortName{'M'};
    static constexpr char DayOptionShortName{'D'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(YearOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT16),
        ParameterTableElementType(MonthOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(DayOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, byte Argument)
    {

        if(ParameterShortName == MonthOptionShortName) {
            setMonth(Argument);
        }
        if(ParameterShortName == DayOptionShortName) {
            setDay(Argument);
        }
    }

	void handleParameter(char ParameterShortName, uint16_t Argument)
	{
		if(ParameterShortName == YearOptionShortName) {
            setYear(Argument);
        }
	}

    void sendAnswerYear(bool AppendSpace) const {
        sendAnswerParameter(YearOptionShortName, RealTimeClock::getInstance().getDateYear(), AppendSpace);
    }
    void sendAnswerMonth(bool AppendSpace) const {
        sendAnswerParameter(MonthOptionShortName, RealTimeClock::getInstance().getDateMonth(), AppendSpace);
    }
    void sendAnswerDay(bool AppendSpace) const {
        sendAnswerParameter(DayOptionShortName, RealTimeClock::getInstance().getDateDay(), AppendSpace);
    }

    void setYear(YearType Year) const {
        StdReturnType returnValue = RealTimeClock::getInstance().setDateYear(Year);
        Error.checkReturnValueAndSend(YearOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void setMonth(MonthType Month) const {
        StdReturnType returnValue = RealTimeClock::getInstance().setDateMonth(Month);
        Error.checkReturnValueAndSend(MonthOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void setDay(DayType Day) const {
        StdReturnType returnValue = RealTimeClock::getInstance().setDateDay(Day);
        Error.checkReturnValueAndSend(DayOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void show() const
    {
        StdReturnType returnValue = Clock::getInstance().show();
        Error.checkReturnValueAndSend(ErrorMessage::API_CLOCK_SHOW, returnValue, ErrorMessage::ERROR_DISPLAY_PENDING);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDateParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdDateParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        sendAnswerYear(true);
        sendAnswerMonth(true);
        sendAnswerDay(false);
    }

    void process() const
    {
        show();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
