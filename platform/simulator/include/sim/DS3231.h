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
/**     \file       DS3231.h
 *      \brief      Simulator stand-in for the DS3231's temperature
 *
 *      \details    Mirrors the public API of the hardware driver, and takes its reading
 *                  from the window instead of from a bus - the same arrangement as the
 *                  light sensor's, and for the same reason: the temperature overlay is
 *                  developed here, where there is no chip to warm up.
 *
 *                  It also stands in for a chip that is not there at all: the firmware
 *                  keeps the overlay away until a reading has arrived, and that is a state
 *                  the simulator has to be able to produce. Hence the reading starts
 *                  invalid rather than at a comfortable room temperature.
 *
******************************************************************************************************************************************************/
#ifndef _DS3231_H_
#define _DS3231_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DS3231 configuration parameter */
#define DS3231_I2C_ADDR                                 0x68u
#define DS3231_TASK_CYCLE                               250u

/******************************************************************************************************************************************************
 *  C L A S S   D S 3 2 3 1
******************************************************************************************************************************************************/
class DS3231
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using TemperatureType = int16_t;

    static constexpr TemperatureType TenthsPerDegree{10};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{DS3231_TASK_CYCLE};

    TemperatureType Temperature{0};
    bool TemperatureValid{false};

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DS3231() { }
    ~DS3231() { }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    StdReturnType getTemperature(TemperatureType& sTemperature) const {
        if(!TemperatureValid) { return E_NOT_OK; }

        sTemperature = Temperature;
        return E_OK;
    }

    // methods
    /* On hardware this reads the chip. Here it takes what the window dialled in. The
       values are static because the firmware keeps its DS3231 inside the Temperature
       singleton, which leaves the window no instance to talk to. */
    void task() {
        Temperature = SimulatedTemperature;
        TemperatureValid = SimulatedTemperatureValid;
    }

    static void setSimulatedTemperature(TemperatureType Value) {
        SimulatedTemperature = Value;
        SimulatedTemperatureValid = true;
    }
    static TemperatureType getSimulatedTemperature() { return SimulatedTemperature; }

    /* Puts the chip back to never having answered, which is the state the overlay has to
       stay away in. */
    static void clearSimulatedTemperature() { SimulatedTemperatureValid = false; }
    static bool isSimulatedTemperatureValid() { return SimulatedTemperatureValid; }

  private:
    static inline TemperatureType SimulatedTemperature{0};
    static inline bool SimulatedTemperatureValid{false};
};

#endif // _DS3231_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
