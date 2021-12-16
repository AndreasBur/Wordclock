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
#include "MsgCmdAnimationParser.h"
#include "MsgCmdTimeParser.h"
#include "MsgCmdDateParser.h"

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

    if(command == COMMAND_REMOTE_PROCEDURE_CALL) {
        MsgCmdRemoteProcedureCallParser cmdRemoteProcedureCallParser(parameter);
        cmdRemoteProcedureCallParser.parse();
        cmdRemoteProcedureCallParser.process();
        cmdRemoteProcedureCallParser.sendAnswer();
    } else if(command == COMMAND_DISPLAY_COLOR) {
        MsgCmdDisplayColorParser cmdDisplayColorParser(parameter);
        cmdDisplayColorParser.parse();
        cmdDisplayColorParser.process();
        cmdDisplayColorParser.sendAnswer();
    } else if(command == COMMAND_DISPLAY_BRIGHTNESS) {
        MsgCmdDisplayBrightnessParser cmdDisplayBrightnessParser(parameter);
        cmdDisplayBrightnessParser.parse();
        cmdDisplayBrightnessParser.process();
        cmdDisplayBrightnessParser.sendAnswer();
    } else if(command == COMMAND_DISPLAY_PIXEL) {
        MsgCmdDisplayPixelParser cmdDisplayPixelParser(parameter);
        cmdDisplayPixelParser.parse();
        cmdDisplayPixelParser.process();
        cmdDisplayPixelParser.sendAnswer();
    } else if(command == COMMAND_ANIMATION) {
        MsgCmdAnimationParser cmdAnimationParser(parameter);
        cmdAnimationParser.parse();
        cmdAnimationParser.process();
        cmdAnimationParser.sendAnswer();
    } else if(command == COMMAND_CLOCK_MODE) {
        MsgCmdClockModeParser cmdClockModeParser(parameter);
        cmdClockModeParser.parse();
        cmdClockModeParser.process();
        cmdClockModeParser.sendAnswer();
    } else if(command == COMMAND_TIME) {
        MsgCmdTimeParser cmdClockTimeParser(parameter);
        cmdClockTimeParser.parse();
        cmdClockTimeParser.process();
        cmdClockTimeParser.sendAnswer();
    } else if(command == COMMAND_DATE) {
        MsgCmdDateParser cmdClockDateParser(parameter);
        cmdClockDateParser.parse();
        cmdClockDateParser.process();
        cmdClockDateParser.sendAnswer();
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    } else if(command == COMMAND_OVERLAY_DATE) {
        MsgCmdOverlayDateParser cmdOverlayDateParser(parameter);
        cmdOverlayDateParser.parse();
        cmdOverlayDateParser.process();
        cmdOverlayDateParser.sendAnswer();
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    } else if(command == COMMAND_OVERLAY_TEMPERATURE) {
        MsgCmdOverlayTemperatureParser cmdOverlayTemperatureParser(parameter);
        cmdOverlayTemperatureParser.parse();
        cmdOverlayTemperatureParser.process();
        cmdOverlayTemperatureParser.sendAnswer();
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    } else if(command == COMMAND_OVERLAY_TEXT) {
        MsgCmdOverlayTextParser MsgCmdOverlayTextParser(parameter);
        MsgCmdOverlayTextParser.parse();
        MsgCmdOverlayTextParser.process();
        MsgCmdOverlayTextParser.sendAnswer();
#endif
    } else {
        Error.send(ErrorMessage::ERROR_WRONG_COMMAND);
    }

    Serial.println();
}


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
