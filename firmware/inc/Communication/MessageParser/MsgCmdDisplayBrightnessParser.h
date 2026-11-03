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
    static constexpr char kBrightnessOptionShortName{'B'};
    static constexpr char kUseAutomaticOptionShortName{'A'};
    static constexpr char kUseGammaCorrectionOptionShortName{'G'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        MsgParameter(kBrightnessOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        MsgParameter(kUseAutomaticOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        MsgParameter(kUseGammaCorrectionOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    static void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kBrightnessOptionShortName) {
            Display::GetInstance().SetBrightness(argument);
        }
        if(parameter_short_name == kUseAutomaticOptionShortName) {
            Display::GetInstance().SetBrightnessUseAutomatic(argument);
        }
        if(parameter_short_name == kUseGammaCorrectionOptionShortName) {
            Display::GetInstance().SetBrightnessUseGammaCorrection(argument);
        }
    }

     static void SendAnswerBrightness(bool append_space) { SendAnswerParameter(kBrightnessOptionShortName, Display::GetInstance().GetBrightness(), append_space); }
     static void SendAnswerAutomatic(bool append_space) { SendAnswerParameter(kUseAutomaticOptionShortName, Display::GetInstance().GetBrightnessUseAutomatic(), append_space); }
     static void SendAnswerGammaCorrection(bool append_space) { SendAnswerParameter(kUseGammaCorrectionOptionShortName, Display::GetInstance().GetBrightnessUseGammaCorrection(), append_space); }

    void Show() const
    {
        StdReturnType return_value = Display::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kDisplayShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdDisplayBrightnessParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdDisplayBrightnessParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer()
    {
        SendAnswerBrightness(true);
        SendAnswerAutomatic(true);
        SendAnswerGammaCorrection(false);

    }

    void Process() { Show(); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
