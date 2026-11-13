// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The ambient-light sensor the display dims itself by.
 *
 * Sets itself up from its first Task() rather than from an Init() call: the core keeps its sensor
 * private and never calls one, so a driver that waited to be initialised would never measure
 * anything. The attempt is repeated on every task, which also picks up a sensor that was not
 * answering earlier - and, through kReadFailuresBeforeReinit, one that stops answering later.
 *
 * The bus is shared with the clock chip, and neither driver owns it - see Twi.h.
 */

#ifndef WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_BH1750_H_
#define WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_BH1750_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* The datasheet's two addresses, selected by the ADDR pin: low is 0x23, high is 0x5C. */
/*! \brief The bus address of the sensor with its ADDR pin low, the one the driver talks to. */
#define BH1750_I2C_ADDR                                 0x23u
/*! \brief The bus address of the sensor with its ADDR pin high; the driver does not use it. */
#define BH1750_I2C_ADDR_ALTERNATIVE                     0x5Cu

/*!
 * \brief The scheduler ticks between two calls of BH1750::Task().
 *
 * In scheduler ticks, so 100 * 10 ms = 1 s between readings. Has to stay well above the 120 ms a
 * high-resolution conversion takes, or Task() reads a measurement that is not finished yet.
 */
#define BH1750_TASK_CYCLE                               100u
/*! \brief The measurement mode BH1750::Task() sets the sensor up in. */
#define BH1750_DEFAULT_MODE                             ModeType::kContinuousHighResMode

/*!
 * \brief The largest illuminance the sensor reads, in lux.
 *
 * The sensor's own top, which a reading is clamped to - not what the brightness automatic spreads
 * between, see the two calibration defaults below.
 */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
/*!
 * \brief The illuminance BH1750::GetIlluminance() returns before the first reading, in lux.
 *
 * It is not the smallest one the sensor reads here: nothing clamps a reading at this end, so a
 * count below the divider converts to 0 lux.
 */
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u
/*! \brief The number of bytes of a reading. */
#define BH1750_ILLUMINANCE_RAW_VALUE_NUMBER_OF_BYTES    2u

/*!
 * \brief The upper bound of the calibration an uncalibrated clock starts with, in lux.
 *
 * What an uncalibrated clock spreads its brightness between, and deliberately not the sensor's
 * range: 65535 lx is direct sunlight, so a living room at 100 lx would sit at the very bottom of it
 * and the automatic would hold the display at its floor everywhere indoors - which reads as a
 * broken sensor rather than as a default. These two are a dim room and a bright day at a window,
 * the span a clock on a wall actually moves in. Both are starting points only; the two calibration
 * procedures replace them with what the room in front of this clock does.
 */
#define BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE         1000u
/*!
 * \brief The lower bound of the calibration an uncalibrated clock starts with, in lux; see
 *        BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE.
 */
#define BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE         1u

/*! \brief The smallest value BH1750::ChangeMeasurementTime() takes. */
#define BH1750_REG_MT_MIN_VALUE                         31u
/*! \brief The largest value BH1750::ChangeMeasurementTime() takes. */
#define BH1750_REG_MT_MAX_VALUE                         254u
/*! \brief The value the measurement time register holds before it is changed. */
#define BH1750_REG_MT_DEFAULT_VALUE                     69u

/*! \brief The command that powers the sensor down. */
#define BH1750_CMD_POWER_DOWN                           0b00000000u
/*! \brief The command that powers the sensor on. */
#define BH1750_CMD_POWER_ON                             0b00000001u
/*! \brief The reset command of the sensor. */
#define BH1750_CMD_RESET                                0b00000111u
/*! \brief The command of BH1750::ModeType::kContinuousHighResMode. */
#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE             0b00010000u
/*! \brief The command of BH1750::ModeType::kContinuousHighResMode2. */
#define BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2           0b00010001u
/*! \brief The command of BH1750::ModeType::kContinuousLowResMode. */
#define BH1750_CMD_CONTINUOUS_LOW_RES_MODE              0b00010011u
/*! \brief The command of BH1750::ModeType::kOneTimeHighResMode. */
#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE               0b00100000u
/*! \brief The command of BH1750::ModeType::kOneTimeHighResMode2. */
#define BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2             0b00100001u
/*! \brief The command of BH1750::ModeType::kOneTimeLowResMode. */
#define BH1750_CMD_ONE_TIME_LOW_RES_MODE                0b00100011u
/*! \brief The command that writes the high bits of the measurement time, ORed into it. */
#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_HIGH_BITS    0b01000000u
/*! \brief The command that writes the low bits of the measurement time, ORed into it. */
#define BH1750_CMD_CHANGE_MEASUREMENT_TIME_LOW_BITS     0b01100000u

