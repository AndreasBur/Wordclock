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

    /* "-12.3C" and its terminator, which is the longest a reading the chip can produce
       makes: it measures from -40 to +85 degrees. */
    static constexpr byte StringLength{6u + 1u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr char DegreeUnit{'C'};
    static constexpr char DecimalPoint{'.'};

    constexpr Temperature() { }
    ~Temperature() { }

    DS3231 Sensor;

    /* Sign, whole part and tenth are taken apart before they are written, so that the
       digits themselves are always those of a positive number - a remainder of a negative
       value is negative in C, and would print a second minus in the middle of the
       reading. */
    static char* appendSign(TemperatureType Tenths, char* String) {
        if(Tenths >= 0) { return String; }

        return appendChar('-', String);
    }
    static TemperatureType wholeDegrees(TemperatureType Tenths) { return absolute(Tenths) / TenthsPerDegree; }
    static TemperatureType tenthOfDegree(TemperatureType Tenths) { return absolute(Tenths) % TenthsPerDegree; }
    static constexpr TemperatureType absolute(TemperatureType Value) { return (Value < 0) ? static_cast<TemperatureType>(-Value) : Value; }

    static char* appendChar(char Char, char* String) { String[0u] = Char; return &String[1u]; }
    static char* appendNumber(TemperatureType Value, char* String) {
        itoa(Value, String, 10u);
        return &String[digitsOfNumber(Value)];
    }

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

    /* The reading as it is shown and reported, "23.4C", into a buffer of StringLength.
       Written by whoever displays it rather than kept here, because the overlay needs it
       to stand still while its text scrolls, and the status command needs it once.

       An empty string when there is no reading - the degree it would otherwise print is
       the one thing a reader must not mistake for a measurement. The letter stands in for
       the degree sign, which the font tables do not carry. */
    StdReturnType getTemperatureString(char* String) const {
        TemperatureType Tenths{0};

        String[0u] = STD_NULL_CHARACTER;
        if(getTemperature(Tenths) == E_NOT_OK) { return E_NOT_OK; }

        char* Position = String;
        Position = appendSign(Tenths, Position);
        Position = appendNumber(wholeDegrees(Tenths), Position);
        Position = appendChar(DecimalPoint, Position);
        Position = appendNumber(tenthOfDegree(Tenths), Position);
        Position = appendChar(DegreeUnit, Position);
        *Position = STD_NULL_CHARACTER;
        return E_OK;
    }

    // methods
    void task() { Sensor.task(); }
};

#endif // _TEMPERATURE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
