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
/**     \file       BH1750.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _BH1750_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "BH1750.h"
#include "Wire.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/
#define BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES        2u

#define BH1750_MT_REG_VALUE_HIGH_BITS_GM                    0b111u
#define BH1750_MT_REG_VALUE_HIGH_BITS_GP                    5u

#define BH1750_MT_REG_VALUE_LOW_BITS_GM                     0b11111u
#define BH1750_MT_REG_VALUE_LOW_BITS_GP                     0u

#define BH1750_MT_CMD_HIGH_BITS_GM                          0b111u
#define BH1750_MT_CMD_HIGH_BITS_GP                          0u

#define BH1750_MT_CMD_LOW_BITS_GM                           0b11111u
#define BH1750_MT_CMD_LOW_BITS_GP                           0u

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
  Constructor of BH1750
******************************************************************************************************************************************************/
BH1750::BH1750()
{
    Mode = MODE_NONE;
    Illuminance = 0u;
} /* BH1750 */


/******************************************************************************************************************************************************
  Destructor of BH1750
******************************************************************************************************************************************************/
BH1750::~BH1750()
{

} /* ~BH1750 */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
StdReturnType BH1750::init(ModeType Mode)
{
    return setMode(Mode);
} /* init */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void BH1750::task()
{
    sendMode();
    readIlluminance();
} /* task */


/******************************************************************************************************************************************************
  setMode()
******************************************************************************************************************************************************/
StdReturnType BH1750::setMode(ModeType sMode)
{
    StdReturnType ReturnValue = E_NOT_OK;
    Mode = sMode;

    if(Mode != MODE_NONE) {
        ReturnValue = sendMode();
    }

    return ReturnValue;
} /* setMode */


/******************************************************************************************************************************************************
  changeMeasurementTime()
******************************************************************************************************************************************************/
StdReturnType BH1750::changeMeasurementTime(byte MTRegValue)
{
    StdReturnType ReturnValue = E_NOT_OK;

    if(isMTRegValueInRange(MTRegValue)) {
        byte MTRegValueHighBits = readBitGroup(MTRegValue, BH1750_MT_REG_VALUE_HIGH_BITS_GM, BH1750_MT_REG_VALUE_HIGH_BITS_GP);
        byte MTRegValueLowBits = readBitGroup(MTRegValue, BH1750_MT_REG_VALUE_LOW_BITS_GM, BH1750_MT_REG_VALUE_LOW_BITS_GP);
        
        ReturnValue = E_OK;
        
        byte CmdCMTHighBitsWithValue = BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS;
        writeBitGroup(CmdCMTHighBitsWithValue, BH1750_MT_CMD_HIGH_BITS_GM, BH1750_MT_CMD_HIGH_BITS_GP, MTRegValueHighBits);
        
        byte CmdCMTLowBitsWithValue = BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS;
        writeBitGroup(CmdCMTLowBitsWithValue, BH1750_MT_CMD_LOW_BITS_GM, BH1750_MT_CMD_LOW_BITS_GP, MTRegValueLowBits);
       
        if(sendCommand(CmdCMTHighBitsWithValue) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
        if(sendCommand(CmdCMTLowBitsWithValue) == E_NOT_OK) { ReturnValue = E_NOT_OK; }
        if(sendMode() == E_NOT_OK) { ReturnValue = E_NOT_OK; }
    }

    return ReturnValue;

} /* changeMeasurementTime */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  readIlluminance()
******************************************************************************************************************************************************/
StdReturnType BH1750::readIlluminance()
{
    StdReturnType ReturnValue = E_NOT_OK;
    byte LowByte, HighByte;

    Wire.requestFrom(BH1750_I2C_ADDR, static_cast<uint8_t>(BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES));
    
    if(Wire.available() == BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES) {
        ReturnValue = E_OK;
        HighByte = Wire.read();
        LowByte = Wire.read();
        Illuminance = convertRawToLux(combineRawValueParts(HighByte, LowByte));
    }

    return ReturnValue;
}


/******************************************************************************************************************************************************
  sendModeForOneTime()
******************************************************************************************************************************************************/
void BH1750::sendModeForOneTimeMode()
{
    switch(Mode) {
        case MODE_ONE_TIME_HIGH_RES_MODE :
        case MODE_ONE_TIME_HIGH_RES_MODE_2 :
        case MODE_ONE_TIME_LOW_RES_MODE :
            sendMode();
            break;
        default:
            break;
    }
}

/******************************************************************************************************************************************************
  sendCommand()
******************************************************************************************************************************************************/
StdReturnType BH1750::sendCommand(byte Command)
{
    Wire.beginTransmission(static_cast<uint8_t>(BH1750_I2C_ADDR));
    Wire.write(Command);
    if(Wire.endTransmission() == E_OK) { return E_OK; }
    else { return E_NOT_OK; }
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 
