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
/**     \file       MsgCmdDisplayColorParser.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_DISPLAY_COLOR_PARSER_H_
#define _MSG_CMD_DISPLAY_COLOR_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdParserCommon.h"
#include "NeoPixel.h"
#include "Display.h"
#include "MsgParameterParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdDisplayColorParser configuration parameter */


/* MsgCmdDisplayColorParser parameter */
#define MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE           3u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdDisplayColorParser : public MsgParameterParser<MsgCmdDisplayColorParser, MSG_CMD_DISPLAY_COLOR_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
	using ColorType = NeoPixel::ColorType;
	
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
  	friend class MsgParameterParser;
	static const ParameterTableType ParameterTable;

	static const char RedParameterChar{'R'};
	static const char GreenParameterChar{'G'};
	static const char BlueParameterChar{'B'};
	static const char ColorValueDelimiter{':'};
		
	ErrorMessage Error;
		
	// functions
	void sendAnswerRed();
	void sendAnswerGreen();
	void sendAnswerBlue();
	
	void handleParameter(char, byte);
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    MsgCmdDisplayColorParser(const char*);
    ~MsgCmdDisplayColorParser();

	// get methods
	const ParameterTableType& getParameterTable() const { return ParameterTable; }

	// set methods

	// methods
	void sendAnswer();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
