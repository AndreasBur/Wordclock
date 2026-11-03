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
 *      \brief      Simulator stub of the BH1750 ambient light sensor driver
 *
 *      \details    Mirrors the public API of the hardware BH1750 driver (mode,
 *                  illuminance, calibration) so the shared firmware core builds
 *                  against the simulator. The I2C access paths are stubbed out;
 *                  no physical sensor is present in the wxWidgets simulator.
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
#define BH1750_TASK_CYCLE                               100u

/* BH1750 parameter */
/* The sensor's own range, not what the brightness automatic spreads between - see the two
   calibration defaults below, which the hardware headers carry with the same reasoning. */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u

/* What an uncalibrated clock spreads its brightness between: a dim room and a bright day at
   a window. Deliberately not the sensor's range, where every indoor reading would sit at the
   very bottom and hold the display at its floor. */
#define BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE         1000u
#define BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE         1u

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
        kModeNone,
        kModeContinuousLowResMode = BH1750_CMD_CONTINUOUS_LOW_RES_MODE,
        kModeContinuousHighResMode = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE,
        kModeContinuousHighResMode2 = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2,
        kModeOneTimeLowResMode = BH1750_CMD_ONE_TIME_LOW_RES_MODE,
        kModeOneTimeHighResMode = BH1750_CMD_ONE_TIME_HIGH_RES_MODE,
        kModeOneTimeHighResMode2 = BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2
    };

    struct CalibrationValuesType {
        IlluminanceType max_value;
        IlluminanceType min_value;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte kTaskCycle{BH1750_TASK_CYCLE};
    ModeType mode_{kModeNone};
    IlluminanceType illuminance_{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType calibration_values_{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE};

    // functions
    StdReturnType ReadIlluminance();
    void SendModeForOneTimeMode();
    StdReturnType SendCommand(byte);

    StdReturnType SendMode() { return SendCommand(mode_); }
    /* Not static, though nothing in it reads a member: it stands in for the backends the
       clock really runs on, and there the same call reaches a peripheral through instance
       state. A stand-in that answered the check would have a different API from the thing it
       stands in for, which is the one property it exists to keep. */
    // NOLINTBEGIN(readability-convert-member-functions-to-static)
    IlluminanceType ConvertRawToLux(IlluminanceType illuminance_raw) const { return illuminance_raw / BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER; }
    // NOLINTEND(readability-convert-member-functions-to-static)
    static constexpr IlluminanceType CombineRawValueParts(byte high_byte, byte low_byte) { return static_cast<uint16_t>(high_byte) << 8u | low_byte; }
    static constexpr bool IsMtRegValueInRange(byte mt_reg_value) { return ((mt_reg_value <= BH1750_REG_MT_MAX_VALUE) && (mt_reg_value >= BH1750_REG_MT_MIN_VALUE)); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N   S
******************************************************************************************************************************************************/
  public:
    constexpr BH1750() { }
    ~BH1750() { }

    // get methods
    static constexpr byte GetTaskCycle() { return kTaskCycle; }
    ModeType GetMode() const { return mode_; }
    IlluminanceType GetIlluminance() const { return illuminance_; }
    CalibrationValuesType GetCalibrationValues() const { return calibration_values_; }
    IlluminanceType GetCalibrationValuesMaxValue() const { return calibration_values_.max_value; }
    IlluminanceType GetCalibrationValuesMinValue() const { return calibration_values_.min_value; }

    // set methods
    StdReturnType SetMode(ModeType);
    void SetCalibrationValuesMaxValue(IlluminanceType max_value) { calibration_values_.max_value = max_value; }
    void SetCalibrationValuesMinValue(IlluminanceType min_value) { calibration_values_.min_value = min_value; }
    void SetCalibrationValues(CalibrationValuesType calibration_values) { calibration_values_ = calibration_values; }

    // methods
    StdReturnType Init(ModeType);
    StdReturnType ChangeMeasurementTime(byte);
    /* Always succeeds, unlike on hardware: what the window dialled in is available from
       the first call, so there is no not-set-up state for the calibration to refuse. */
    StdReturnType StartCalibrationMaxValue() {
        Task();
        calibration_values_.max_value = illuminance_;
        return kEOk;
    }
    StdReturnType StartCalibrationMinValue() {
        Task();
        calibration_values_.min_value = illuminance_;
        return kEOk;
    }

    /* On hardware this reads the sensor. Here it takes what the window dialled in, so
       the brightness automatic can be exercised without a sensor. The value is static
       because the firmware keeps its BH1750 inside the Illuminance singleton, which
       leaves the window no instance to talk to. */
    void Task() { illuminance_ = simulated_illuminance; }

    static void SetSimulatedIlluminance(IlluminanceType value) { simulated_illuminance = value; }
    static IlluminanceType GetSimulatedIlluminance() { return simulated_illuminance; }

  private:
    /* a bright room by default, so switching the automatic on does not darken the
       display before anything was dialled in - the calibrated maximum rather than the
       sensor's, which is the top of the slider's range in Settings */
    static inline IlluminanceType simulated_illuminance{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE};
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
