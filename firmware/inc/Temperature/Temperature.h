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

    static constexpr TemperatureType kTenthsPerDegree{DS3231::kTenthsPerDegree};

    /* "-12.3C" and its terminator, which is the longest a reading the chip can produce
       makes: it measures from -40 to +85 degrees. */
    static constexpr byte kStringLength{6u + 1u};

    /* The shown form carries the degree sign as well, which is one byte more - it is
       Latin-1 here, the same single-byte encoding the font tables are indexed by. */
    static constexpr byte kStringToShowLength{kStringLength + 1u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr char kDegreeUnit{'C'};
    /* Latin-1 0xB0, which is what the font tables index by and what the letter table's
       umlauts already are. It stays out of the reported form on purpose - see
       getTemperatureString() below. */
    static constexpr char kDegreeSign{'\xB0'};
    static constexpr char kDecimalPoint{'.'};

    constexpr Temperature() { }
    ~Temperature() { }

    DS3231 sensor_;

    /* Sign, whole part and tenth are taken apart before they are written, so that the
       digits themselves are always those of a positive number - a remainder of a negative
       value is negative in C, and would print a second minus in the middle of the
       reading. */
    static char* AppendSign(TemperatureType tenths, char* string) {
        if(tenths >= 0) { return string; }

        return AppendChar('-', string);
    }
    static TemperatureType WholeDegrees(TemperatureType tenths) { return Absolute(tenths) / kTenthsPerDegree; }
    static TemperatureType TenthOfDegree(TemperatureType tenths) { return Absolute(tenths) % kTenthsPerDegree; }
    static constexpr TemperatureType Absolute(TemperatureType value) { return (value < 0) ? static_cast<TemperatureType>(-value) : value; }

    static char* AppendChar(char character, char* string) { string[0u] = character; return &string[1u]; }
    static char* AppendDegreeSign(bool wanted, char* string) {
        if(!wanted) { return string; }

        return AppendChar(kDegreeSign, string);
    }

    /* Both public forms come through here, so the digits cannot drift apart between what
       is shown and what is reported - only the degree sign differs. */
    StdReturnType AppendReading(char* string, bool with_degree_sign) const {
        TemperatureType tenths{0};

        string[0u] = STD_NULL_CHARACTER;
        if(GetTemperature(tenths) == kENotOk) { return kENotOk; }

        char* position = string;
        position = AppendSign(tenths, position);
        position = AppendNumber(WholeDegrees(tenths), position);
        position = AppendChar(kDecimalPoint, position);
        position = AppendNumber(TenthOfDegree(tenths), position);
        position = AppendDegreeSign(with_degree_sign, position);
        position = AppendChar(kDegreeUnit, position);
        *position = STD_NULL_CHARACTER;
        return kEOk;
    }
    static char* AppendNumber(TemperatureType value, char* string) {
        itoa(value, string, 10u);
        return &string[DigitsOfNumber(value)];
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Temperature& GetInstance() {
        static Temperature singleton_instance;
        return singleton_instance;
    }

    // get methods
    byte GetTaskCycle() const { return sensor_.GetTaskCycle(); }

    /* E_NOT_OK while there is no reading, which is the answer a build without the chip
       keeps giving. */
    StdReturnType GetTemperature(TemperatureType& temperature) const { return sensor_.GetTemperature(temperature); }

    bool IsTemperatureAvailable() const {
        TemperatureType unused{0};
        return GetTemperature(unused) == kEOk;
    }

    /* The reading as it is reported, "23.4C", into a buffer of StringLength. Written by
       whoever needs it rather than kept here, because the overlay needs it to stand still
       while its text scrolls, and the status command needs it once.

       An empty string when there is no reading - the degree it would otherwise print is
       the one thing a reader must not mistake for a measurement.

       No degree sign in this one, and that is the point of the split: this string travels
       the serial line and the web socket, and a web socket text frame has to be valid
       UTF-8. A raw Latin-1 0xB0 in it makes the browser close the connection with a
       protocol error rather than draw one character oddly. */
    StdReturnType GetTemperatureString(char* string) const {
        return AppendReading(string, false);
    }

    /* The same reading as it is shown, "23.4°C", into a buffer of StringToShowLength. The
       display indexes the font tables by Latin-1 byte, so the sign can be one here. */
    StdReturnType GetTemperatureStringToShow(char* string) const {
        return AppendReading(string, true);
    }

    // methods
    void Task() { sensor_.Task(); }
};

#endif // _TEMPERATURE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
