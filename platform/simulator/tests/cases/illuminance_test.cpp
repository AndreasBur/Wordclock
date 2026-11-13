/*
 * What the light sensor's reading does to the display's brightness.
 *
 * The automatic is checked on DisplayBrightness itself rather than through the strip: what is at
 * stake is the mapping from lux to a level, and the strip only shows whether a pixel ended up lit
 * at all. The sensor is the simulator's stub, so a room is a number set from here.
 *
 * None of this was covered before, and both cases below fail on the arithmetic that stood here: the
 * lower calibration bound was read nowhere, and the upper one defaulted to the sensor's 65535 lx,
 * which left every indoor clock pinned to the automatic's floor.
 */

#include "Illuminance.h"

#include "DisplayBrightness.h"
#include "cases.h"
#include "check.h"

namespace {
/*
 * The automatic's floor, which DisplayBrightness keeps to itself. Repeated here on purpose:
 * a test that read the constant from the class under test could not tell that the value
 * moved.
 */
constexpr byte kAutomaticFloor{8u};

// The reading the automatic works from. task() is what carries the stub's static into the
// driver, which is the same path the scheduler takes on hardware.
void SetRoom(Illuminance::IlluminanceType lux) {
  BH1750::SetSimulatedIlluminance(lux);
  Illuminance::GetInstance().Task();
}

void SetBounds(Illuminance::IlluminanceType dark, Illuminance::IlluminanceType bright) {
  Illuminance::GetInstance().SetCalibrationValuesMinValue(dark);
  Illuminance::GetInstance().SetCalibrationValuesMaxValue(bright);
}

/* A fresh DisplayBrightness each time, so nothing carries over between the rooms below. */
byte AutomaticAt(Illuminance::IlluminanceType lux, byte setting) {
  DisplayBrightness brightness;

  SetRoom(lux);
  brightness.SetBrightness(setting);
  brightness.EnableAutomatic();

  return brightness.CalcConfiguredBrightness();
}

/* The sensor is a singleton and the cases after this file's expect their own state. */
void RestoreDefaults() {
  Illuminance::GetInstance().ResetToDefaults();
  BH1750::SetSimulatedIlluminance(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE);
}

}  // namespace

/*
 * The defaults, which is the state every clock is in until somebody calibrates one. A
 * living room has to arrive at a level somebody would call dimmed rather than at the floor
 * - that floor is what the previous scale gave for anything short of direct sunlight, and
 * a display stuck at 8 of 255 reads as a broken sensor, not as a default.
 */
void TestAutomaticDefaultsSuitARoom() {
  RestoreDefaults();

  byte const dark = AutomaticAt(0u, 255u);
  byte const living_room = AutomaticAt(100u, 255u);
  byte const bright_room = AutomaticAt(500u, 255u);
  byte const daylight_brightness = AutomaticAt(BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE, 255u);

  Expect(dark == kAutomaticFloor, "an unlit room takes the display to the floor and no further");
  Expect(living_room > kAutomaticFloor * 2u, "a living room has to be clearly off the floor");
  Expect(living_room < bright_room, "and darker than a bright one");
  Expect(bright_room < daylight_brightness, "which is darker again than the calibrated maximum");
  Expect(daylight_brightness == 255u,
         "at that maximum the automatic gives back the brightness that was set");

  /* Above the upper bound there is nothing left to give, and no wrap on the way. */
  Expect(AutomaticAt(BH1750_ILLUMINANCE_MAX_LX_VALUE, 255u) == 255u, "and it stays there above it");

  /*
   * The scale that used to stand here, kept as a bound rather than as a story: calibrating
   * to the sensor's own ceiling is what the old default was, and it puts the same living
   * room straight back on the floor.
   */
  SetBounds(0u, BH1750_ILLUMINANCE_MAX_LX_VALUE);
  Expect(AutomaticAt(100u, 255u) == kAutomaticFloor,
         "the sensor's full range is what pinned a room to the floor");

  RestoreDefaults();
}

/*
 * Both bounds, because the lower one used to be decoration: the procedure that calibrates
 * it answered, Persistence saved it across a restart, and the brightness never moved by a
 * single step. Raising the floor under an unchanged room has to darken the display.
 */
void TestAutomaticSpreadsBetweenBothBounds() {
  RestoreDefaults();

  SetBounds(0u, 1000u);
  byte const over_a_low_floor = AutomaticAt(500u, 255u);

  SetBounds(400u, 1000u);
  byte const over_a_high_floor = AutomaticAt(500u, 255u);

  Expect(over_a_high_floor < over_a_low_floor,
         "the calibrated minimum has to change what the display does");

  /* The ends of the span, which is where a spread differs from a division. */
  SetBounds(200u, 800u);
  Expect(AutomaticAt(200u, 255u) == kAutomaticFloor,
         "at the lower bound the display sits on the floor");
  Expect(AutomaticAt(100u, 255u) == kAutomaticFloor, "and below it as well");
  Expect(AutomaticAt(800u, 255u) == 255u, "at the upper bound it is the full setting");

  /*
   * Bounds in the wrong order are what a calibration done the wrong way round stores, and
   * nothing rejects them. The display keeps what its owner set rather than dividing by a
   * span of zero.
   */
  SetBounds(900u, 100u);
  Expect(AutomaticAt(500u, 200u) == 200u, "bounds in the wrong order leave the setting alone");
  SetBounds(500u, 500u);
  Expect(AutomaticAt(500u, 200u) == 200u, "and so does a span of nothing");

  /* A setting already below the floor is the owner's, not the automatic's to raise. */
  SetBounds(1u, 1000u);
  Expect(AutomaticAt(1000u, 4u) == 4u, "a setting under the floor is left where it was");

  RestoreDefaults();
}
