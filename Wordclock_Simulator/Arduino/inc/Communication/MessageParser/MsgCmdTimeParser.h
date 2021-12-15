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
/**     \file       MsgCmdTimeParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_TIME_PARSER_H_
#define _MSG_CMD_TIME_PARSER_H_

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
/* MsgCmdTimeParser configuration parameter */


/* MsgCmdTimeParser parameter */
#define MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE           3u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdTimeParser : public MsgParameterParser<MsgCmdTimeParser, MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = RealTimeClock::HourType;
    using MinuteType = RealTimeClock::MinuteType;
    using SecondType = RealTimeClock::SecondType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    ClockTime Time;

    static constexpr char HourOptionShortName{'H'};
    static constexpr char MinuteOptionShortName{'M'};
    static constexpr char SecondOptionShortName{'S'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(HourOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(MinuteOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(SecondOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == HourOptionShortName) {
            setHour(Argument);
        }
        if(ParameterShortName == MinuteOptionShortName) {
            setMinute(Argument);
        }
        if(ParameterShortName == SecondOptionShortName) {
            setSecond(Argument);
        }
    }

    void sendAnswerHour(bool AppendSpace) const {
        sendAnswerParameter(HourOptionShortName, Time.getHour(), AppendSpace);
    }
    void sendAnswerMinute(bool AppendSpace) const {
        sendAnswerParameter(MinuteOptionShortName, Time.getMinute(), AppendSpace);
    }
    void sendAnswerSecond(bool AppendSpace) const {
        sendAnswerParameter(SecondOptionShortName, Time.getSecond(), AppendSpace);
    }

    void setHour(HourType Hour) {
        StdReturnType returnValue = Time.setHour(Hour);
        Error.checkReturnValueAndSend(HourOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void setMinute(MinuteType Minute) {
        StdReturnType returnValue = Time.setMinute(Minute);
        Error.checkReturnValueAndSend(MinuteOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void setSecond(SecondType Second) {
        StdReturnType returnValue = Time.setSecond(Second);
        Error.checkReturnValueAndSend(SecondOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdTimeParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) {
        Time = RealTimeClock::getInstance().getTime();
    }
    ~MsgCmdTimeParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        sendAnswerHour(true);
        sendAnswerMinute(true);
        sendAnswerSecond(false);
    }

    void process()
    {
        RealTimeClock::getInstance().setTime(Time);
        Time = RealTimeClock::getInstance().getTime();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
