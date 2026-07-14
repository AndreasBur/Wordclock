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
/**     \file       MsgCmdRemoteProcedureCallParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_
#define _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Illuminance.h"
#include "Display.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdRemoteProcedureCallParser configuration parameter */


/* MsgCmdRemoteProcedureCallParser parameter */
#define MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE           1u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdRemoteProcedureCallParser : public MsgParameterParser<MsgCmdRemoteProcedureCallParser, MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum RpcIdType
    {
        RPC_ID_NONE,
        RPC_ID_ILLUMINANCE_CALIBRATION_MAX_VALUE,
        RPC_ID_ILLUMINANCE_CALIBRATION_MIN_VALUE,
        RPC_ID_DISPLAY_ENABLE,
        RPC_ID_DISPLAY_DISABLE,
        RPC_ID_DISPLAY_SHOW,
        RPC_ID_DISPLAY_CLEAR,
        RPC_ID_DISPLAY_TEST,
        RPC_ID_DISPLAY_COLOR_RED_INCREMENT,
        RPC_ID_DISPLAY_COLOR_GREEN_INCREMENT,
        RPC_ID_DISPLAY_COLOR_BLUE_INCREMENT,
        RPC_ID_DISPLAY_COLOR_RED_DECREMENT,
        RPC_ID_DISPLAY_COLOR_GREEN_DECREMENT,
        RPC_ID_DISPLAY_COLOR_BLUE_DECREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_INCREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_DECREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_ON,
        RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_OFF,
        RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_ON,
        RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_OFF,
        RPC_ID_POWER_ON,
        RPC_ID_POWER_OFF,
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    RpcIdType RpcId{RPC_ID_NONE};
    StdReturnType ReturnValue{E_OK};
    static constexpr char RemoteProcedureShortName{'P'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(RemoteProcedureShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length); }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == RemoteProcedureShortName) { RpcId = static_cast<RpcIdType>(Argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdRemoteProcedureCallParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdRemoteProcedureCallParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const {
        Serial.print(F("RpcId="));
        Serial.println(RpcId);
        Error.send(ReturnValue);
    }

    void process()
    {
        switch(RpcId) {
            case RPC_ID_ILLUMINANCE_CALIBRATION_MAX_VALUE :
                Illuminance::getInstance().startCalibrationMaxValue();
                break;
            case RPC_ID_ILLUMINANCE_CALIBRATION_MIN_VALUE :
                Illuminance::getInstance().startCalibrationMinValue();
                break;
            case RPC_ID_DISPLAY_ENABLE :
                Display::getInstance().enable();
                break;
            case RPC_ID_DISPLAY_DISABLE :
                Display::getInstance().disable();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_INCREMENT :
                Display::getInstance().incrementBrightness();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_DECREMENT :
                Display::getInstance().decrementBrightness();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_ON :
                Display::getInstance().enableBrightnessAutomatic();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_OFF :
                Display::getInstance().disableBrightnessGammaCorrection();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_ON :
                Display::getInstance().enbleBrightnessGammaCorrection();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_OFF :
                Display::getInstance().disableBrightnessAutomatic();
                break;
            case RPC_ID_DISPLAY_COLOR_RED_INCREMENT :
                Display::getInstance().incrementColorRed();
                break;
            case RPC_ID_DISPLAY_COLOR_GREEN_INCREMENT :
                Display::getInstance().incrementColorGreen();
                break;
            case RPC_ID_DISPLAY_COLOR_BLUE_INCREMENT :
                Display::getInstance().incrementColorBlue();
                break;
            case RPC_ID_DISPLAY_COLOR_RED_DECREMENT :
                Display::getInstance().decrementColorRed();
                break;
            case RPC_ID_DISPLAY_COLOR_GREEN_DECREMENT :
                Display::getInstance().decrementColorGreen();
                break;
            case RPC_ID_DISPLAY_COLOR_BLUE_DECREMENT :
                Display::getInstance().decrementColorBlue();
                break;
            case RPC_ID_DISPLAY_SHOW :
                ReturnValue = Display::getInstance().show();
                break;
            case RPC_ID_DISPLAY_CLEAR :
                Display::getInstance().clear();
                break;
            case RPC_ID_DISPLAY_TEST :
                Display::getInstance().test();
                break;
            default:
                break;
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
