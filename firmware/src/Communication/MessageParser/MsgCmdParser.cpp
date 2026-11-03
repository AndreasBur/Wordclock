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
#include "MsgCmdNetworkParser.h"
#include "MsgCmdColorCycleParser.h"
#include "MsgCmdConsoleParser.h"
#include "MsgCmdNightSwitchParser.h"
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
 * string, then parse -> Process -> SendAnswer. This helper runs that sequence so
 * the dispatch below stays a flat table instead of a copy-pasted if/else chain. */
template <typename ParserType>
void DispatchCommand(const char* parameter)
{
    ParserType parser(parameter);
    parser.Parse();
    parser.Process();
    parser.SendAnswer();
}
} /* namespace */


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Parse()
******************************************************************************************************************************************************/
void MsgCmdParser::Parse()
{
    CommandType command = GetCommand();
    const char* parameter = GetParameter();

    if(command != CommandType::kNone) SendAnswer(command);

    switch(command) {
        case CommandType::kRemoteProcedureCall: DispatchCommand<MsgCmdRemoteProcedureCallParser>(parameter); break;
        case CommandType::kDisplayColor:         DispatchCommand<MsgCmdDisplayColorParser>(parameter);        break;
        case CommandType::kDisplayBrightness:    DispatchCommand<MsgCmdDisplayBrightnessParser>(parameter);   break;
        case CommandType::kDisplayPixel:         DispatchCommand<MsgCmdDisplayPixelParser>(parameter);        break;
        case CommandType::kAnimation:             DispatchCommand<MsgCmdAnimationParser>(parameter);           break;
        case CommandType::kClockMode:            DispatchCommand<MsgCmdClockModeParser>(parameter);           break;
        case CommandType::kTime:                  DispatchCommand<MsgCmdTimeParser>(parameter);                break;
        case CommandType::kDate:                  DispatchCommand<MsgCmdDateParser>(parameter);                break;
        case CommandType::kStatus:                DispatchCommand<MsgCmdStatusParser>(parameter);              break;
        case CommandType::kNetwork:               DispatchCommand<MsgCmdNetworkParser>(parameter);             break;
        case CommandType::kNightSwitch:          DispatchCommand<MsgCmdNightSwitchParser>(parameter);         break;
        case CommandType::kColorCycle:           DispatchCommand<MsgCmdColorCycleParser>(parameter);          break;
        case CommandType::kConsole:               DispatchCommand<MsgCmdConsoleParser>(parameter);             break;
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        case CommandType::kOverlayDate:          DispatchCommand<MsgCmdOverlayDateParser>(parameter);         break;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        case CommandType::kOverlayTemperature:   DispatchCommand<MsgCmdOverlayTemperatureParser>(parameter);  break;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        case CommandType::kOverlayText:          DispatchCommand<MsgCmdOverlayTextParser>(parameter);         break;
#endif
        default:                            error_.Send(ErrorMessage::ErrorType::kWrongCommand, false);        break;
    }

    Serial.println();
}


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
