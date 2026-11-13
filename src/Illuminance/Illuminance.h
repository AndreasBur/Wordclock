// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The ambient light the display dims itself by.
 *
 * A thin owner around the platform's BH1750, so the core reaches the sensor without knowing which
 * one it holds - the hardware driver or the simulator's stub.
 *
 * There is deliberately no Init() here to match the other modules': the sensor is private and the
 * scheduler only ever calls Task(), so the driver sets itself up from its first one. See the
 * platform's BH1750.h.
 */

#ifndef WORDCLOCK_SRC_ILLUMINANCE_ILLUMINANCE_H_
#define WORDCLOCK_SRC_ILLUMINANCE_ILLUMINANCE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "BH1750.h"

/*!
 * \brief The ambient light: the last reading of the BH1750, and the calibration the display's
 *        brightness is spread over.
 */
class Illuminance
{
  public:
    /*! \brief An illuminance in lux. */
    using IlluminanceType = BH1750::IlluminanceType;

    /*! \brief The bounds of the calibration, `max_value` and `min_value`, in lux. */
    using CalibrationValuesType = BH1750::CalibrationValuesType;

    /*! \brief A measurement mode of the BH1750. */
    using ModeType = BH1750::ModeType;
  
  private:
    constexpr Illuminance() { }
    ~Illuminance() = default;
    
    BH1750 sensor_;
  
  public:
    /*! \brief Returns the one illuminance. */
    static Illuminance& GetInstance() {
        static Illuminance singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the scheduler ticks between two calls of Task(). */
    byte GetTaskCycle() const { return sensor_.GetTaskCycle(); }

    /*! \brief Returns the illuminance last read, in lux. */
    IlluminanceType GetIlluminance() const { return sensor_.GetIlluminance(); }

    /*! \brief Returns both bounds of the calibration. */
    CalibrationValuesType GetCalibrationValues() const { return sensor_.GetCalibrationValues(); }

    /*! \brief Returns the upper bound of the calibration, in lux. */
    IlluminanceType GetCalibrationValuesMaxValue() const { return sensor_.GetCalibrationValues().max_value; }

    /*! \brief Returns the lower bound of the calibration, in lux. */
    IlluminanceType GetCalibrationValuesMinValue() const { return sensor_.GetCalibrationValues().min_value; }

    /*!
     * \brief Sets the upper bound of the calibration.
     *
     * \param[in] max_value The upper bound, in lux.
     */
    void SetCalibrationValuesMaxValue(IlluminanceType max_value) { sensor_.SetCalibrationValuesMaxValue(max_value); }

    /*!
     * \brief Sets the lower bound of the calibration.
     *
     * \param[in] min_value The lower bound, in lux.
     */
    void SetCalibrationValuesMinValue(IlluminanceType min_value) { sensor_.SetCalibrationValuesMinValue(min_value); }

    /*!
     * \brief Sets both bounds of the calibration.
     *
     * \param[in] calibration_values The upper and the lower bound, in lux.
     */
    void SetCalibrationValues(CalibrationValuesType calibration_values) { sensor_.SetCalibrationValues(calibration_values); }

    /*!
     * \brief Puts the calibration back to the span an uncalibrated clock starts in.
     *
     * The same two bounds the driver initialises from, BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE
     * and BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE, so a reset and a fresh chip agree. Not the
     * sensor's full range: see the defaults in the platform's BH1750.h for why that one leaves
     * the automatic sitting at its floor in every room.
     */
    void ResetToDefaults() {
        SetCalibrationValuesMaxValue(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE);
        SetCalibrationValuesMinValue(BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE);
    }

    /*!
     * \brief Takes the illuminance the sensor reads now as the upper bound of the calibration.
     *
     * It runs the sensor's task once and stores the illuminance the sensor then holds.
     *
     * \return kEOk, or kENotOk while the sensor is not set up, so the answer to the procedure is
     *         the sensor's rather than an unconditional success; the calibration is then left as it
     *         was. The simulator's sensor never refuses.
     */
    StdReturnType StartCalibrationMaxValue() { return sensor_.StartCalibrationMaxValue(); }

    /*!
     * \brief Takes the illuminance the sensor reads now as the lower bound of the calibration.
     *
     * It runs the sensor's task once and stores the illuminance the sensor then holds.
     *
     * \return kEOk, or kENotOk while the sensor is not set up, so the answer to the procedure is
     *         the sensor's rather than an unconditional success; the calibration is then left as it
     *         was. The simulator's sensor never refuses.
     */
    StdReturnType StartCalibrationMinValue() { return sensor_.StartCalibrationMinValue(); }

    /*!
     * \brief Lets the BH1750 driver of the platform take its next step.
     *
     * On the hardware the driver sets the sensor up on its first calls and reads it from then on.
     */
    void Task() { sensor_.Task(); }
};

#endif  // WORDCLOCK_SRC_ILLUMINANCE_ILLUMINANCE_H_
