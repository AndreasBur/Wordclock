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
#include "BH1750.h"
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
        RPC_ID_BH1750_CALIBRATION_MAX_VALUE,
        RPC_ID_BH1750_CALIBRATION_MIN_VALUE,
        RPC_ID_DISPLAY_ENABLE,
        RPC_ID_DISPLAY_DISABLE,
        RPC_ID_DISPLAY_CLEAR,
        RPC_ID_DISPLAY_TEST,

        
    };
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    RpcIdType RpcId;
    static constexpr char RemoteProcedureShortName{'P'};
        
    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(RemoteProcedureShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };
    
    // functions
    void handleParameter(char ParameterShortName, byte Argument)
    {
        RpcId = static_cast<RpcIdType>(Argument);
        
        switch(RpcId) {
            case RPC_ID_BH1750_CALIBRATION_MAX_VALUE :
                BH1750::getInstance().startCalibrationMaxValue();
                break;
            case RPC_ID_BH1750_CALIBRATION_MIN_VALUE :
                BH1750::getInstance().startCalibrationMinValue();
                break;
            case RPC_ID_DISPLAY_ENABLE :
                Display::getInstance().enable();
                break;
            case RPC_ID_DISPLAY_DISABLE :
                Display::getInstance().disable();
                break;
            case RPC_ID_DISPLAY_CLEAR :
                Display::getInstance().clear();
                break;
            case RPC_ID_DISPLAY_TEST :
                Display::getInstance().test();
                break;
            default:
        }
    }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdRemoteProcedureCallParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter), RpcId(RPC_ID_NONE) { }
    ~MsgCmdRemoteProcedureCallParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() {
        Serial.print(F("RpcId: "));
        Serial.print(RpcId);
    }
    
    void process() { }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
