/* The clock chip and what the firmware makes of it: the temperature the overlay shows, and
   the time it keeps over a power cut. What these checks are here for is the part no
   arithmetic in a header can show - which register a transfer is aimed at, what a chip that
   says its time is worthless does to the answer, and that a reading survives the trip as
   the string it is read back as. */
#include "Arduino.h"
#include "DS3231.h"
#include "Overlays.h"
#include "Temperature.h"
#include "check.h"

#include <Wire.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>


/* One temperature reading, handed over the way the chip would. */
static void answerWith(uint8_t Degrees, uint8_t Fraction)
{
    Wire.respondWith({Degrees, Fraction});
    Wire.Written.clear();
    Temperature::getInstance().task();
}

/* The status register first, then the seven time registers - the order the driver reads
   them in, out of one prepared buffer. */
static void answerWithTime(uint8_t Status, std::vector<uint8_t> Registers)
{
    Registers.insert(Registers.begin(), Status);
    Wire.respondWith(Registers);
    Wire.Written.clear();
}

static void testTemperature()
{
    Temperature& temperature = Temperature::getInstance();
    Overlays& overlays = Overlays::getInstance();

    /* A bus with nothing on it: requestFrom() hands back no bytes, and nothing must look
       like a measurement afterwards. */
    Wire.respondWith({});
    temperature.task();
    check(!temperature.isTemperatureAvailable(), "a silent bus leaves no reading behind");

    overlays.setTemperatureIsActive(true);
    check(overlays.showTemperatureNow() == E_NOT_OK, "without a reading the overlay stays away");

    /* The datasheet's own example: 25.25 degrees, which the firmware keeps in tenths. */
    answerWith(0x19u, 0x40u);
    check(temperature.isTemperatureAvailable(), "an answered read gives a reading");

    Temperature::TemperatureType Tenths{0};
    check(temperature.getTemperature(Tenths) == E_OK && Tenths == 252,
          "the register pair reads as 25.2 degrees");
    check(Wire.Written.size() >= 2u && Wire.Written[1] == DS3231_REG_TEMPERATURE_MSB,
          "the read is addressed at the temperature register");

    check(overlays.showTemperatureNow() == E_OK, "with a reading the overlay starts");
    check(strcmp(overlays.getTemperatureString(), "25.2C") == 0,
          "the overlay shows the reading with one decimal");
    check(overlays.abort() == E_OK, "the overlay ends again");

    /* Below zero the whole degrees are more negative than the reading, and the quarters
       count upwards from there - so the sign belongs in front of the string and nowhere
       else in it. */
    answerWith(0xFEu, 0x80u);
    check(overlays.showTemperatureNow() == E_OK, "the overlay starts on a reading below zero");
    check(strcmp(overlays.getTemperatureString(), "-1.5C") == 0,
          "a reading below zero carries exactly one minus");
    check(overlays.abort() == E_OK, "the overlay ends again");

    /* A dropped transfer keeps the last reading rather than taking the overlay away for a
       whole period - the room does not change while one transfer is lost. */
    Wire.respondWith({});
    temperature.task();
    check(temperature.isTemperatureAvailable(), "a lost transfer keeps the last reading");

    overlays.setTemperatureIsActive(false);
}

static void testTimeRegisters()
{
    DS3231 chip;
    ClockDateTime DateTime;

    /* 14 August 2026, 09:41:07 UTC - a Friday, which is what the weekday register has to
       come out as when it is written back below. */
    const std::vector<uint8_t> Registers{0x07u, 0x41u, 0x09u, 0x06u, 0x14u, 0x08u, 0x26u};

    answerWithTime(0x00u, Registers);
    check(chip.getDateTime(DateTime) == E_OK, "a running chip hands its time over");
    check(DateTime.getDateYear() == 2026u && DateTime.getDateMonth() == 8u && DateTime.getDateDay() == 14u,
          "the date registers read as the date they hold");
    check(DateTime.getTimeHour() == 9u && DateTime.getTimeMinute() == 41u && DateTime.getTimeSecond() == 7u,
          "the time registers read as the time they hold");
    check(Wire.Written.size() >= 4u && Wire.Written[1] == DS3231_REG_STATUS && Wire.Written[3] == DS3231_REG_SECONDS,
          "the status register is asked before the time block");

    /* The flag the chip sets when its oscillator stood still. The registers below it still
       read like a date - they are the same ones as above - so taking them would set the
       clock to whenever the battery gave out. */
    answerWithTime(DS3231_STATUS_OSCILLATOR_STOPPED_MASK, Registers);
    check(chip.getDateTime(DateTime) == E_NOT_OK, "a stopped oscillator makes the time unusable");

    /* A chip that was never written reads zero everywhere, which is month zero and day
       zero. ClockDate rejects both, and that is what has to reach the caller. */
    answerWithTime(0x00u, {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u});
    check(chip.getDateTime(DateTime) == E_NOT_OK, "an unwritten chip is not a date");

    check(chip.getDateTime(DateTime) == E_NOT_OK, "a silent bus is not a date either");
}

static void testWritingTheTime()
{
    DS3231 chip;
    ClockDateTime DateTime;

    DateTime.setDateYear(2026u);
    DateTime.setDateMonth(8u);
    DateTime.setDateDay(14u);
    DateTime.setTimeHour(9u);
    DateTime.setTimeMinute(41u);
    DateTime.setTimeSecond(7u);

    /* Read back by the flag-clearing that follows the write. Two bits set: the one that
       says the oscillator stopped, and one of the alarm flags beside it, which must
       survive. */
    Wire.respondWith({0x88u});
    Wire.Written.clear();
    check(chip.setDateTime(DateTime) == E_OK, "the chip takes a time");

    /* Address, register number, seven values - all in BCD, and the weekday counted from
       one where ClockDate counts from zero. Friday is 5 there, 6 here. */
    const std::vector<uint8_t> Expected{DS3231_I2C_ADDR, DS3231_REG_SECONDS,
                                        0x07u, 0x41u, 0x09u, 0x06u, 0x14u, 0x08u, 0x26u};
    check(Wire.Written.size() >= Expected.size() &&
          std::equal(Expected.begin(), Expected.end(), Wire.Written.begin()),
          "the time goes out as the register block the chip expects");

    /* And the flag is cleared afterwards, with the bit beside it left alone. */
    check(Wire.Written.size() == Expected.size() + 5u &&
          Wire.Written[Expected.size() + 1u] == DS3231_REG_STATUS &&
          Wire.Written[Expected.size() + 4u] == 0x08u,
          "the stopped flag is cleared and the alarm flag beside it kept");
}

int main()
{
    testTemperature();
    testTimeRegisters();
    testWritingTheTime();
    return report();
}
