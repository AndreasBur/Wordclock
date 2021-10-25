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
/**     \file       MsgParameterParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_PARAMETER_PARSER_H_
#define _MSG_PARAMETER_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Message.h"
#include "ErrorMessage.h"
#include "MsgParameter.h"
#include "StringTools.h"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgParser configuration parameter */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_CHAR                     STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8                    STD_ON
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT16                   STD_ON
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT32                   STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT64                   STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT8                     STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT16                    STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT32                    STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT64                    STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_FLOAT                    STD_OFF
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_DOUBLE                   STD_OFF

/* MsgParser parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M S G   P A R A M E T E R   P A R S E R
******************************************************************************************************************************************************/
template <typename Derived, size_t ParameterTableSize> class MsgParameterParser
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using ParameterTableElementType = MsgParameter;
    using ParameterTableType = std::array<ParameterTableElementType, ParameterTableSize>;
    using PositionType = StringTools::PositionType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    static constexpr char OptionArgumentDelimiter{'='};
    ErrorMessage Error;

    ~MsgParameterParser() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    enum StateType {
        STATE_PARSE,
        STATE_OPTION_START,
        STATE_OPTION_CHAR,
        STATE_OPTION_ARGUMENT
    };

    static constexpr char OptionStartChar{'-'};
    static constexpr byte ArgumentNumberBase{10u};

    const char* ParameterBuffer{nullptr};
    const ParameterTableType& ParameterTable;

    // functions
    ParameterTableElementType getParameterTableElement(byte Index) const {
        ParameterTableElementType parameterTableElement;
        memcpy_P(&parameterTableElement, &ParameterTable[Index], sizeof(ParameterTableElementType));
        return parameterTableElement;
    }

    StdReturnType getMsgParameterByOptionShortName(char OptionShortName, ParameterTableElementType& Parameter) const {
        for(size_t index = 0; index < ParameterTable.size(); index++) {
            ParameterTableElementType parameterTableElement = getParameterTableElement(index);
            if(parameterTableElement.getOptionShortName() == OptionShortName) {
                Parameter = parameterTableElement;
                return E_OK;
            }
        }
        return E_NOT_OK;
    }

    PositionType parseArgument(MsgParameter Parameter, const char* Argument) {
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_CHAR == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_CHAR) { return convertArgument<char>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT8) { return convertArgument<uint8_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT16 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT16) { return convertArgument<uint16_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT32 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT32) { return convertArgument<uint32_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT64 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT64) { return convertArgument<uint64_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT8 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT8) { return convertArgument<int8_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT16 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT16) { return convertArgument<int16_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT32 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT32) { return convertArgument<int32_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT64 == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT64) { return convertArgument<int64_t>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_FLOAT == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_FLOAT) { return convertArgument<float>(Parameter, Argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_DOUBLE == STD_ON)
        if(Parameter.getArgumentType() == MsgParameter::ARGUMENT_TYPE_DOUBLE) { return convertArgument<double>(Parameter, Argument); }
# endif
        return 0u;
    }

    template <typename T> PositionType convertArgument(MsgParameter Parameter, const char* Argument) {
        T value;
        PositionType position;

        StringTools::ResultType result = StringTools::stringTo(Argument, position, value, ArgumentNumberBase);
        handleConvertResult(Parameter, result);
        if(result == StringTools::RESULT_OK) static_cast<Derived*>(this)->handleParameter(Parameter.getOptionShortName(), value);
        return position;
    }

    void handleConvertResult(MsgParameter Parameter, StringTools::ResultType Result) const {
        if(Result == StringTools::RESULT_NO_VALUE) {
            Error.send(Parameter, ErrorMessage::ERROR_NO_VALUE_GIVEN);
        }
        if(Result == StringTools::RESULT_OVERFLOW) {
            Error.send(Parameter, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgParameterParser(const ParameterTableType& sParameterTable, const char* sParameter)
    : Error(), ParameterBuffer(sParameter), ParameterTable(sParameterTable) { }

    // get methods
    const char* getParameter() const { return ParameterBuffer; }
    const ParameterTableType& getParameterTable() const { return ParameterTable; }

    // set methods

    // methods
    void parse()
    {
        StateType state = STATE_PARSE;
        char optionShortName;

        for(PositionType position = 0u; ParameterBuffer[position] != STD_NULL_CHARACTER; position++) {
            if(state == STATE_PARSE) {
                char currentChar = ParameterBuffer[position];
                if(currentChar == OptionStartChar) { state = STATE_OPTION_CHAR; }
            } else if(state == STATE_OPTION_CHAR) {
                optionShortName = ParameterBuffer[position];
                state = STATE_OPTION_ARGUMENT;
            } else if(state == STATE_OPTION_ARGUMENT) {
                ParameterTableElementType parameter;
                if(getMsgParameterByOptionShortName(optionShortName, parameter) == E_OK) {
                    position += parseArgument(parameter, &ParameterBuffer[position]) - 1u;
                } else { Error.send(optionShortName, ErrorMessage::ERROR_PARAMETER_UNKNOWN); }
                state = STATE_PARSE;
            }
        }
    }

    template <typename T> void sendAnswerParameter(char OptionShortName, T Value, bool AppendSpace = true) const
    {
        Serial.print(OptionShortName);
        Serial.print(OptionArgumentDelimiter);
        Serial.print(Value);
        if(AppendSpace) { Serial.print(' '); }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
