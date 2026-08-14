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
/**     \file       MsgCmdParser.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _MSG_CMD_PARSER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "MsgCmdParser.h"
#include "MsgCmdRemoteProcedureCallParser.h"
#include "MsgCmdDisplayColorParser.h"
#include "MsgCmdDisplayBrightnessParser.h"
#include "MsgCmdDisplayPixelParser.h"
#include "MsgCmdClockModeParser.h"
#include "MsgCmdStatusParser.h"
#include "MsgCmdAnimationParser.h"
#include "MsgCmdTimeParser.h"
#include "MsgCmdDateParser.h"
#include "Overlays.h"
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
# include "MsgCmdOverlayDateParser.h"
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
# include "MsgCmdOverlayTemperatureParser.h"
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
# include "MsgCmdOverlayTextParser.h"
#endif

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
namespace {
/* Every command parser shares the same lifecycle: construct from the parameter
 * string, then parse -> process -> sendAnswer. This helper runs that sequence so
 * the dispatch below stays a flat table instead of a copy-pasted if/else chain. */
template <typename ParserType>
void dispatchCommand(const char* Parameter)
{
    ParserType parser(Parameter);
    parser.parse();
    parser.process();
    parser.sendAnswer();
}
} /* namespace */


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  parse()
******************************************************************************************************************************************************/
void MsgCmdParser::parse()
{
    CommandType command = getCommand();
    const char* parameter = getParameter();

    if(command != COMMAND_NONE) sendAnswer(command);

    switch(command) {
        case COMMAND_REMOTE_PROCEDURE_CALL: dispatchCommand<MsgCmdRemoteProcedureCallParser>(parameter); break;
        case COMMAND_DISPLAY_COLOR:         dispatchCommand<MsgCmdDisplayColorParser>(parameter);        break;
        case COMMAND_DISPLAY_BRIGHTNESS:    dispatchCommand<MsgCmdDisplayBrightnessParser>(parameter);   break;
        case COMMAND_DISPLAY_PIXEL:         dispatchCommand<MsgCmdDisplayPixelParser>(parameter);        break;
        case COMMAND_ANIMATION:             dispatchCommand<MsgCmdAnimationParser>(parameter);           break;
        case COMMAND_CLOCK_MODE:            dispatchCommand<MsgCmdClockModeParser>(parameter);           break;
        case COMMAND_TIME:                  dispatchCommand<MsgCmdTimeParser>(parameter);                break;
        case COMMAND_DATE:                  dispatchCommand<MsgCmdDateParser>(parameter);                break;
        case COMMAND_STATUS:                dispatchCommand<MsgCmdStatusParser>(parameter);              break;
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        case COMMAND_OVERLAY_DATE:          dispatchCommand<MsgCmdOverlayDateParser>(parameter);         break;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        case COMMAND_OVERLAY_TEMPERATURE:   dispatchCommand<MsgCmdOverlayTemperatureParser>(parameter);  break;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        case COMMAND_OVERLAY_TEXT:          dispatchCommand<MsgCmdOverlayTextParser>(parameter);         break;
#endif
        default:                            Error.send(ErrorMessage::ERROR_WRONG_COMMAND, false);        break;
    }

    Serial.println();
}


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
