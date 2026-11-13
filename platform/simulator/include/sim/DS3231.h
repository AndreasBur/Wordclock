// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator stand-in for the DS3231's temperature.
 *
 * Mirrors the public API of the hardware driver, and takes its reading from the window instead of
 * from a bus - the same arrangement as the light sensor's, and for the same reason: the temperature
 * overlay is developed here, where there is no chip to warm up.
 *
 * It also stands in for a chip that is not there at all: the firmware keeps the overlay away until
 * a reading has arrived, and that is a state the simulator has to be able to produce. Hence the
 * reading starts invalid rather than at a comfortable room temperature.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_DS3231_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_DS3231_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The bus address of the chip; not used, as the simulator opens no bus. */
#define DS3231_I2C_ADDR                                 0x68u
/*! \brief The scheduler ticks between two calls of DS3231::Task(). */
#define DS3231_TASK_CYCLE                               250u

/*!
 * \brief The DS3231 clock chip, as far as the core reaches it: the temperature it measures, in
 *        tenths of a degree Celsius.
 *
 * The core owns one, inside Temperature, and reaches it through Task() and GetTemperature(). There
 * is no reading until the chip has answered once, which a clock built without the chip never
 * does.
 *
 * The simulator has no chip. Task() takes the temperature the settings window dialled in, through
 * SetSimulatedTemperature(), and takes no reading while the window says no chip is connected,
 * through ClearSimulatedTemperature(). Unlike a chip, it can lose its reading again.
 */
class DS3231
{
  public:
    /*! \brief A temperature in tenths of a degree Celsius, signed. */
    using TemperatureType = int16_t;

    /*! \brief The tenths in a degree, the unit of a TemperatureType. */
    static constexpr TemperatureType kTenthsPerDegree{10};

  private:
    static constexpr byte kTaskCycle{DS3231_TASK_CYCLE};

    TemperatureType temperature_{0};
    bool temperature_valid_{false};

  public:
    /*! \brief Makes a chip that has no reading yet. */
    constexpr DS3231() { }

    /*! \brief Destroys the chip. */
    ~DS3231() = default;

    /*! \brief Returns the scheduler ticks between two calls of Task(), DS3231_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*!
     * \brief Gives the temperature last read.
     *
     * In the simulator, the reading is the one the last Task() took from the settings window, and
     * there is none if the window said no chip is connected at that Task().
     *
     * \param[out] temperature The temperature, in tenths of a degree Celsius.
     * \return kEOk, or kENotOk while there is no reading; `temperature` is then left as it was.
     */
    StdReturnType GetTemperature(TemperatureType& temperature) const {
        if(!temperature_valid_) { return kENotOk; }

        temperature = temperature_;
        return kEOk;
    }

    /*!
     * \brief Brings the temperature up to date.
     *
     * On hardware this reads the chip. Here it takes what the window dialled in. The values are
     * static because the firmware keeps its DS3231 inside the Temperature singleton, which
     * leaves the window no instance to talk to.
     *
     * It takes whether there is a reading as well, so a reading goes away again on the Task()
     * after ClearSimulatedTemperature().
     */
    void Task() {
        temperature_ = simulated_temperature;
        temperature_valid_ = simulated_temperature_valid;
    }

    /*!
     * \brief Sets the temperature the next Task() takes, and makes it a reading, as the settings
     *        window does while its chip is connected.
     *
     * Shared by every DS3231, as the value is static.
     *
     * \param[in] value The temperature, in tenths of a degree Celsius.
     */
    static void SetSimulatedTemperature(TemperatureType value) {
        simulated_temperature = value;
        simulated_temperature_valid = true;
    }

    /*!
     * \brief Returns the temperature the next Task() takes, in tenths of a degree Celsius; 0 until
     *        SetSimulatedTemperature() is called.
     */
    static TemperatureType GetSimulatedTemperature() { return simulated_temperature; }

    /*!
     * \brief Puts the chip back to never having answered, which is the state the overlay has to
     *        stay away in.
     *
     * The next Task() takes no reading. The temperature set last is kept.
     */
    static void ClearSimulatedTemperature() { simulated_temperature_valid = false; }

    /*!
     * \brief Returns whether the next Task() takes a reading: false at first and after
     *        ClearSimulatedTemperature(), true after SetSimulatedTemperature().
     */
    static bool IsSimulatedTemperatureValid() { return simulated_temperature_valid; }

  private:
    static inline TemperatureType simulated_temperature{0};
    static inline bool simulated_temperature_valid{false};
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_DS3231_H_
