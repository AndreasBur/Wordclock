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
/**     \file       Communication.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _COMMUNICATION_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Communication.h"


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
  task()
******************************************************************************************************************************************************/
void Communication::task()
{
    if(isMessageComplete()) {
        MsgCmdParser CmdParser(IncomingMessage);
        CmdParser.parse();
    } else {
        addMessagePart();
    }
}


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  addMessagePart()
******************************************************************************************************************************************************/
void Communication::addMessagePart()
{
/*
    while (Serial.available()) {
        // get the new byte from uart
        char inChar = static_cast<char>(Serial.read());
        if(inChar == EndOfMessageChar) { State = STATE_MESSAGE_COMPLETE; }
        if(IncomingMessage.addChar(inChar) == E_NOT_OK) {
            Error.send(ErrorMessage::ERROR_MESSAGE_TOO_LONG);
        }
    }
*/
    char Buffer[] = "4 -S50 -A1\n";
    
    for(uint8_t i = 0u; i <= strlen(Buffer); i++)
    {
        if(Buffer[i] == EndOfMessageChar) { State = STATE_MESSAGE_COMPLETE; }
        else IncomingMessage.addChar(Buffer[i]);
    }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 