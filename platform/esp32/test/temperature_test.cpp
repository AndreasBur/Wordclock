/* The path from the clock chip's register pair to the letters the overlay hands the text
   renderer. What it is here for is the part no arithmetic in a header can show: that the
   driver addresses the right register, that a silent bus leaves the overlay away rather
   than showing a zero, and that the reading survives the trip as the string it is read
   back as. */
#include "Arduino.h"
#include "DS3231.h"
#include "Overlays.h"
#include "Temperature.h"

#include <Wire.h>

#include <cstdio>
#include <cstring>

static int Failures = 0;
static void check(bool Ok, const char* What) {
    printf("%-56s %s\n", What, Ok ? "ok" : "FAIL");
    if(!Ok) Failures++;
}

/* One reading, handed over the way the chip would: the two temperature registers. */
static void answerWith(uint8_t Degrees, uint8_t Fraction)
{
    Wire.Response = {Degrees, Fraction};
    Wire.Written.clear();
    Temperature::getInstance().task();
}

int main()
{
    Temperature& temperature = Temperature::getInstance();
    Overlays& overlays = Overlays::getInstance();

    /* A bus with nothing on it: requestFrom() hands back no bytes, and nothing must look
       like a measurement afterwards. */
    Wire.Response.clear();
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
    Wire.Response.clear();
    temperature.task();
    check(temperature.isTemperatureAvailable(), "a lost transfer keeps the last reading");

    printf("\n%s\n", Failures == 0 ? "all checks passed" : "FAILURES");
    return Failures == 0 ? 0 : 1;
}
