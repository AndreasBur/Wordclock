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
const MsgCmdDisplayColorParser::ParameterTableType MsgCmdDisplayColorParser::ParameterTable PROGMEM
{
	ParameterTableElementType(RedOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
	ParameterTableElementType(GreenOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
	ParameterTableElementType(BlueOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
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
MsgCmdDisplayColorParser::MsgCmdDisplayColorParser(const char* sParameter)
: MsgParameterParser(ParameterTable, sParameter)
{

}

/******************************************************************************************************************************************************
  Destructor of MsgCmdDisplayColorParser
******************************************************************************************************************************************************/
MsgCmdDisplayColorParser::~MsgCmdDisplayColorParser()
{

} /* ~MsgCmdDisplayColorParser */

void MsgCmdDisplayColorParser::sendAnswer()
{
	sendAnswerRed();
	sendAnswerGreen();
	sendAnswerBlue();
	Serial.println();
}

/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

void MsgCmdDisplayColorParser::sendAnswerRed()
{
	Serial.print(RedOptionShortName);
	Serial.print(OptionArgumentDelimiter);
	Serial.print(Display::getInstance().getColorRed());
}

void MsgCmdDisplayColorParser::sendAnswerGreen()
{
	Serial.print(GreenOptionShortName);
	Serial.print(OptionArgumentDelimiter);
	Serial.print(Display::getInstance().getColorGreen());
}

void MsgCmdDisplayColorParser::sendAnswerBlue()
{
	Serial.print(BlueOptionShortName);
	Serial.print(OptionArgumentDelimiter);
	Serial.print(Display::getInstance().getColorBlue());
}

void MsgCmdDisplayColorParser::handleParameter(char ParameterShortName, byte Argument)
{
	if(ParameterShortName == RedOptionShortName) { Display::getInstance().setColorRed(Argument); }
	if(ParameterShortName == GreenOptionShortName) { Display::getInstance().setColorGreen(Argument); }
	if(ParameterShortName == BlueOptionShortName) { Display::getInstance().setColorBlue(Argument); }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 