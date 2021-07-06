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
#define BH1750_I2C_ADDR                                 0x76u

/* BH1750 parameter */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u
#define BH1750_REG_MT_MIN_VALUE                         31u
#define BH1750_REG_MT_MAX_VALUE                         254u
#define BH1750_REG_MT_DEFAULT_VALUE                     69u

#define BH1750_CMD_POWER_DOWN                           0b00000000u
#define BH1750_CMD_POWER_ON                             0b00000001u
#define BH1750_CMD_RESET                                0b00000111u

#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE             0b00010000u
#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2           0b00010001u
#define BH1750_CMD_CONTINUOUS_LOW_RES_MODE              0b00010011u

#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE               0b00100000u
#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2             0b00100001u
#define BH1750_CMD_ONE_TIME_LOW_RES_MODE                0b00100011u

#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS    0b01000000u
#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS     0b01100000u

#define BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER            1.2f

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   B H 1 7 5 0
******************************************************************************************************************************************************/
class BH1750
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using IlluminanceType = uint16_t;

    enum ModeType {
        MODE_NONE,
        MODE_CONTINUOUS_LOW_RES_MODE = BH1750_CMD_CONTINUOUS_LOW_RES_MODE,
        MODE_CONTINUOUS_HIGH_RES_MODE = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE,
        MODE_CONTINUOUS_HIGH_RES_MODE_2 = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2,
        MODE_ONE_TIME_LOW_RES_MODE = BH1750_CMD_ONE_TIME_LOW_RES_MODE,
        MODE_ONE_TIME_HIGH_RES_MODE = BH1750_CMD_ONE_TIME_HIGH_RES_MODE,
        MODE_ONE_TIME_HIGH_RES_MODE_2 = BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2
    };

    struct CalibrationValuesType {
        IlluminanceType MaxValue;
        IlluminanceType MinValue;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ModeType Mode;
    IlluminanceType Illuminance;
    CalibrationValuesType CalibrationValues;

    // functions
    BH1750() : Mode(MODE_NONE), Illuminance(BH1750_ILLUMINANCE_MIN_LX_VALUE),
               CalibrationValues{BH1750_ILLUMINANCE_MAX_LX_VALUE, BH1750_ILLUMINANCE_MIN_LX_VALUE} { }
    ~BH1750() { }

    StdReturnType readIlluminance();
    void sendModeForOneTimeMode();
    StdReturnType sendCommand(byte);

    StdReturnType sendMode() { return sendCommand(Mode); }
    IlluminanceType convertRawToLux(uint16_t IlluminanceRaw) const { return IlluminanceRaw / BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER; }
    IlluminanceType combineRawValueParts(byte HighByte, byte LowByte) const { return static_cast<uint16_t>(HighByte) << 8u | LowByte; }
    bool isMTRegValueInRange(byte MTRegValue) const { return (MTRegValue <= BH1750_REG_MT_MAX_VALUE && MTRegValue >= BH1750_REG_MT_MIN_VALUE); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static BH1750& getInstance() {
        static BH1750 singletonInstance;
        return singletonInstance;
    }

    // get methods
    ModeType getMode() const { return Mode; }
    IlluminanceType getIlluminance() const { return Illuminance; }
    CalibrationValuesType getCalibrationValues() const { return CalibrationValues; }
    IlluminanceType getCalibrationValuesMaxValue() const { return CalibrationValues.MaxValue; }
    IlluminanceType getCalibrationValuesMinValue() const { return CalibrationValues.MinValue; }

    // set methods
    StdReturnType setMode(ModeType);
    void setCalibrationValuesMaxValue(IlluminanceType MaxValue) { CalibrationValues.MaxValue = MaxValue; }
    void setCalibrationValuesMinValue(IlluminanceType MinValue) { CalibrationValues.MinValue = MinValue; }
    void setCalibrationValues(CalibrationValuesType sCalibrationValues) { CalibrationValues = sCalibrationValues; }

    // methods
    StdReturnType init(ModeType);
    StdReturnType changeMeasurementTime(byte);
    void startCalibrationMaxValue() { task(); CalibrationValues.MaxValue = Illuminance; }
    void startCalibrationMinValue() { task(); CalibrationValues.MinValue = Illuminance; }

    void task() { }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
