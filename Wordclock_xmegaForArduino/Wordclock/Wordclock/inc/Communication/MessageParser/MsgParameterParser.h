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

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgParser configuration parameter */


/* MsgParser parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
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
 *  P R O T E C T E D   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  protected:
	ErrorMessage Error;
    const char* Parameter;
	const ParameterTableType& ParameterTable;
	
	~MsgParameterParser() {
		
	}
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
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
	
	// functions
    ParameterTableElementType getParameterTableElement(byte Index) const {
	    ParameterTableElementType parameterTableElement;
	    memcpy_P(&parameterTableElement, &ParameterTable[Index], sizeof(ParameterTableElementType));
	    return parameterTableElement;
    }
	
	StdReturnType getMsgParameterByOptionShortName(char OptionShortName, ParameterTableElementType& sMsgOption) const {
		for(size_t index = 0; index < ParameterTable.size(); index++) {
			ParameterTableElementType parameterTableElement = getParameterTableElement(index);
			if(parameterTableElement.getOptionShortName() == OptionShortName) {
				sMsgOption = parameterTableElement;
				return E_OK;
			}
		}
		return E_NOT_OK;
	}
	
	PositionType parseArgument(MsgParameter Option, const char* Argument) {
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_CHAR) { return convertArgument<char>(Option, Argument); }
		if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT8) { return convertArgument<uint8_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT16) { return convertArgument<uint16_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT32) { return convertArgument<uint32_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT64) { return convertArgument<uint64_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT8) { return convertArgument<int8_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT16) { return convertArgument<int16_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT32) { return convertArgument<int32_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_INT64) { return convertArgument<int64_t>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_FLOAT) { return convertArgument<float>(Option, Argument); }
		//if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_DOUBLE) { return convertArgument<double>(Option, Argument); }
		return 0u;
	}

	template <typename T> PositionType convertArgument(MsgParameter Option, const char* Argument) {
		T value;
		PositionType position;
		
		StringTools::ResultType result = StringTools::stringTo(Argument, position, value, ArgumentNumberBase);
		handleConvertResult(result);
		if(result == StringTools::RESULT_OK) static_cast<Derived*>(this)->handleParameter(Option.getOptionShortName(), value);
		return position;
	}
	
	void handleConvertResult(StringTools::ResultType Result) const {
		if(Result == StringTools::RESULT_NO_VALUE) {
			Error.send(ErrorMessage::ERROR_NO_VALUE_GIVEN);
		}
		if(Result == StringTools::RESULT_OVERFLOW) {
			Error.send(ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
		}
	}

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgParameterParser(const ParameterTableType& sParameterTable, const char* sParameter)
	: Parameter(sParameter), ParameterTable(sParameterTable) {
		
	}

	// get methods
	
	const char* getParameter() const { return Parameter; }
	const ParameterTableType& getParameterTable();

	// set methods

	// methods
	void parse()
	{	
		StateType State = STATE_PARSE;
		char optionChar;
		
		for(PositionType position = 0; Parameter[position] != '\0'; position++) {
			if(State == STATE_PARSE) {
				char currentChar = Parameter[position];
				if(currentChar == OptionStartChar) { State = STATE_OPTION_CHAR; }
			} else if(State == STATE_OPTION_CHAR) {
				optionChar = Parameter[position];
				State = STATE_OPTION_ARGUMENT;
			} else if(State == STATE_OPTION_ARGUMENT) {
				ParameterTableElementType option;
				if(getMsgParameterByOptionShortName(optionChar, option) == E_OK) {
					position += parseArgument(option, &Parameter[position]) - 1u;
				}
				State = STATE_PARSE;
			}
		}
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
