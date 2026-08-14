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
 *      \details    Two things the chip keeps for the clock: the time it counts on its
 *                  battery, and the die temperature it measures to compensate its crystal.
 *                  The first fills the gap between power-on and the first SNTP answer, the
 *                  second is what the temperature overlay shows.
 *
 *                  What is stored in the time registers is UTC, not the local time the
 *                  display shows. A clock that stored local time would have nothing to say
 *                  about the hour that occurs twice when summer time ends, and would be an
 *                  hour wrong from a switch until the next SNTP answer. RealTimeClock does
 *                  the conversion in both directions.
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
#include "ClockDateTime.h"

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

/* Time and date, seven registers in one block, every field in BCD. */
#define DS3231_REG_SECONDS                              0x00u
#define DS3231_DATE_TIME_NUMBER_OF_BYTES                7u

/* The bits beside the value in the registers that carry one: the hours register selects
   12-hour mode in bit 6 and holds the AM/PM or the tens bit in bit 5, and the month
   register carries the century in bit 7. Both are masked off on the way in and left at
   zero on the way out - the clock runs in 24-hour mode, and ClockDate covers 2000 to 2099,
   which is exactly the century the chip counts in with that bit clear. */
#define DS3231_HOURS_VALUE_MASK                         0b00111111u
#define DS3231_MONTH_VALUE_MASK                         0b00011111u

/* Status register, whose top bit says the oscillator stopped - which is how a chip that
   ran out of battery says its time is not to be believed. */
#define DS3231_REG_STATUS                               0x0Fu
#define DS3231_STATUS_OSCILLATOR_STOPPED_MASK           0b10000000u

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

    StdReturnType readRegisters(byte Register, byte* Values, byte Length);
    StdReturnType writeRegisters(byte Register, const byte* Values, byte Length);
    StdReturnType clearOscillatorStopped();
    bool hasOscillatorStopped();

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

    /* The registers hold their fields in BCD, two decimal digits per byte. Public for the
       same reason as toTenths(): it is checkable without a bus, and the assertions below
       do it. */
    static constexpr byte fromBcd(byte Value) { return static_cast<byte>(((Value >> 4u) * 10u) + (Value & 0x0Fu)); }
    static constexpr byte toBcd(byte Value) { return static_cast<byte>(((Value / 10u) << 4u) | (Value % 10u)); }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    /* E_NOT_OK while no reading has arrived yet, which is what a caller has to tell apart
       from a reading of zero degrees. */
    StdReturnType getTemperature(TemperatureType& sTemperature) const {
        if(!TemperatureValid) { return E_NOT_OK; }

        sTemperature = Temperature;
        return E_OK;
    }

    /* Reads what the chip counted, in UTC. E_NOT_OK when the chip does not answer, when it
       reports that its oscillator stopped - a battery that ran out, so what it counted
       since is nothing - or when the registers do not form a date, which is what an
       unwritten chip and a botched transfer both look like. */
    StdReturnType getDateTime(ClockDateTime&);

    // set methods
    /* Writes the time to the chip, in UTC, and clears the stopped flag with it: the value
       just written is by definition believable again. */
    StdReturnType setDateTime(const ClockDateTime&);

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

/* Both ends of what the registers carry, and the round trip in between. */
static_assert(DS3231::fromBcd(0x59u) == 59u, "the tens digit sits in the upper nibble");
static_assert(DS3231::toBcd(59u) == 0x59u, "and goes back there");
static_assert(DS3231::fromBcd(DS3231::toBcd(0u)) == 0u, "zero must round-trip");
static_assert(DS3231::fromBcd(DS3231::toBcd(99u)) == 99u, "the largest two-digit value must round-trip");

#endif // _DS3231_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
