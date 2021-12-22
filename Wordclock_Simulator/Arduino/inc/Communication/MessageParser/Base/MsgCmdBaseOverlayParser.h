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
/**     \file       MsgCmdBaseOverlayParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_BASE_OVERLAY_PARSER_H_
#define _MSG_CMD_BASE_OVERLAY_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdBaseOverlayParser configuration parameter */


/* MsgCmdBaseOverlayParser parameter */
#define MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE           8u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdBaseOverlayParser
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using MsgParameterBaseParserType = MsgParameterParser<MsgCmdBaseOverlayParser, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
    using ParameterTableType = typename MsgParameterBaseParserType::ParameterTableType;
    using ParameterTableElementType = typename MsgParameterBaseParserType::ParameterTableElementType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  protected:
    static constexpr char PeriodOptionShortName{'P'};
    static constexpr char EnduranceOptionShortName{'E'};
    static constexpr char MonthOptionShortName{'M'};
    static constexpr char DayOptionShortName{'D'};
    static constexpr char ValidOptionShortName{'V'};
    static constexpr char ActiveOptionShortName{'A'};
	static constexpr char TextOptionShortName{'T'};
	static constexpr char TextSpeedOptionShortName{'S'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(PeriodOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(EnduranceOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(MonthOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(DayOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(ValidOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(ActiveOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
		ParameterTableElementType(TextOptionShortName, MsgParameter::ARGUMENT_TYPE_STRING),
		ParameterTableElementType(TextSpeedOptionShortName, MsgParameter::ARGUMENT_TYPE_STRING),
    };

    constexpr MsgCmdBaseOverlayParser() { }
    ~MsgCmdBaseOverlayParser() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:


	// get methods


	// set methods

	// methods

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
