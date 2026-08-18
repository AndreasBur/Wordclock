/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       illuminance_test.cpp
 *      \brief      What the light sensor's reading does to the display's brightness
 *
 *      \details    The automatic is checked on DisplayBrightness itself rather than
 *                  through the strip: what is at stake is the mapping from lux to a level,
 *                  and the strip only shows whether a pixel ended up lit at all. The
 *                  sensor is the simulator's stub, so a room is a number set from here.
 *
 *                  None of this was covered before, and both cases below fail on the
 *                  arithmetic that stood here: the lower calibration bound was read
 *                  nowhere, and the upper one defaulted to the sensor's 65535 lx, which
 *                  left every indoor clock pinned to the automatic's floor.
******************************************************************************************************************************************************/

#include "check.h"
#include "cases.h"

#include "DisplayBrightness.h"
#include "Illuminance.h"

namespace {

/* The automatic's floor, which DisplayBrightness keeps to itself. Repeated here on purpose:
   a test that read the constant from the class under test could not tell that the value
   moved. */
constexpr byte AutomaticFloor{8u};

/* The reading the automatic works from. task() is what carries the stub's static into the
   driver, which is the same path the scheduler takes on hardware. */
void setRoom(Illuminance::IlluminanceType Lux)
{
    BH1750::setSimulatedIlluminance(Lux);
    Illuminance::getInstance().task();
}

void setBounds(Illuminance::IlluminanceType Dark, Illuminance::IlluminanceType Bright)
{
    Illuminance::getInstance().setCalibrationValuesMinValue(Dark);
    Illuminance::getInstance().setCalibrationValuesMaxValue(Bright);
}

/* A fresh DisplayBrightness each time, so nothing carries over between the rooms below. */
byte automaticAt(Illuminance::IlluminanceType Lux, byte Setting)
{
    DisplayBrightness Brightness;

    setRoom(Lux);
    Brightness.setBrightness(Setting);
    Brightness.enableAutomatic();

    return Brightness.calcConfiguredBrightness();
}

/* The sensor is a singleton and the cases after this file's expect their own state. */
void restoreDefaults()
{
    Illuminance::getInstance().resetToDefaults();
    BH1750::setSimulatedIlluminance(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE);
}

} // namespace


/* The defaults, which is the state every clock is in until somebody calibrates one. A
   living room has to arrive at a level somebody would call dimmed rather than at the floor
   - that floor is what the previous scale gave for anything short of direct sunlight, and
   a display stuck at 8 of 255 reads as a broken sensor, not as a default. */
void testAutomaticDefaultsSuitARoom()
{
    restoreDefaults();

    const byte Dark = automaticAt(0u, 255u);
    const byte LivingRoom = automaticAt(100u, 255u);
    const byte BrightRoom = automaticAt(500u, 255u);
    const byte Daylight = automaticAt(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, 255u);

    expect(Dark == AutomaticFloor, "an unlit room takes the display to the floor and no further");
    expect(LivingRoom > AutomaticFloor * 2u, "a living room has to be clearly off the floor");
    expect(LivingRoom < BrightRoom, "and darker than a bright one");
    expect(BrightRoom < Daylight, "which is darker again than the calibrated maximum");
    expect(Daylight == 255u, "at that maximum the automatic gives back the brightness that was set");

    /* Above the upper bound there is nothing left to give, and no wrap on the way. */
    expect(automaticAt(BH1750_ILLUMINANCE_MAX_LX_VALUE, 255u) == 255u, "and it stays there above it");

    /* The scale that used to stand here, kept as a bound rather than as a story: calibrating
       to the sensor's own ceiling is what the old default was, and it puts the same living
       room straight back on the floor. */
    setBounds(0u, BH1750_ILLUMINANCE_MAX_LX_VALUE);
    expect(automaticAt(100u, 255u) == AutomaticFloor, "the sensor's full range is what pinned a room to the floor");

    restoreDefaults();
}


/* Both bounds, because the lower one used to be decoration: the procedure that calibrates
   it answered, Persistence saved it across a restart, and the brightness never moved by a
   single step. Raising the floor under an unchanged room has to darken the display. */
void testAutomaticSpreadsBetweenBothBounds()
{
    restoreDefaults();

    setBounds(0u, 1000u);
    const byte OverALowFloor = automaticAt(500u, 255u);

    setBounds(400u, 1000u);
    const byte OverAHighFloor = automaticAt(500u, 255u);

    expect(OverAHighFloor < OverALowFloor, "the calibrated minimum has to change what the display does");

    /* The ends of the span, which is where a spread differs from a division. */
    setBounds(200u, 800u);
    expect(automaticAt(200u, 255u) == AutomaticFloor, "at the lower bound the display sits on the floor");
    expect(automaticAt(100u, 255u) == AutomaticFloor, "and below it as well");
    expect(automaticAt(800u, 255u) == 255u, "at the upper bound it is the full setting");

    /* Bounds in the wrong order are what a calibration done the wrong way round stores, and
       nothing rejects them. The display keeps what its owner set rather than dividing by a
       span of zero. */
    setBounds(900u, 100u);
    expect(automaticAt(500u, 200u) == 200u, "bounds in the wrong order leave the setting alone");
    setBounds(500u, 500u);
    expect(automaticAt(500u, 200u) == 200u, "and so does a span of nothing");

    /* A setting already below the floor is the owner's, not the automatic's to raise. */
    setBounds(1u, 1000u);
    expect(automaticAt(1000u, 4u) == 4u, "a setting under the floor is left where it was");

    restoreDefaults();
}