/* The measurement time is split over two commands, three high bits and five low ones. */
/*! \brief The mask of the three high bits of a measurement time, written down unshifted. */
#define BH1750_MT_REG_VALUE_HIGH_BITS_GM                0b111u
/*! \brief The position of the lowest of the three high bits of a measurement time. */
#define BH1750_MT_REG_VALUE_HIGH_BITS_GP                5u
/*! \brief The mask of the five low bits of a measurement time, written down unshifted. */
#define BH1750_MT_REG_VALUE_LOW_BITS_GM                 0b11111u
/*! \brief The position of the lowest of the five low bits of a measurement time. */
#define BH1750_MT_REG_VALUE_LOW_BITS_GP                 0u

/*!
 * \brief What a raw count of the sensor is divided by to give lux.
 *
 * It holds at the default mode and measurement time; the driver scales it by both.
 */
#define BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER            1.2f

/*!
 * \brief The ambient-light sensor: the illuminance it last read, and the bounds of the calibration
 *        the display's brightness is spread over.
 *
 * The core owns one, inside Illuminance, and reaches it through Task(), the getters and the
 * calibration. It calls no Init(): the sensor has to be ready from its Task() alone.
 *
 * On the AVR the sensor is reached over TWI1 through Twi, which it shares with the DS3231. Task()
 * sets the sensor up while it is not, and sets it up again after kReadFailuresBeforeReinit failed
 * readings in a row.
 */
class BH1750
{
  public:
    /*! \brief An illuminance in lux. */
    using IlluminanceType = uint16_t;

    /*! \brief A measurement mode of the sensor; its value is the command that selects it. */
    enum class ModeType : byte {
        kNone, /*!< No mode: the sensor is not set up. */
        /*! \brief Measures again and again, at low resolution. */
        kContinuousLowResMode = BH1750_CMD_CONTINUOUS_LOW_RES_MODE,
        /*! \brief Measures again and again, at high resolution. */
        kContinuousHighResMode = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE,
        /*! \brief Measures again and again, at high resolution in half a lux. */
        kContinuousHighResMode2 = BH1750_CMD_CONTINUOUS_HIGH_RES_MODE_2,
        /*! \brief Measures once, at low resolution, and powers down. */
        kOneTimeLowResMode = BH1750_CMD_ONE_TIME_LOW_RES_MODE,
        /*! \brief Measures once, at high resolution, and powers down. */
        kOneTimeHighResMode = BH1750_CMD_ONE_TIME_HIGH_RES_MODE,
        /*! \brief Measures once, at high resolution in half a lux, and powers down. */
        kOneTimeHighResMode2 = BH1750_CMD_ONE_TIME_HIGH_RES_MODE_2
    };

    /*! \brief The bounds of the calibration, in lux. */
    struct CalibrationValuesType {
        IlluminanceType max_value; /*!< The upper bound, in lux. */
        IlluminanceType min_value; /*!< The lower bound, in lux. */
    };

  private:
    static constexpr byte kTaskCycle{BH1750_TASK_CYCLE};
    /*
     * Readings in a row that may fail before the driver sets the sensor up again, so five
     * seconds at kTaskCycle. Long enough that one disturbed transfer keeps the last value,
     * which is what ReadIlluminance() is written for, and short enough that a sensor which
     * came back is picked up while the room still looks the same.
     */
    static constexpr byte kReadFailuresBeforeReinit{5u};
    // The count means half a lux in the two mode 2 variants rather than a whole one, so a
    // reading in them has to be divided by twice as much.
    static constexpr float kHighResMode2Divider{2.0f};

