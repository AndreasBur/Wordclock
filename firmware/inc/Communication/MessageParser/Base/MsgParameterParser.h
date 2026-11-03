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

#include <type_traits>
#include "Communication.h"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgParameterParser configuration parameter */
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
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_STRING                   STD_ON

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
    using LengthType = StringTools::LengthType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    static constexpr char kOptionArgumentDelimiter{'='};
    ErrorMessage error_;

    /* Protected rather than private with the derived class as a friend, which is what
       clang-tidy asks for: a friend would open this base's whole private side to it, where
       protected opens only what is meant for it. What the private constructor would
       prevent - somebody inheriting this template with another class's Derived - the
       assertion catches instead, at the first point where Derived is a complete type. */
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
    constexpr MsgParameterParser(const ParameterTableType& parameter_table, const char* parameter)
    : error_(), parameter_buffer_(parameter), parameter_table_(parameter_table) {
        static_assert(std::is_base_of<MsgParameterParser, Derived>::value,
                      "MsgParameterParser is a CRTP base: Derived has to be the class that inherits it");
    }
    ~MsgParameterParser() { }


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    enum StateType {
        kStateParse,
        kStateOptionStart,
        kStateOptionChar,
        kStateOptionArgument
    };

    static constexpr char kOptionStartChar{'-'};
    static constexpr byte kArgumentNumberBase{10u};

    const char* parameter_buffer_{nullptr};
    const ParameterTableType& parameter_table_;

    // functions
    Derived& Underlying() { return static_cast<Derived&>(*this); }
    Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

    static bool IsEndOfStringReached(char character) {
        return (character == STD_NULL_CHARACTER) ||
               (character == Communication::GetEndOfMessageChar() ||
               (character == kOptionStartChar));
    }

    ParameterTableElementType GetParameterTableElement(byte index) const {
        ParameterTableElementType parameter_table_element;
        memcpy_P(&parameter_table_element, &parameter_table_[index], sizeof(ParameterTableElementType));
        return parameter_table_element;
    }

    StdReturnType GetMsgParameterByOptionShortName(char option_short_name, ParameterTableElementType& parameter) const {
        for(size_t index = 0; index < parameter_table_.size(); index++) {
            ParameterTableElementType parameter_table_element = GetParameterTableElement(index);
            if(parameter_table_element.GetOptionShortName() == option_short_name) {
                parameter = parameter_table_element;
                return kEOk;
            }
        }
        return kENotOk;
    }

    PositionType ParseArgument(MsgParameter parameter, const char* argument) {
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_CHAR == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeChar) { return ConvertArgument<char>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeUint8) { return ConvertArgument<uint8_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT16 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeUint16) { return ConvertArgument<uint16_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT32 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeUint32) { return ConvertArgument<uint32_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT64 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeUint64) { return ConvertArgument<uint64_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT8 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeInt8) { return ConvertArgument<int8_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT16 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeInt16) { return ConvertArgument<int16_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT32 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeInt32) { return ConvertArgument<int32_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT64 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeInt64) { return ConvertArgument<int64_t>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_FLOAT == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeFloat) { return ConvertArgument<float>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_DOUBLE == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeDouble) { return ConvertArgument<double>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_STRING == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::kArgumentTypeString) { return ConvertArgumentToString(parameter, argument); }
# endif
        return 0u;
    }

    template <typename T> PositionType ConvertArgument(MsgParameter parameter, const char* argument) {
        T value;
        PositionType position;
        StringTools::ResultType result = StringTools::StringTo(argument, position, value, kArgumentNumberBase);
        HandleConvertResult(parameter, result);
        if(result == StringTools::kResultOk) { Underlying().HandleParameter(parameter.GetOptionShortName(), value); }
        return position;
    }

    PositionType ConvertArgumentToString(MsgParameter parameter, const char* argument) {
        PositionType position = 0u;
        while(!IsEndOfStringReached(argument[position])) { position++; }
        LengthType length = position + 1u;
        Underlying().HandleParameter(parameter.GetOptionShortName(), argument, length);
        return position;
    }

    void HandleConvertResult(MsgParameter parameter, StringTools::ResultType result) const {
        if(result == StringTools::kResultNoValue) {
            error_.Send(parameter, ErrorMessage::kErrorNoValueGiven);
        }
        if(result == StringTools::kResultOverflow) {
            error_.Send(parameter, ErrorMessage::kErrorValueOutOfBounds);
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:

    // get methods
    const char* GetParameter() const { return parameter_buffer_; }
    const ParameterTableType& GetParameterTable() const { return parameter_table_; }

    // set methods

    // methods
    void Parse()
    {
        StateType state = kStateParse;
        char option_short_name;

        for(PositionType position = 0u; parameter_buffer_[position] != STD_NULL_CHARACTER; position++) {
            if(state == kStateParse) {
                char current_char = parameter_buffer_[position];
                if(current_char == kOptionStartChar) { state = kStateOptionChar; }
            } else if(state == kStateOptionChar) {
                option_short_name = parameter_buffer_[position];
                state = kStateOptionArgument;
            } else if(state == kStateOptionArgument) {
                ParameterTableElementType parameter;
                if(GetMsgParameterByOptionShortName(option_short_name, parameter) == kEOk) {
                    position += ParseArgument(parameter, &parameter_buffer_[position]) - 1u;
                } else { error_.Send(option_short_name, ErrorMessage::kErrorParameterUnknown); }
                state = kStateParse;
            }
        }
    }

    template <typename T> static void SendAnswerParameter(char option_short_name, T value, bool append_space = true)
    {
        Serial.print(option_short_name);
        Serial.print(kOptionArgumentDelimiter);
        Serial.print(value);
        if(append_space) { Serial.print(' '); }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
