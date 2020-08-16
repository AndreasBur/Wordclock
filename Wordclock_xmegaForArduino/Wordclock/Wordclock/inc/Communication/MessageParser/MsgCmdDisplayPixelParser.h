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
/**     \file       MsgCmdDisplayPixelParser.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_DISPLAY_PIXEL_PARSER_H_
#define _MSG_CMD_DISPLAY_PIXEL_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "MsgParameterParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdDisplayPixelParser configuration parameter */


/* MsgCmdDisplayPixelParser parameter */
#define MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE           2u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdDisplayPixelParser : public MsgParameterParser<MsgCmdDisplayPixelParser, MSG_CMD_DISPLAY_PIXEL_PARSER_PARAMETER_TABLE_SIZE>
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
    byte Index{0u};
    boolean State{false};
    
    static constexpr char IndexOptionShortName{'I'};
    static constexpr char StateOptionShortName{'S'};
    
    static constexpr ParameterTableType ParameterTable PROGMEM
    {
        ParameterTableElementType(IndexOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(StateOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
    };
    
    // functions
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == IndexOptionShortName) { Index = Argument; }
        if(ParameterShortName == StateOptionShortName) { State = Argument; }
    }
    
    void sendAnswerIndex()
    {
        Serial.print(IndexOptionShortName);
        Serial.print(OptionArgumentDelimiter);
        Serial.print(Index);
    }

    void sendAnswerState()
    {
        Serial.print(StateOptionShortName);
        Serial.print(OptionArgumentDelimiter);
        Serial.print(Display::getInstance().getPixel(Index));
    }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayPixelParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdDisplayPixelParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer()
    {
        sendAnswerIndex();
        sendAnswerState();
    }
    
    void process() { Error.send(Error::API_DISPLAY_SHOW, Display::getInstance().show()); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
