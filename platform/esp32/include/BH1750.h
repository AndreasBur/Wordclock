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
 *      \brief      BH1750 ambient light sensor driver over I2C
 *
 *      \details    Mirrors the public API of the simulator's stub, so the firmware's
 *                  Illuminance module holds either one without knowing which. The
 *                  register handling follows the xmega driver, which was written
 *                  against the same datasheet.
 *
 *                  The sensor is set up from the first task() rather than from an init()
 *                  call, because the firmware core has no place to make that call from:
 *                  Illuminance keeps its BH1750 private and its own init() is declared
 *                  but never defined. Setting up lazily also retries, which is what a
 *                  sensor that is not answering yet after power-on needs.
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
/* The datasheet's two addresses, selected by the ADDR pin: low is 0x23, high is 0x5C.
   Note that the simulator's stub and the xmega driver both carry 0x76, which is neither -
   harmless there because the stub never opens a bus, but a sensor addressed that way
   stays silent. */
#define BH1750_I2C_ADDR                                 0x23u
#define BH1750_I2C_ADDR_ALTERNATIVE                     0x5Cu

/* The core's board-specific defaults, so a devkit needs no change here. */
#define BH1750_I2C_PIN_SDA                              SDA
#define BH1750_I2C_PIN_SCL                              SCL
#define BH1750_I2C_FREQUENCY_HZ                         100000u

/* In scheduler ticks, so 100 * 10 ms = 1 s between readings. Has to stay well above the
   120 ms a high-resolution conversion takes, or task() reads a measurement that is not
   finished yet. */
#define BH1750_TASK_CYCLE                               100u

#define BH1750_DEFAULT_MODE                             MODE_CONTINUOUS_HIGH_RES_MODE

/* BH1750 parameter */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u
#define BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES    2u
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

/* The measurement time is split over two commands, three high bits and five low ones. */
#define BH1750_MT_REG_VALUE_HIGH_BITS_GM                0b111u
#define BH1750_MT_REG_VALUE_HIGH_BITS_GP                5u
#define BH1750_MT_REG_VALUE_LOW_BITS_GM                 0b11111u
#define BH1750_MT_REG_VALUE_LOW_BITS_GP                 0u

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
    static constexpr byte TaskCycle{BH1750_TASK_CYCLE};
    ModeType Mode{MODE_NONE};
    IlluminanceType Illuminance{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType CalibrationValues{BH1750_ILLUMINANCE_MAX_LX_VALUE, BH1750_ILLUMINANCE_MIN_LX_VALUE};
    bool BusStarted{false};

    // functions
    StdReturnType startBus();
    StdReturnType readIlluminance();
    void sendModeForOneTimeMode();
    StdReturnType sendCommand(byte);

    StdReturnType sendMode() { return sendCommand(Mode); }
    IlluminanceType convertRawToLux(IlluminanceType IlluminanceRaw) const { return IlluminanceRaw / BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER; }
    IlluminanceType combineRawValueParts(byte HighByte, byte LowByte) const { return static_cast<uint16_t>(HighByte) << 8u | LowByte; }
    bool isMTRegValueInRange(byte MTRegValue) const { return ((MTRegValue <= BH1750_REG_MT_MAX_VALUE) && (MTRegValue >= BH1750_REG_MT_MIN_VALUE)); }
    bool isOneTimeMode() const {
        return (Mode == MODE_ONE_TIME_HIGH_RES_MODE) || (Mode == MODE_ONE_TIME_HIGH_RES_MODE_2) || (Mode == MODE_ONE_TIME_LOW_RES_MODE);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N   S
******************************************************************************************************************************************************/
  public:
    constexpr BH1750() { }
    ~BH1750() { }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }
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
    void startCalibrationMaxValue() {
        task();
        CalibrationValues.MaxValue = Illuminance;
    }
    void startCalibrationMinValue() {
        task();
        CalibrationValues.MinValue = Illuminance;
    }

    void task();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
