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
 *                  call, because the core keeps its BH1750 private and never calls one.
 *                  Setting up lazily also retries, which is what a sensor that is not
 *                  answering yet after power-on needs - and what a sensor that stops
 *                  answering later needs, see ReadFailuresBeforeReinit.
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
    /* Readings in a row that may fail before the driver sets the sensor up again, so five
       seconds at TaskCycle. Long enough that one disturbed transfer keeps the last value,
       which is what readIlluminance() is written for, and short enough that a sensor which
       came back is picked up while the room still looks the same. */
    static constexpr byte ReadFailuresBeforeReinit{5u};
    /* The count means half a lux in the two mode 2 variants rather than a whole one, so a
       reading in them has to be divided by twice as much. */
    static constexpr float HighResMode2Divider{2.0f};

    ModeType Mode{MODE_NONE};
    IlluminanceType Illuminance{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType CalibrationValues{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE};
    byte MeasurementTime{BH1750_REG_MT_DEFAULT_VALUE};
    byte ReadFailures{0u};
    bool BusStarted{false};

    // functions
    StdReturnType startBus();
    StdReturnType readIlluminance();
    void sendModeForOneTimeMode();
    StdReturnType sendCommand(byte);

    StdReturnType sendMode() { return sendCommand(Mode); }
    /* Both halves of what task() checks before it reads: a bus that was opened, and a mode
       the sensor acknowledged. Clearing either one puts the set-up back on the next run. */
    bool isReady() const { return BusStarted && (Mode != MODE_NONE); }

    /* The datasheet's conversion, which is not the constant it looks like: the divider
       moves with the mode and with the measurement time. A fixed 1.2 is right only at the
       default of both - which is what the sensor starts in, and why this went unnoticed. */
    float toLuxDivider() const { return BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER * toModeDivider() * toMeasurementTimeFactor(); }
    float toModeDivider() const { return isHighResMode2() ? HighResMode2Divider : 1.0f; }
    /* A longer integration counts more for the same light, so the register's distance from
       its reset value scales the reading back out again. */
    float toMeasurementTimeFactor() const { return static_cast<float>(MeasurementTime) / BH1750_REG_MT_DEFAULT_VALUE; }
    bool isHighResMode2() const { return (Mode == MODE_CONTINUOUS_HIGH_RES_MODE_2) || (Mode == MODE_ONE_TIME_HIGH_RES_MODE_2); }

    /* Clamped rather than cast straight: the shortest measurement time divides by about
       0.54, which takes a full-scale count past what IlluminanceType holds - and a float
       that does not fit the integer it is cast to is undefined behaviour, not a wrap. */
    IlluminanceType convertRawToLux(IlluminanceType IlluminanceRaw) const {
        const float Lux = static_cast<float>(IlluminanceRaw) / toLuxDivider();
        if(Lux >= static_cast<float>(BH1750_ILLUMINANCE_MAX_LX_VALUE)) { return BH1750_ILLUMINANCE_MAX_LX_VALUE; }
        return static_cast<IlluminanceType>(Lux);
    }
    IlluminanceType combineRawValueParts(byte HighByte, byte LowByte) const { return static_cast<uint16_t>(HighByte) << 8u | LowByte; }
    bool isMTRegValueInRange(byte MTRegValue) const { return ((MTRegValue <= BH1750_REG_MT_MAX_VALUE) && (MTRegValue >= BH1750_REG_MT_MIN_VALUE)); }
    bool isOneTimeMode() const {
        return (Mode == MODE_ONE_TIME_HIGH_RES_MODE) || (Mode == MODE_ONE_TIME_HIGH_RES_MODE_2) || (Mode == MODE_ONE_TIME_LOW_RES_MODE);
    }

    /* One dropped reading keeps the last value on purpose - see readIlluminance(). A sensor
       that stops answering altogether is the other case: that value would otherwise stand
       for good and the automatic would follow a room it can no longer see. Enough failures
       therefore drop the mode, which is what puts task() back to setting the sensor up. */
    void countReadFailure() {
        if(++ReadFailures < ReadFailuresBeforeReinit) { return; }
        ReadFailures = 0u;
        Mode = MODE_NONE;
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
    /* Refused while the sensor is not set up, rather than storing whatever Illuminance
       happens to hold: a clock calibrated in that state would keep a bound the sensor never
       measured, Persistence would save it, and a stored zero maximum switches the automatic
       off for good. The caller reports the refusal, so it is visible instead of silent. */
    StdReturnType startCalibrationMaxValue() {
        if(!isReady()) { return E_NOT_OK; }
        task();
        CalibrationValues.MaxValue = Illuminance;
        return E_OK;
    }
    StdReturnType startCalibrationMinValue() {
        if(!isReady()) { return E_NOT_OK; }
        task();
        CalibrationValues.MinValue = Illuminance;
        return E_OK;
    }

    void task();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
