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
/**     \file       Temperature.h
 *      \brief      What the clock knows about the temperature
 *
 *      \details    The same arrangement as Illuminance and its BH1750: the core owns the
 *                  driver, reaches it by header name, and every platform supplies its own
 *                  DS3231.h behind that name.
 *
 *                  There is no reading until the chip has answered once, and a build with
 *                  no chip on the bus never gets one. That is why the value comes with a
 *                  return code rather than as a number: the overlay has to be able to stay
 *                  away, and a temperature of zero is a perfectly ordinary reading it must
 *                  not be confused with.
 *
******************************************************************************************************************************************************/
#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "DS3231.h"

/******************************************************************************************************************************************************
 *  C L A S S   T E M P E R A T U R E
******************************************************************************************************************************************************/
class Temperature
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* Tenths of a degree Celsius, signed. */
    using TemperatureType = DS3231::TemperatureType;

    static constexpr TemperatureType TenthsPerDegree{DS3231::TenthsPerDegree};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    constexpr Temperature() { }
    ~Temperature() { }

    DS3231 Sensor;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Temperature& getInstance() {
        static Temperature SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    byte getTaskCycle() const { return Sensor.getTaskCycle(); }

    /* E_NOT_OK while there is no reading, which is the answer a build without the chip
       keeps giving. */
    StdReturnType getTemperature(TemperatureType& sTemperature) const { return Sensor.getTemperature(sTemperature); }

    bool isTemperatureAvailable() const {
        TemperatureType Unused{0};
        return getTemperature(Unused) == E_OK;
    }

    // methods
    void task() { Sensor.task(); }
};

#endif // _TEMPERATURE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
