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
/**     \file       MsgCmdClockModeParser.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_CLOCK_MODE_PARSER_H_
#define _MSG_CMD_CLOCK_MODE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdClockModeParser configuration parameter */


/* MsgCmdClockModeParser parameter */
#define MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE           1u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdClockModeParser : public MsgParameterParser<MsgCmdClockModeParser, MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    static constexpr char ModeOptionShortName{'M'};
        
    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(ModeOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };
    
    // functions// functions
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == ModeOptionShortName) {
            Clock::getInstance().setMode(static_cast<Clock::ModeType>(Argument));
        }
    }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdClockModeParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdClockModeParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer()
    {
        Serial.print(ModeOptionShortName);
        Serial.print(OptionArgumentDelimiter);
        Serial.print(Display::getInstance().getColorRed());
    }
    
    void process() { Clock::getInstance().show(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
