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
 *      \brief      The ambient-light sensor the display dims itself by
 *
 *      \details    Sets itself up from its first Task() rather than from an Init() call: the
 *                  core keeps its sensor private and never calls one, so a driver that waited
 *                  to be initialised would never measure anything. The attempt is repeated on
 *                  every task, which also picks up a sensor that was not answering earlier -
 *                  and, through kReadFailuresBeforeReinit, one that stops answering later.
 *
 *                  The bus is shared with the clock chip, and neither driver owns it - see
 *                  Twi.h.
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
/* The datasheet's two addresses, selected by the ADDR pin: low is 0x23, high is 0x5C. */
#define BH1750_I2C_ADDR                                 0x23u
#define BH1750_I2C_ADDR_ALTERNATIVE                     0x5Cu

/* In scheduler ticks, so 100 * 10 ms = 1 s between readings. Has to stay well above the
   120 ms a high-resolution conversion takes, or Task() reads a measurement that is not
   finished yet. */
#define BH1750_TASK_CYCLE                               100u
#define BH1750_DEFAULT_MODE                             ModeType::kContinuousHighResMode

/* BH1750 parameter */
/* The sensor's own range, which is what a reading is clamped to - not what the brightness
   automatic spreads between, see the two calibration defaults below. */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u
#define BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES    2u

/* What an uncalibrated clock spreads its brightness between, and deliberately not the
   sensor's range: 65535 lx is direct sunlight, so a living room at 100 lx would sit at the
   very bottom of it and the automatic would hold the display at its floor everywhere
   indoors - which reads as a broken sensor rather than as a default. These two are a dim
   room and a bright day at a window, the span a clock on a wall actually moves in. Both
   are starting points only; the two calibration procedures replace them with what the room
   in front of this clock does. */
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

/* The measurement time is split over two commands, three high bits and five low ones. */
#define BH1750_MT_REG_VALUE_HIGH_BITS_GM                0b111u
#define BH1750_MT_REG_VALUE_HIGH_BITS_GP                5u
#define BH1750_MT_REG_VALUE_LOW_BITS_GM                 0b11111u
#define BH1750_MT_REG_VALUE_LOW_BITS_GP                 0u

#define BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER            1.2f

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

    enum class ModeType {
        kNone,
        kContinuousLowResMode = BH1750_CMD_CONTINUOUS_LOW_RES_MODE,
        kContinuousHighResMode = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE,
        kContinuousHighResMode2 = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2,
        kOneTimeLowResMode = BH1750_CMD_ONE_TIME_LOW_RES_MODE,
        kOneTimeHighResMode = BH1750_CMD_ONE_TIME_HIGH_RES_MODE,
        kOneTimeHighResMode2 = BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2
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
    /* Readings in a row that may fail before the driver sets the sensor up again, so five
       seconds at task_cycle_. Long enough that one disturbed transfer keeps the last value,
       which is what ReadIlluminance() is written for, and short enough that a sensor which
       came back is picked up while the room still looks the same. */
    static constexpr byte kReadFailuresBeforeReinit{5u};
    /* The count means half a lux in the two mode 2 variants rather than a whole one, so a
       reading in them has to be divided by twice as much. */
    static constexpr float kHighResMode2Divider{2.0f};

    ModeType mode_{ModeType::kNone};
    IlluminanceType illuminance_{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType calibration_values_{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE};
    byte measurement_time_{BH1750_REG_MT_DEFAULT_VALUE};
    byte read_failures_{0u};
    bool bus_started_{false};

    // functions
    StdReturnType StartBus();
    StdReturnType ReadIlluminance();
    void SendModeForOneTimeMode();
    static StdReturnType SendCommand(byte);
    StdReturnType SendMode() { return SendCommand(ToUnderlying(mode_)); }
    /* Both halves of what Task() checks before it reads: a bus that was opened, and a mode
       the sensor acknowledged. Clearing either one puts the set-up back on the next run. */
    bool IsReady() const { return bus_started_ && (mode_ != ModeType::kNone); }

    /* The datasheet's conversion, which is not the constant it looks like: the divider
       moves with the mode and with the measurement time. A fixed 1.2 is right only at the
       default of both - which is what the sensor starts in, and why this went unnoticed. */
    float ToLuxDivider() const { return BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER * ToModeDivider() * ToMeasurementTimeFactor(); }
    float ToModeDivider() const { return IsHighResMode2() ? kHighResMode2Divider : 1.0f; }
    /* A longer integration counts more for the same light, so the register's distance from
       its reset value scales the reading back out again. */
    float ToMeasurementTimeFactor() const { return static_cast<float>(measurement_time_) / BH1750_REG_MT_DEFAULT_VALUE; }
    bool IsHighResMode2() const { return (mode_ == ModeType::kContinuousHighResMode2) || (mode_ == ModeType::kOneTimeHighResMode2); }

    /* Clamped rather than cast straight: the shortest measurement time divides by about
       0.54, which takes a full-scale count past what IlluminanceType holds - and a float
       that does not fit the integer it is cast to is undefined behaviour, not a wrap. */
    IlluminanceType ConvertRawToLux(IlluminanceType illuminance_raw) const {
        const float lux = static_cast<float>(illuminance_raw) / ToLuxDivider();
        if(lux >= static_cast<float>(BH1750_ILLUMINANCE_MAX_LX_VALUE)) { return BH1750_ILLUMINANCE_MAX_LX_VALUE; }
        return static_cast<IlluminanceType>(lux);
    }
    static constexpr IlluminanceType CombineRawValueParts(byte high_byte, byte low_byte) { return static_cast<uint16_t>(high_byte) << 8u | low_byte; }
    static constexpr bool IsMtRegValueInRange(byte mt_reg_value) { return ((mt_reg_value <= BH1750_REG_MT_MAX_VALUE) && (mt_reg_value >= BH1750_REG_MT_MIN_VALUE)); }
    bool IsOneTimeMode() const {
        return (mode_ == ModeType::kOneTimeHighResMode) || (mode_ == ModeType::kOneTimeHighResMode2) || (mode_ == ModeType::kOneTimeLowResMode);
    }

    /* One dropped reading keeps the last value on purpose - see ReadIlluminance(). A sensor
       that stops answering altogether is the other case: that value would otherwise stand
       for good and the automatic would follow a room it can no longer see. Enough failures
       therefore drop the mode, which is what puts Task() back to setting the sensor up. */
    void CountReadFailure() {
        if(++read_failures_ < kReadFailuresBeforeReinit) { return; }
        read_failures_ = 0u;
        mode_ = ModeType::kNone;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
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
    /* Refused while the sensor is not set up, rather than storing whatever Illuminance
       happens to hold: a clock calibrated in that state would keep a bound the sensor never
       measured, Persistence would save it, and a stored zero maximum switches the automatic
       off for good. The caller reports the refusal, so it is visible instead of silent. */
    StdReturnType StartCalibrationMaxValue() {
        if(!IsReady()) { return kENotOk; }
        Task();
        calibration_values_.max_value = illuminance_;
        return kEOk;
    }
    StdReturnType StartCalibrationMinValue() {
        if(!IsReady()) { return kENotOk; }
        Task();
        calibration_values_.min_value = illuminance_;
        return kEOk;
    }
    void Task();
};

#endif // _BH1750_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