    ModeType mode_{ModeType::kNone};
    IlluminanceType illuminance_{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType calibration_values_{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE};
    byte measurement_time_{BH1750_REG_MT_DEFAULT_VALUE};
    byte read_failures_{0u};
    bool bus_started_{false};

    StdReturnType StartBus();
    StdReturnType ReadIlluminance();
    void SendModeForOneTimeMode();
    static StdReturnType SendCommand(byte command);
    StdReturnType SendMode() { return SendCommand(ToUnderlying(mode_)); }
    // Both halves of what Task() checks before it reads: a bus that was opened, and a mode
    // the sensor acknowledged. Clearing either one puts the set-up back on the next run.
    bool IsReady() const { return bus_started_ && (mode_ != ModeType::kNone); }

    /*
     * The datasheet's conversion, which is not the constant it looks like: the divider
     * moves with the mode and with the measurement time. A fixed 1.2 is right only at the
     * default of both - which is what the sensor starts in, and why this went unnoticed.
     */
    float ToLuxDivider() const { return BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER * ToModeDivider() * ToMeasurementTimeFactor(); }
    float ToModeDivider() const { return IsHighResMode2() ? kHighResMode2Divider : 1.0f; }
    // A longer integration counts more for the same light, so the register's distance from
    // its reset value scales the reading back out again.
    float ToMeasurementTimeFactor() const { return static_cast<float>(measurement_time_) / BH1750_REG_MT_DEFAULT_VALUE; }
    bool IsHighResMode2() const { return (mode_ == ModeType::kContinuousHighResMode2) || (mode_ == ModeType::kOneTimeHighResMode2); }

    /*
     * Clamped rather than cast straight: the shortest measurement time divides by about
     * 0.54, which takes a full-scale count past what IlluminanceType holds - and a float
     * that does not fit the integer it is cast to is undefined behaviour, not a wrap.
     */
    IlluminanceType ConvertRawToLux(IlluminanceType illuminance_raw) const {
        const float lux = static_cast<float>(illuminance_raw) / ToLuxDivider();
        if(lux >= static_cast<float>(BH1750_ILLUMINANCE_MAX_LX_VALUE)) { return BH1750_ILLUMINANCE_MAX_LX_VALUE; }
        return static_cast<IlluminanceType>(lux);
    }
    static constexpr IlluminanceType CombineRawValueParts(byte high_byte, byte low_byte) { return static_cast<IlluminanceType>((high_byte << 8u) | low_byte); }
    static constexpr bool IsMtRegValueInRange(byte mt_reg_value) { return ((mt_reg_value <= BH1750_REG_MT_MAX_VALUE) && (mt_reg_value >= BH1750_REG_MT_MIN_VALUE)); }
    bool IsOneTimeMode() const {
        return (mode_ == ModeType::kOneTimeHighResMode) || (mode_ == ModeType::kOneTimeHighResMode2) || (mode_ == ModeType::kOneTimeLowResMode);
    }

    /*
     * One dropped reading keeps the last value on purpose - see ReadIlluminance(). A sensor
     * that stops answering altogether is the other case: that value would otherwise stand
     * for good and the automatic would follow a room it can no longer see. Enough failures
     * therefore drop the mode, which is what puts Task() back to setting the sensor up.
     */
    void CountReadFailure() {
        if(++read_failures_ < kReadFailuresBeforeReinit) { return; }
        read_failures_ = 0u;
        mode_ = ModeType::kNone;
    }

  public:
    /*!
     * \brief Makes a sensor with no mode, the illuminance BH1750_ILLUMINANCE_MIN_LX_VALUE and the
     *        default calibration.
     */
    constexpr BH1750() { }

    /*! \brief Destroys the sensor. */
    ~BH1750() = default;

    /*! \brief Returns the scheduler ticks between two calls of Task(), BH1750_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*!
     * \brief Returns the measurement mode the sensor is in; kNone while it is not set up.
     *
     * It falls back to kNone after kReadFailuresBeforeReinit failed readings in a row too, so that
     * the next Task() sets the sensor up again.
     *
     * \return The measurement mode.
     */
    ModeType GetMode() const { return mode_; }

    /*!
     * \brief Returns the illuminance last read, in lux; BH1750_ILLUMINANCE_MIN_LX_VALUE until the
     *        first reading.
     *
     * A failed reading keeps the last value.
     *
     * \return The illuminance, in lux.
     */
    IlluminanceType GetIlluminance() const { return illuminance_; }

    /*! \brief Returns both bounds of the calibration. */
    CalibrationValuesType GetCalibrationValues() const { return calibration_values_; }

    /*! \brief Returns the upper bound of the calibration, in lux. */
    IlluminanceType GetCalibrationValuesMaxValue() const { return calibration_values_.max_value; }

    /*! \brief Returns the lower bound of the calibration, in lux. */
    IlluminanceType GetCalibrationValuesMinValue() const { return calibration_values_.min_value; }

    /*!
     * \brief Puts the sensor into the measurement mode `mode`.
     *
     * \param[in] mode The mode; kNone is refused.
     * \return kEOk, or kENotOk if `mode` is kNone or the sensor did not take it; the mode is then
     *         left as it was.
     */
    StdReturnType SetMode(ModeType mode);

    /*!
     * \brief Sets the upper bound of the calibration.
     *
     * \param[in] max_value The upper bound, in lux.
     */
    void SetCalibrationValuesMaxValue(IlluminanceType max_value) { calibration_values_.max_value = max_value; }

    /*!
     * \brief Sets the lower bound of the calibration.
     *
     * \param[in] min_value The lower bound, in lux.
     */
    void SetCalibrationValuesMinValue(IlluminanceType min_value) { calibration_values_.min_value = min_value; }

    /*!
     * \brief Sets both bounds of the calibration.
     *
     * \param[in] calibration_values The upper and the lower bound, in lux.
     */
    void SetCalibrationValues(CalibrationValuesType calibration_values) { calibration_values_ = calibration_values; }

    /*!
     * \brief Sets the sensor up: opens the bus, powers the sensor on and puts it into `mode`.
     *
     * Task() calls it with BH1750_DEFAULT_MODE. Opening the bus never fails on the AVR, and with
     * kNone the bus is opened and the sensor powered on before the mode is refused.
     *
     * \param[in] mode The measurement mode; kNone is refused.
     * \return kEOk, or kENotOk if the bus did not open, the sensor did not take a command or `mode`
     *         is kNone; the mode is then left as it was.
     */
    StdReturnType Init(ModeType mode);

    /*!
     * \brief Sets the sensor's measurement time register to `mt_reg_value`.
     *
     * It sends the mode again afterwards, as the sensor takes the new time only with the next
     * measurement command.
     *
     * \param[in] mt_reg_value The value, from BH1750_REG_MT_MIN_VALUE to BH1750_REG_MT_MAX_VALUE.
     * \return kEOk, or kENotOk if `mt_reg_value` is out of that range or the sensor did not take a
     *         command; the readings are then converted with the measurement time they were
     *         converted with before.
     * \note While the sensor is not set up, the mode it sends again is kNone, whose value is the
     *       command that powers the sensor down.
     */
    StdReturnType ChangeMeasurementTime(byte mt_reg_value);

    /*!
     * \brief Takes the illuminance the sensor reads now as the upper bound of the calibration.
     *
     * It runs Task() once and stores the illuminance then held.
     *
     * Refused while the sensor is not set up, rather than storing whatever Illuminance happens to
     * hold: a clock calibrated in that state would keep a bound the sensor never measured,
     * Persistence would save it, and a stored zero maximum switches the automatic off for good.
     * The caller reports the refusal, so it is visible instead of silent.
     *
     * \return kEOk, or kENotOk while the sensor is not set up; the calibration is then left as it
     *         was.
     */
    StdReturnType StartCalibrationMaxValue() {
        if(!IsReady()) { return kENotOk; }
        Task();
        calibration_values_.max_value = illuminance_;
        return kEOk;
    }

    /*!
     * \brief Takes the illuminance the sensor reads now as the lower bound of the calibration.
     *
     * It runs Task() once and stores the illuminance then held. Refused while the sensor is not
     * set up, for the reason StartCalibrationMaxValue() gives.
     *
     * \return kEOk, or kENotOk while the sensor is not set up; the calibration is then left as it
     *         was.
     */
    StdReturnType StartCalibrationMinValue() {
        if(!IsReady()) { return kENotOk; }
        Task();
        calibration_values_.min_value = illuminance_;
        return kEOk;
    }

    /*!
     * \brief Brings the illuminance up to date.
     *
     * While the sensor is not set up, it sets it up with Init() and reads nothing on that run.
     * From then on it reads the sensor. After a reading it starts the next measurement in a
     * one-time mode; after kReadFailuresBeforeReinit failed ones in a row it drops the mode, so
     * that the next run sets the sensor up again.
     *
     * Set up from here rather than from an Init() call, and retried until it works: the core keeps
     * its sensor private and has no place to call Init() from. The retry costs one failing
     * transfer per cycle on a board with no sensor, which the bounded timeouts keep short.
     */
    void Task();
};

#endif  // WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_BH1750_H_
