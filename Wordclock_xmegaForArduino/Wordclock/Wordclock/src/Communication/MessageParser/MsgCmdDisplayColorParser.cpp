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
/**     \file       MsgCmdDisplayColorParser.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _MSG_CMD_DISPLAY_COLOR_PARSER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "MsgCmdDisplayColorParser.h"
#include "StringTools.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const MsgCmdDisplayColorParser::OptionTableType MsgCmdDisplayColorParser::OptionTable PROGMEM
{
	OptionTableElementType('R', MsgOption::VALUE_TYPE_UINT8),
	OptionTableElementType('G', MsgOption::VALUE_TYPE_UINT8),
	OptionTableElementType('B', MsgOption::VALUE_TYPE_UINT8)
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of MsgCmdDisplayColorParser
******************************************************************************************************************************************************/
/*! \brief          MsgCmdDisplayColorParser Constructor
 *  \details        Instantiation of the MsgCmdDisplayColorParser library
 *
 *  \return         -
******************************************************************************************************************************************************/
MsgCmdDisplayColorParser::MsgCmdDisplayColorParser(const char* sCmdValue) : MsgCmdOptionParser(sCmdValue)
{

} /* MsgCmdDisplayColorParser */


/******************************************************************************************************************************************************
  Destructor of MsgCmdDisplayColorParser
******************************************************************************************************************************************************/
MsgCmdDisplayColorParser::~MsgCmdDisplayColorParser()
{

} /* ~MsgCmdDisplayColorParser */


void MsgCmdDisplayColorParser::parse()
{
	if(CmdValue == nullptr) {
		sendAnswer();
	} else {
		setColors(CmdValue);
		sendAnswer();
	}
}

void MsgCmdDisplayColorParser::sendAnswer()
{
	sendAnswerRed();
	sendAnswerGreen();
	sendAnswerBlue();
	Serial.println();
}

void MsgCmdDisplayColorParser::sendAnswerRed()
{
	Serial.print(RedParameterChar);
	Serial.print(ColorValueDelimiter);
	Serial.print(Display::getInstance().getColorRed());
}

void MsgCmdDisplayColorParser::sendAnswerGreen()
{
	Serial.print(GreenParameterChar);
	Serial.print(ColorValueDelimiter);
	Serial.print(Display::getInstance().getColorGreen());
}

void MsgCmdDisplayColorParser::sendAnswerBlue()
{
	Serial.print(BlueParameterChar);
	Serial.print(ColorValueDelimiter);
	Serial.print(Display::getInstance().getColorBlue());
}


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

void MsgCmdDisplayColorParser::setColors(const char* CmdValue)
{
	setColorRed(CmdValue);
	setColorGreen(CmdValue);
	setColorBlue(CmdValue);
}

void MsgCmdDisplayColorParser::setColorRed(const char* CmdValue)
{
	uint8_t Value;
	
	if(getColorValue(CmdValue, RedParameterChar, Value) == E_OK) {
		Display::getInstance().setColorRed(Value);
	}
}

void MsgCmdDisplayColorParser::setColorGreen(const char* CmdValue)
{
	uint8_t Value;
	
	if(getColorValue(CmdValue, GreenParameterChar, Value) == E_OK) {
		Display::getInstance().setColorGreen(Value);
	}
}

void MsgCmdDisplayColorParser::setColorBlue(const char* CmdValue)
{
	uint8_t Value;
	
	if(getColorValue(CmdValue, BlueParameterChar, Value) == E_OK) {
		Display::getInstance().setColorBlue(Value);
	}
}

stdReturnType MsgCmdDisplayColorParser::getColorValue(const char* CmdValue, char ColorParameterChar, ColorType& Value)
{
	char parameterAndDelimiter[]{ColorParameterChar, ColorValueDelimiter, '\0'};
	const char* parameterStart = strstr(CmdValue, parameterAndDelimiter);
	const char* valueStart = parameterStart + 2u;
	StringTools::ResultType result = StringTools::stringToUnsignedInteger(valueStart, 10, Value);
	
	if((result == StringTools::RESULT_OVERFLOW)) {
		Error.send(ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
		return E_NOT_OK;
	} else if (result == StringTools::RESULT_NO_VALUE){
		Error.send(ErrorMessage::ERROR_NO_VALUE_GIVEN);
		return E_NOT_OK;
	} else {
		return E_OK;
	}
}


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 