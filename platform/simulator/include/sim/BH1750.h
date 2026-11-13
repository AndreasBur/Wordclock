// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator stub of the BH1750 ambient light sensor driver.
 *
 * Mirrors the public API of the hardware BH1750 driver (mode, illuminance, calibration) so the
 * shared firmware core builds against the simulator. The I2C access paths are stubbed out; no
 * physical sensor is present in the wxWidgets simulator.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_BH1750_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_BH1750_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The bus address of the sensor; not used, as the simulator opens no bus. */
#define BH1750_I2C_ADDR                                 0x76u
/*! \brief The scheduler ticks between two calls of BH1750::Task(). */
#define BH1750_TASK_CYCLE                               100u

/*!
 * \brief The largest illuminance the sensor reads, in lux.
 *
 * The sensor's own range, not what the brightness automatic spreads between - see the two
 * calibration defaults below, which the hardware headers carry with the same reasoning.
 */
#define BH1750_ILLUMINANCE_MAX_LX_VALUE                 65535u
/*!
 * \brief The smallest illuminance the sensor reads, in lux, and the one BH1750::GetIlluminance()
 *        returns before the first BH1750::Task().
 */
#define BH1750_ILLUMINANCE_MIN_LX_VALUE                 1u

/*!
 * \brief The upper bound of the calibration an uncalibrated clock starts with, in lux.
 *
 * What an uncalibrated clock spreads its brightness between: a dim room and a bright day at a
 * window. Deliberately not the sensor's range, where every indoor reading would sit at the very
 * bottom and hold the display at its floor.
 *
 * It is also the illuminance the slider of the settings window sets at the top of its travel,
 * where it starts.
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

/*! \brief What a raw count of the sensor is divided by to give lux. */
#define BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER            1.2f

/*!
 * \brief The ambient-light sensor: the illuminance it last read, and the bounds of the calibration
 *        the display's brightness is spread over.
 *
 * The core owns one, inside Illuminance, and reaches it through Task(), the getters and the
 * calibration. It calls no Init(): the sensor has to be ready from its Task() alone.
 *
 * The simulator has no sensor. Task() takes the illuminance the settings window dialled in,
 * through SetSimulatedIlluminance(), so the brightness automatic can be exercised without one.
 * Nothing sets a measurement mode: Init(), SetMode() and ChangeMeasurementTime() are declared but
 * not defined.
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
    ModeType mode_{ModeType::kNone};
    IlluminanceType illuminance_{BH1750_ILLUMINANCE_MIN_LX_VALUE};
    CalibrationValuesType calibration_values_{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE};

    StdReturnType ReadIlluminance();
    void SendModeForOneTimeMode();
    StdReturnType SendCommand(byte command);

    StdReturnType SendMode() { return SendCommand(ToUnderlying(mode_)); }
    /*
     * Not static, though nothing in it reads a member: it stands in for the backends the
     * clock really runs on, and there the same call reaches a peripheral through instance
     * state. A stand-in that answered the check would have a different API from the thing it
     * stands in for, which is the one property it exists to keep.
     */
    // NOLINTBEGIN(readability-convert-member-functions-to-static)
    // Divided by 1.2 alone, so the result is smaller than the raw value and fits; the backends
    // divide by as little as 0.54 and clamp for that.
    IlluminanceType ConvertRawToLux(IlluminanceType illuminance_raw) const {
        return static_cast<IlluminanceType>(static_cast<float>(illuminance_raw) / BH1750_ILLUMINANCE_RAW_VALUE_DIVIDER);
    }
    // NOLINTEND(readability-convert-member-functions-to-static)
    static constexpr IlluminanceType CombineRawValueParts(byte high_byte, byte low_byte) { return static_cast<IlluminanceType>((high_byte << 8u) | low_byte); }
    static constexpr bool IsMtRegValueInRange(byte mt_reg_value) { return ((mt_reg_value <= BH1750_REG_MT_MAX_VALUE) && (mt_reg_value >= BH1750_REG_MT_MIN_VALUE)); }

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
     * Nothing in the simulator sets a mode, so it is always kNone.
     *
     * \return The measurement mode.
     */
    ModeType GetMode() const { return mode_; }

    /*!
     * \brief Returns the illuminance last read, in lux; BH1750_ILLUMINANCE_MIN_LX_VALUE until the
     *        first reading.
     *
     * In the simulator, the reading is the one the last Task() took from the settings window.
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
     * \note The simulator declares this function but does not define it, so a call does not link.
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
     * \param[in] mode The measurement mode; kNone is refused.
     * \return kEOk, or kENotOk if the bus did not open, the sensor did not take a command or `mode`
     *         is kNone; the mode is then left as it was.
     * \note The simulator declares this function but does not define it, so a call does not link.
     */
    StdReturnType Init(ModeType mode);

    /*!
     * \brief Sets the sensor's measurement time register to `mt_reg_value`.
     *
     * \param[in] mt_reg_value The value, from BH1750_REG_MT_MIN_VALUE to BH1750_REG_MT_MAX_VALUE.
     * \return kEOk, or kENotOk if `mt_reg_value` is out of that range or the sensor did not take a
     *         command; the readings are then converted with the measurement time they were
     *         converted with before.
     * \note The simulator declares this function but does not define it, so a call does not link.
     */
    StdReturnType ChangeMeasurementTime(byte mt_reg_value);

    /*!
     * \brief Takes the illuminance the sensor reads now as the upper bound of the calibration.
     *
     * It runs Task() once and stores the illuminance then held.
     *
     * Always succeeds, unlike on hardware: what the window dialled in is available from the
     * first call, so there is no not-set-up state for the calibration to refuse.
     *
     * \return kEOk, or kENotOk while the sensor is not set up; the calibration is then left as it
     *         was.
     */
    StdReturnType StartCalibrationMaxValue() {
        Task();
        calibration_values_.max_value = illuminance_;
        return kEOk;
    }

    /*!
     * \brief Takes the illuminance the sensor reads now as the lower bound of the calibration.
     *
     * It runs Task() once and stores the illuminance then held.
     *
     * Always succeeds, unlike on hardware: what the window dialled in is available from the
     * first call, so there is no not-set-up state for the calibration to refuse.
     *
     * \return kEOk, or kENotOk while the sensor is not set up; the calibration is then left as it
     *         was.
     */
    StdReturnType StartCalibrationMinValue() {
        Task();
        calibration_values_.min_value = illuminance_;
        return kEOk;
    }

    /*!
     * \brief Brings the illuminance up to date.
     *
     * On hardware this reads the sensor. Here it takes what the window dialled in, so the
     * brightness automatic can be exercised without a sensor. The value is static because the
     * firmware keeps its BH1750 inside the Illuminance singleton, which leaves the window no
     * instance to talk to.
     */
    void Task() { illuminance_ = simulated_illuminance; }

    /*!
     * \brief Sets the illuminance the next Task() takes, as the settings window's slider does.
     *
     * Shared by every BH1750, as the value is static.
     *
     * \param[in] value The illuminance, in lux.
     */
    static void SetSimulatedIlluminance(IlluminanceType value) { simulated_illuminance = value; }

    /*!
     * \brief Returns the illuminance the next Task() takes, in lux;
     *        BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE until SetSimulatedIlluminance() is called.
     */
    static IlluminanceType GetSimulatedIlluminance() { return simulated_illuminance; }

  private:
    /*
     * a bright room by default, so switching the automatic on does not darken the
     * display before anything was dialled in - the calibrated maximum rather than the
     * sensor's, which is the top of the slider's range in Settings
     */
    static inline IlluminanceType simulated_illuminance{BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE};
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_BH1750_H_
