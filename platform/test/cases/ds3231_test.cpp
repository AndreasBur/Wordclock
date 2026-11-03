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
static void AnswerWith(uint8_t degrees, uint8_t fraction)
{
    Wire.respondWith({degrees, fraction});
    Wire.Written.clear();
    Temperature::GetInstance().Task();
}

/* The status register first, then the seven time registers - the order the driver reads
   them in, out of one prepared buffer. */
static void AnswerWithTime(uint8_t status, std::vector<uint8_t> registers)
{
    registers.insert(registers.begin(), status);
    Wire.respondWith(registers);
    Wire.Written.clear();
}

static void TestTemperature()
{
    Temperature& temperature = Temperature::GetInstance();
    Overlays& overlays = Overlays::GetInstance();

    /* A bus with nothing on it: requestFrom() hands back no bytes, and nothing must look
       like a measurement afterwards. */
    Wire.respondWith({});
    temperature.Task();
    Check(!temperature.IsTemperatureAvailable(), "a silent bus leaves no reading behind");

    overlays.SetTemperatureIsActive(true);
    Check(overlays.ShowTemperatureNow() == kENotOk, "without a reading the overlay stays away");

    /* The datasheet's own example: 25.25 degrees, which the firmware keeps in tenths. */
    AnswerWith(0x19u, 0x40u);
    Check(temperature.IsTemperatureAvailable(), "an answered read gives a reading");

    Temperature::TemperatureType tenths{0};
    Check(temperature.GetTemperature(tenths) == kEOk && tenths == 252,
          "the register pair reads as 25.2 degrees");
    Check(Wire.Written.size() >= 2u && Wire.Written[1] == DS3231_REG_TEMPERATURE_MSB,
          "the read is addressed at the temperature register");

    Check(overlays.ShowTemperatureNow() == kEOk, "with a reading the overlay starts");
    /* Split around the escape on purpose: "\xB0C" would swallow the C as a third hex digit
       and stop compiling, which is the trap this string invites. */
    Check(strcmp(overlays.GetTemperatureString(), "25.2\xB0" "C") == 0,
          "the overlay shows the reading with one decimal");
    Check(overlays.Abort() == kEOk, "the overlay ends again");

    /* Below zero the whole degrees are more negative than the reading, and the quarters
       count upwards from there - so the sign belongs in front of the string and nowhere
       else in it. */
    AnswerWith(0xFEu, 0x80u);
    Check(overlays.ShowTemperatureNow() == kEOk, "the overlay starts on a reading below zero");
    Check(strcmp(overlays.GetTemperatureString(), "-1.5\xB0" "C") == 0,
          "a reading below zero carries exactly one minus");
    Check(overlays.Abort() == kEOk, "the overlay ends again");

    /* A dropped transfer keeps the last reading rather than taking the overlay away for a
       whole period - the room does not change while one transfer is lost. */
    Wire.respondWith({});
    temperature.Task();
    Check(temperature.IsTemperatureAvailable(), "a lost transfer keeps the last reading");

    overlays.SetTemperatureIsActive(false);
}

static void TestTimeRegisters()
{
    DS3231 chip;
    ClockDateTime date_time;

    /* 14 August 2026, 09:41:07 UTC - a Friday, which is what the weekday register has to
       come out as when it is written back below. */
    const std::vector<uint8_t> registers{0x07u, 0x41u, 0x09u, 0x06u, 0x14u, 0x08u, 0x26u};

    AnswerWithTime(0x00u, registers);
    Check(chip.GetDateTime(date_time) == kEOk, "a running chip hands its time over");
    Check(date_time.GetDateYear() == 2026u && date_time.GetDateMonth() == 8u && date_time.GetDateDay() == 14u,
          "the date registers read as the date they hold");
    Check(date_time.GetTimeHour() == 9u && date_time.GetTimeMinute() == 41u && date_time.GetTimeSecond() == 7u,
          "the time registers read as the time they hold");
    Check(Wire.Written.size() >= 4u && Wire.Written[1] == DS3231_REG_STATUS && Wire.Written[3] == DS3231_REG_SECONDS,
          "the status register is asked before the time block");

    /* The flag the chip sets when its oscillator stood still. The registers below it still
       read like a date - they are the same ones as above - so taking them would set the
       clock to whenever the battery gave out. */
    AnswerWithTime(DS3231_STATUS_OSCILLATOR_STOPPED_MASK, registers);
    Check(chip.GetDateTime(date_time) == kENotOk, "a stopped oscillator makes the time unusable");

    /* A chip that was never written reads zero everywhere, which is month zero and day
       zero. ClockDate rejects both, and that is what has to reach the caller. */
    AnswerWithTime(0x00u, {0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u});
    Check(chip.GetDateTime(date_time) == kENotOk, "an unwritten chip is not a date");

    Check(chip.GetDateTime(date_time) == kENotOk, "a silent bus is not a date either");
}

static void TestWritingTheTime()
{
    DS3231 chip;
    ClockDateTime date_time;

    date_time.SetDateYear(2026u);
    date_time.SetDateMonth(8u);
    date_time.SetDateDay(14u);
    date_time.SetTimeHour(9u);
    date_time.SetTimeMinute(41u);
    date_time.SetTimeSecond(7u);

    /* Read back by the flag-clearing that follows the write. Two bits set: the one that
       says the oscillator stopped, and one of the alarm flags beside it, which must
       survive. */
    Wire.respondWith({0x88u});
    Wire.Written.clear();
    Check(chip.SetDateTime(date_time) == kEOk, "the chip takes a time");

    /* Address, register number, seven values - all in BCD, and the weekday counted from
       one where ClockDate counts from zero. Friday is 5 there, 6 here. */
    const std::vector<uint8_t> expected{DS3231_I2C_ADDR, DS3231_REG_SECONDS,
                                        0x07u, 0x41u, 0x09u, 0x06u, 0x14u, 0x08u, 0x26u};
    Check(Wire.Written.size() >= expected.size() &&
          std::equal(expected.begin(), expected.end(), Wire.Written.begin()),
          "the time goes out as the register block the chip expects");

    /* And the flag is cleared afterwards, with the bit beside it left alone. */
    Check(Wire.Written.size() == expected.size() + 5u &&
          Wire.Written[expected.size() + 1u] == DS3231_REG_STATUS &&
          Wire.Written[expected.size() + 4u] == 0x08u,
          "the stopped flag is cleared and the alarm flag beside it kept");
}

int main()
{
    TestTemperature();
    TestTimeRegisters();
    TestWritingTheTime();
    return Report();
}
