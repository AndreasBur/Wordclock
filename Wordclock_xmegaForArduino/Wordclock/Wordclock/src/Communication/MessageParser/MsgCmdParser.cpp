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
#include "MsgCmdDisplayColorParser.h"
#include "MsgCmdDisplayBrightnessParser.h"
#include "MsgCmdDisplayPixelParser.h"
#include "MsgCmdClockTimeParser.h"
#include "MsgCmdAnimationClockParser.h"

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

    sendAnswer(command);

    if(command == COMMAND_DISPLAY_COLOR) {
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
    } else if(command == COMMAND_ANIMATION_CLOCK) {
        MsgCmdAnimationClockParser cmdAnimationClockParser(parameter);
        cmdAnimationClockParser.parse();
        cmdAnimationClockParser.process();
        cmdAnimationClockParser.sendAnswer();
    } else if(command == COMMAND_CLOCK_TIME) {
        MsgCmdClockTimeParser cmdClockTimeParser(parameter);
        cmdClockTimeParser.parse();
        cmdClockTimeParser.process();
        cmdClockTimeParser.sendAnswer();
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
