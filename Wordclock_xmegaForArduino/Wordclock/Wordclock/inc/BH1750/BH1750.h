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
/**     \file       BH1750.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _BH1750_H_
#define _BH1750_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* BH1750 configuration parameter */
#define BH1750_I2C_ADDR									0x76u

/* BH1750 parameter */
#define BH1750_REG_MT_MIN_VALUE							31u
#define BH1750_REG_MT_MAX_VALUE							254u
#define BH1750_REG_MT_DEFAULT_VALUE						69u

#define BH1750_CMD_POWER_DOWN							0b00000000u
#define BH1750_CMD_POWER_ON								0b00000001u
#define BH1750_CMD_RESET								0b00000111u

#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE				0b00010000u
#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2			0b00010001u
#define BH1750_CMD_CONTINUOUS_LOW_RES_MODE				0b00010011u

#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE				0b00100000u
#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2				0b00100001u
#define BH1750_CMD_ONE_TIME_LOW_RES_MODE				0b00100011u

#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS	0b01000000u
#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS     0b01100000u

#define BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER			1.2f

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class BH1750
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum ModeType {
        MODE_NONE,
        MODE_CONTINUOUS_LOW_RES_MODE = BH1750_CMD_CONTINUOUS_LOW_RES_MODE,
        MODE_CONTINUOUS_HIGH_RES_MODE = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE,
        MODE_CONTINUOUS_HIGH_RES_MODE_2 = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2,
        MODE_ONE_TIME_LOW_RES_MODE = BH1750_CMD_ONE_TIME_LOW_RES_MODE,
        MODE_ONE_TIME_HIGH_RES_MODE = BH1750_CMD_ONE_TIME_HIGH_RES_MODE,
        MODE_ONE_TIME_HIGH_RES_MODE_2 = BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2
    };
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    ModeType Mode;
    uint16_t Illuminance;

    // functions
    stdReturnType readIlluminance();
    void sendModeForOneTimeMode();
	stdReturnType sendCommand(byte);

	stdReturnType sendMode() { return sendCommand(Mode); }
    uint16_t convertRawToLux(uint16_t IlluminanceRaw) { return IlluminanceRaw / BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER; }
    uint16_t combineRawValueParts(byte HighByte, byte LowByte) { return static_cast<uint16_t>(HighByte) << 8 | LowByte; }
    bool isMTRegValueInRange(byte MTRegValue) { return (MTRegValue <= BH1750_REG_MT_MAX_VALUE && MTRegValue >= BH1750_REG_MT_MIN_VALUE); }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    BH1750();
    ~BH1750();

	// get methods
    ModeType getMode() const { return Mode; }
    uint16_t getIlluminance() const { return Illuminance; }

	// set methods
    stdReturnType setMode(ModeType);

	// methods
    stdReturnType init(ModeType);
    stdReturnType changeMeasurementTime(byte);
    void task();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
