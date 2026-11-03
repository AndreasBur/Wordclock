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
    using ColorType = Pixel::ColorType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    static constexpr char kRedOptionShortName{'R'};
    static constexpr char kGreenOptionShortName{'G'};
    static constexpr char kBlueOptionShortName{'B'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kRedOptionShortName, MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kGreenOptionShortName, MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kBlueOptionShortName, MsgParameter::kArgumentTypeUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    static void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kRedOptionShortName) { Display::GetInstance().SetColorRed(argument); }
        if(parameter_short_name == kGreenOptionShortName) { Display::GetInstance().SetColorGreen(argument); }
        if(parameter_short_name == kBlueOptionShortName) { Display::GetInstance().SetColorBlue(argument); }
    }

    static void SendAnswerRed(bool append_space) { SendAnswerParameter(kRedOptionShortName, Display::GetInstance().GetColorRed(), append_space); }
    static void SendAnswerGreen(bool append_space) { SendAnswerParameter(kGreenOptionShortName, Display::GetInstance().GetColorGreen(), append_space); }
    static void SendAnswerBlue(bool append_space) { SendAnswerParameter(kBlueOptionShortName, Display::GetInstance().GetColorBlue(), append_space); }

    void Show() const
    {
        StdReturnType return_value = Display::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::kApiDisplayShow, return_value, ErrorMessage::kErrorDisplayPending);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayColorParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdDisplayColorParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer()
    {
        SendAnswerRed(true);
        SendAnswerGreen(true);
        SendAnswerBlue(false);
    }

    void Process() const { Show(); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
