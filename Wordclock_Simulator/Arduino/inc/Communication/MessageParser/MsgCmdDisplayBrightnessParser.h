/*************************************************************************************************************_MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_H_*****************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       MsgCmdDisplayBrightnessParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_H_
#define _MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixel.h"
#include "Display.h"
#include "MsgParameterParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdDisplayBrightnessParser configuration parameter */


/* MsgCmdDisplayBrightnessParser parameter */
#define MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE           3u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdDisplayBrightnessParser : public MsgParameterParser<MsgCmdDisplayBrightnessParser, MSG_CMD_DISPLAY_BRIGHTNESS_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    static constexpr char BrightnessOptionShortName{'B'};
    static constexpr char UseAutomaticOptionShortName{'A'};
    static constexpr char UseGammaCorrectionOptionShortName{'G'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        MsgParameter(BrightnessOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        MsgParameter(UseAutomaticOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        MsgParameter(UseGammaCorrectionOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length); }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == BrightnessOptionShortName) {
            Display::getInstance().setBrightness(Argument);
        }
        if(ParameterShortName == UseAutomaticOptionShortName) {
            Display::getInstance().setBrightnessUseAutomatic(Argument);
        }
        if(ParameterShortName == UseGammaCorrectionOptionShortName) {
            Display::getInstance().setBrightnessUseGammaCorrection(Argument);
        }
    }

     void sendAnswerBrightness(bool AppendSpace) const { sendAnswerParameter(BrightnessOptionShortName, Display::getInstance().getBrightness(), AppendSpace); }
     void sendAnswerAutomatic(bool AppendSpace) const { sendAnswerParameter(UseAutomaticOptionShortName, Display::getInstance().getBrightnessUseAutomatic(), AppendSpace); }
     void sendAnswerGammaCorrection(bool AppendSpace) const { sendAnswerParameter(UseGammaCorrectionOptionShortName, Display::getInstance().getBrightnessUseGammaCorrection(), AppendSpace); }

    void show() const
    {
        StdReturnType returnValue = Display::getInstance().show();
        Error.checkReturnValueAndSend(ErrorMessage::API_DISPLAY_SHOW, returnValue, ErrorMessage::ERROR_DISPLAY_PENDING);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayBrightnessParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdDisplayBrightnessParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer()
    {
        sendAnswerBrightness(true);
        sendAnswerAutomatic(true);
        sendAnswerGammaCorrection(false);

    }

    void process() { show(); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
