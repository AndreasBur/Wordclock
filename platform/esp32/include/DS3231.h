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
 *      \brief      Temperature of the DS3231 real time clock chip
 *
 *      \details    The DS3231 measures its own die temperature to compensate its crystal,
 *                  and hands that reading out in two registers. It is what the temperature
 *                  overlay shows.
 *
 *                  Only the temperature, deliberately: the clock on this platform gets its
 *                  time from SNTP and keeps it in the system clock, so the chip's
 *                  time-keeping side has nothing to do here yet. It is what would close the
 *                  gap after a power cut, and the same chip on the same bus is then already
 *                  wired - see the roadmap.
 *
 *                  A die temperature is not a room temperature. The chip sits in the case
 *                  next to whatever else is warm in there, so what it reads is a degree or
 *                  two above the room and needs the offset the case turns out to have.
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
/* The only address the chip has; A0 to A2 do not exist on it. */
#define DS3231_I2C_ADDR                                 0x68u

/* The same pins and speed as the light sensor, which is the point of putting both on one
   bus. Wire.begin() is idempotent, so whichever driver runs first opens it. */
#define DS3231_I2C_PIN_SDA                              SDA
#define DS3231_I2C_PIN_SCL                              SCL
#define DS3231_I2C_FREQUENCY_HZ                         100000u

/* In scheduler ticks, so 250 * 10 ms = 2.5 s between readings. The chip converts on its
   own every 64 s, so this does not set how fresh the value is - it sets how long after a
   start it takes until there is one at all, and how quickly a reappearing chip is picked
   up again. The scheduler counts cycles in a byte, so this cannot be raised past 255. */
#define DS3231_TASK_CYCLE                               250u

/* DS3231 parameter */
/* Temperature register pair: the whole degrees as a signed byte, and the quarters in the
   top two bits of the byte behind it. */
#define DS3231_REG_TEMPERATURE_MSB                      0x11u
#define DS3231_TEMPERATURE_NUMBER_OF_BYTES              2u
#define DS3231_TEMPERATURE_FRACTION_GM                  0b11u
#define DS3231_TEMPERATURE_FRACTION_GP                  6u

/******************************************************************************************************************************************************
 *  C L A S S   D S 3 2 3 1
******************************************************************************************************************************************************/
class DS3231
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* Tenths of a degree Celsius, signed. Tenths rather than the chip's own quarters
       because that is what a display shows, and signed because the chip reads below zero
       and the overlay has to say so. */
    using TemperatureType = int16_t;

    static constexpr TemperatureType TenthsPerDegree{10};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{DS3231_TASK_CYCLE};
    static constexpr TemperatureType QuartersPerDegree{4};

    TemperatureType Temperature{0};
    /* Until the chip has answered once there is nothing to show, and the overlay stays
       away rather than showing a zero that looks like a reading. */
    bool TemperatureValid{false};
    bool BusStarted{false};

    // functions
    StdReturnType startBus();
    StdReturnType readTemperature();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DS3231() { }
    ~DS3231() { }

    /* The quarters sit in the top two bits, and the whole degrees are signed - so the two
       halves are combined in quarters, where the fraction is a plain addition even below
       zero, and only then scaled. Truncates towards zero, which loses at most a tenth.

       Public because it is the one part of this driver that can be checked without a bus,
       and the assertions below do exactly that. */
    static constexpr TemperatureType toTenths(int8_t Degrees, byte FractionRegister) {
        const TemperatureType Quarters = static_cast<TemperatureType>(Degrees * QuartersPerDegree) +
                                         READ_BIT_GROUP(FractionRegister, DS3231_TEMPERATURE_FRACTION_GM, DS3231_TEMPERATURE_FRACTION_GP);
        return static_cast<TemperatureType>((Quarters * TenthsPerDegree) / QuartersPerDegree);
    }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    /* E_NOT_OK while no reading has arrived yet, which is what a caller has to tell apart
       from a reading of zero degrees. */
    StdReturnType getTemperature(TemperatureType& sTemperature) const {
        if(!TemperatureValid) { return E_NOT_OK; }

        sTemperature = Temperature;
        return E_OK;
    }

    // methods
    void task();
};

/* The register pair, read off the datasheet's own examples. */
static_assert(DS3231::TenthsPerDegree == 10, "the conversion below is written in tenths");

/* The datasheet's own two examples, which are the pair that matters: one above zero, and
   one below where the fraction counts upwards from a whole degree more negative than the
   result. Outside the class because a member function cannot be called in a constant
   expression until the class is complete. */
static_assert(DS3231::toTenths(0x19, 0x40u) == 252, "+25.25 degrees must read as 25.2");
static_assert(DS3231::toTenths(static_cast<int8_t>(0xFE), 0x80u) == -15, "-1.5 degrees must read as -1.5");
static_assert(DS3231::toTenths(0, 0x00u) == 0, "zero must stay zero");

#endif // _DS3231_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
