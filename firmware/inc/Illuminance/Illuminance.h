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
/**     \file       Illuminance.h
 *      \brief      The ambient light the display dims itself by
 *
 *      \details    A thin owner around the platform's BH1750, so the core reaches the
 *                  sensor without knowing which one it holds - the hardware driver or the
 *                  simulator's stub.
 *
 *                  There is deliberately no init() here to match the other modules': the
 *                  sensor is private and the scheduler only ever calls task(), so the
 *                  driver sets itself up from its first one. See the platform's BH1750.h.
******************************************************************************************************************************************************/
#ifndef _ILLUMINANCE_H_
#define _ILLUMINANCE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "BH1750.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Illuminance configuration parameter */


/* Illuminance parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   I L L U M I N A N C E
******************************************************************************************************************************************************/
class Illuminance
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using IlluminanceType = BH1750::IlluminanceType;
    using CalibrationValuesType = BH1750::CalibrationValuesType;
    using ModeType = BH1750::ModeType;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    constexpr Illuminance() { }
    ~Illuminance() { }
    
    BH1750 Sensor;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Illuminance& getInstance() {
        static Illuminance singletonInstance;
        return singletonInstance;
    }

    byte getTaskCycle() const { return Sensor.getTaskCycle(); }
    IlluminanceType getIlluminance() const { return Sensor.getIlluminance(); }
    CalibrationValuesType getCalibrationValues() const { return Sensor.getCalibrationValues(); }
    IlluminanceType getCalibrationValuesMaxValue() const { return Sensor.getCalibrationValues().MaxValue; }
    IlluminanceType getCalibrationValuesMinValue() const { return Sensor.getCalibrationValues().MinValue; }

    // set methods
    void setCalibrationValuesMaxValue(IlluminanceType MaxValue) { Sensor.setCalibrationValuesMaxValue(MaxValue); }
    void setCalibrationValuesMinValue(IlluminanceType MinValue) { Sensor.setCalibrationValuesMinValue(MinValue); }
    void setCalibrationValues(CalibrationValuesType CalibrationValues) { Sensor.setCalibrationValues(CalibrationValues); }

    // methods
    /* The calibration back to the span an uncalibrated clock starts in - the same two
       bounds the driver initialises from, so a reset and a fresh chip agree. Not the
       sensor's full range: see the defaults in the platform's BH1750.h for why that one
       leaves the automatic sitting at its floor in every room. */
    void resetToDefaults() {
        setCalibrationValuesMaxValue(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE);
        setCalibrationValuesMinValue(BH1750_CALIBRATION_MIN_DEFAULT_LX_VALUE);
    }

    /* Both refuse while the sensor has nothing measured, so the answer to the procedure is
       the sensor's rather than an unconditional success. */
    StdReturnType startCalibrationMaxValue() { return Sensor.startCalibrationMaxValue(); }
    StdReturnType startCalibrationMinValue() { return Sensor.startCalibrationMinValue(); }

    void task() { Sensor.task(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
