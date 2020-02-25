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
template <size_t ParameterTableSize> class MsgParameterParser
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
    const char* Parameter;
	
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
  
	static const char OptionStartChar{'-'};
	
	// functions
    ParameterTableElementType getParameterTableElement(byte Index) const {
	    ParameterTableElementType parameterTableElement;
	    memcpy_P(&parameterTableElement, &getParameterTable()[Index], sizeof(ParameterTableElementType));
	    return parameterTableElement;
    }
	
	stdReturnType getMsgParameterByOptionShortName(char OptionShortName, ParameterTableElementType& sMsgOption) {
		for(size_t index = 0; index < getParameterTable().size(); index++) {
			ParameterTableElementType parameterTableElement = getParameterTableElement(index);
			if(parameterTableElement.getOptionShortName() == OptionShortName) {
				sMsgOption = parameterTableElement;
				return E_OK;
			}
		}
		return E_NOT_OK;
	}
	
	PositionType parseArgument(MsgParameter Option, const char* Argument) {
		if(Option.getArgumentType() == MsgParameter::ARGUMENT_TYPE_UINT8) {
			PositionType position;
			uint8_t value;
			StringTools::stringToUnsignedInteger(Argument, position, 10, value);
			return position;
		}
	}

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgParameterParser(const char* sParameter) : Parameter(sParameter) {
		
	}

	// get methods
	
	const char* getParameter() const { return Parameter; }
	virtual const ParameterTableType& getParameterTable() const = 0;

	// set methods

	// methods
	void parse()
	{
		for(PositionType position = 0; Parameter[position] != '\0'; position++) {
			char currentChar = Parameter[position];
			if(currentChar == OptionStartChar) {
				char optionChar = Parameter[++position];
				ParameterTableElementType option;
				if(getMsgParameterByOptionShortName(optionChar, option) == E_OK) {
					position += parseArgument(option, &Parameter[++position]);
				}
			}
		}
	}
	
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
