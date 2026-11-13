// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How the brightness setting becomes the level the LEDs are driven with.
 */

#include "DisplayBrightness.h"

/*
 * The room's brightness, spread between the two calibrated bounds rather than divided
 * by the upper one alone. Dividing left the lower bound with no effect at all - the
 * procedure that calibrates it answered, Persistence stored it, and nothing ever read
 * it - and it put a living room at 100 lx a sixth of a percent up a scale that ends at
 * the sensor's 65535, so the automatic held every indoor clock at its floor.
 */
byte DisplayBrightness::CalcBrightnessAutomatic(byte brightness)
{
    const Illuminance& sensor = Illuminance::GetInstance();

    if(brightness < kBrightnessAutomaticMinValue) { return brightness; }

    return ToAutomaticLevel(brightness, ToIlluminanceFactor(sensor.GetIlluminance(),
                                                            sensor.GetCalibrationValuesMinValue(),
                                                            sensor.GetCalibrationValuesMaxValue()));
}

byte DisplayBrightness::CalcConfiguredBrightness() const
{
    if(use_gamma_correction_) {
        if(use_automatic_) { return CalcBrightnessAutomaticCorrected(); }
        else { return CalcBrightnessCorrected(); }
    } else {
        if(use_automatic_) { return CalcBrightnessAutomatic(); }
        else { return brightness_; }
    }
}

void DisplayBrightness::ResetToDefaults()
{
    brightness_ = kBrightnessInitValue;
    use_automatic_ = kUseAutomaticInitValue;
    use_gamma_correction_ = kUseGammaCorrectionInitValue;
    night_level_ = kFadeLevelFull;
}

byte DisplayBrightness::CalcBrightness() const
{
    byte value = CalcConfiguredBrightness();

    if(night_level_ != kFadeLevelFull) { value = ScaleByLevel(value, night_level_); }
    if(fade_level_ != kFadeLevelFull) { value = ScaleByLevel(value, fade_level_); }

    return value;
}
