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
    static constexpr char RedOptionShortName{'R'};
    static constexpr char GreenOptionShortName{'G'};
    static constexpr char BlueOptionShortName{'B'};

    static constexpr ParameterTableType ParameterTable PROGMEM
    {
        ParameterTableElementType(RedOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(GreenOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(BlueOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == RedOptionShortName) { Display::getInstance().setColorRed(Argument); }
        if(ParameterShortName == GreenOptionShortName) { Display::getInstance().setColorGreen(Argument); }
        if(ParameterShortName == BlueOptionShortName) { Display::getInstance().setColorBlue(Argument); }
    }

    void sendAnswerRed() { sendAnswerParameter(RedOptionShortName, Display::getInstance().getColorRed()); }
    void sendAnswerGreen() { sendAnswerParameter(GreenOptionShortName, Display::getInstance().getColorGreen()); }
    void sendAnswerBlue() { sendAnswerParameter(BlueOptionShortName, Display::getInstance().getColorBlue()); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayColorParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdDisplayColorParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer()
    {
        sendAnswerRed();
        sendAnswerGreen();
        sendAnswerBlue();
    }

    void process() { Display::getInstance().show(); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
