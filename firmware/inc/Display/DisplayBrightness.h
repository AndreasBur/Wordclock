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
/**     \file       DisplayBrightness.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_BRIGHTNESS_H_
#define _DISPLAY_BRIGHTNESS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Illuminance.h"
#include "GammaCorrection.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Brightness configuration parameter */


/* Brightness parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y   B R I G H T N E S S
******************************************************************************************************************************************************/
class DisplayBrightness
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
     using IlluminanceType = Illuminance::IlluminanceType;
 
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* What a clock starts with, and what the reset puts back - named once so the two
       cannot drift apart. */
    static constexpr byte kBrightnessInitValue{255u};
    static constexpr bool kUseAutomaticInitValue{false};
    static constexpr bool kUseGammaCorrectionInitValue{false};
    /* What a fade scales the result by. Full means there is no fade, which is the state
       everything but a running AnimationFade is in. */
    static constexpr byte kFadeLevelFull{255u};

    byte brightness_{kBrightnessInitValue};
    byte fade_level_{kFadeLevelFull};
    /* The night switch's own scaling, kept apart from fade_level_ although it does the same
       arithmetic. A fade belongs to a running animation, which clears it when it ends -
       and an animation ending in the small hours would take the night dimming with it. */
    byte night_level_{kFadeLevelFull};
    bool use_automatic_{kUseAutomaticInitValue};
    bool use_gamma_correction_{kUseGammaCorrectionInitValue};
    GammaCorrection g_correction_;

    static constexpr float kAutomaticCorrectionFactor{1.0f};
    static constexpr byte kBrightnessMaxValue{255};
    static constexpr byte kBrightnessAutomaticMinValue{8u};
        
    // function
    /* The room's brightness, spread between the two calibrated bounds rather than divided
       by the upper one alone. Dividing left the lower bound with no effect at all - the
       procedure that calibrates it answered, Persistence stored it, and nothing ever read
       it - and it put a living room at 100 lx a sixth of a percent up a scale that ends at
       the sensor's 65535, so the automatic held every indoor clock at its floor. */
    static byte CalcBrightnessAutomatic(byte brightness) {
        const Illuminance& sensor = Illuminance::GetInstance();

        if(brightness < kBrightnessAutomaticMinValue) { return brightness; }

        return ToAutomaticLevel(brightness, ToIlluminanceFactor(sensor.GetIlluminance(),
                                                                sensor.GetCalibrationValuesMinValue(),
                                                                sensor.GetCalibrationValuesMaxValue()));
    }
    /* Where the reading sits between the bounds: 0 at the dark one, 1 at the bright one.
       Bounds that are not in order are what a calibration in the wrong order stores, and
       nothing rejects it - so they are answered with 1 here, which is the display the owner
       set and the same thing the automatic did before it had a lower bound at all. */
    static constexpr float ToIlluminanceFactor(IlluminanceType measured, IlluminanceType dark, IlluminanceType bright) {
        if(bright <= dark) { return 1.0f; }
        if(measured <= dark) { return 0.0f; }
        if(measured >= bright) { return 1.0f; }

        return static_cast<float>(measured - dark) / static_cast<float>(bright - dark);
    }
    /* Never darker than kBrightnessAutomaticMinValue: a display the automatic took all the
       way down reads as a clock that has failed, not as a dark room. */
    static constexpr byte ToAutomaticLevel(byte brightness, float illuminance_factor) {
        const byte level = static_cast<byte>(brightness * illuminance_factor * kAutomaticCorrectionFactor);

        return (level < kBrightnessAutomaticMinValue) ? kBrightnessAutomaticMinValue : level;
    }
    /* The same shape as Pixel's own dimming: a level of 255 leaves the value alone, and
       one less than that scales it down without ever reaching zero by rounding. */
    static constexpr byte ScaleByLevel(byte value, byte level) {
        return static_cast<byte>((static_cast<uint16_t>(value) * (level + 1u)) >> 8u);
    }

    byte CalcBrightnessCorrected() const { return g_correction_.CalcCorrectedValue(brightness_); }
    byte CalcBrightnessAutomatic() const { return CalcBrightnessAutomatic(brightness_); }
    byte CalcBrightnessAutomaticCorrected() const { return CalcBrightnessAutomatic(g_correction_.CalcCorrectedValue(brightness_)); }
        
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DisplayBrightness() { }
    ~DisplayBrightness() { }

	// get methods
    byte GetBrightness() const { return brightness_; }
    byte CalcConfiguredBrightness() const {
        if(use_gamma_correction_) {
            if(use_automatic_) { return CalcBrightnessAutomaticCorrected(); }
            else { return CalcBrightnessCorrected(); }
        } else {
            if(use_automatic_) { return CalcBrightnessAutomatic(); }
            else { return brightness_; }
        }
    }
    bool GetUseAutomatic() const { return use_automatic_; }
    bool GetUseGammaCorrection() const { return use_gamma_correction_; }

	// set methods
    void SetBrightness(byte brightness) { brightness_ = brightness; }
    void SetUseAutomatic(bool use_automatic) { use_automatic_ = use_automatic; }
    void SetUseGammaCorrection(bool use_gamma_correction) { use_gamma_correction_ = use_gamma_correction; }

	// methods
    void ResetToDefaults() {
        brightness_ = kBrightnessInitValue;
        use_automatic_ = kUseAutomaticInitValue;
        use_gamma_correction_ = kUseGammaCorrectionInitValue;
        night_level_ = kFadeLevelFull;
    }

    void EnableAutomatic() { use_automatic_ = true; }
    void DisableAutomatic() { use_automatic_ = false; }
    void EnableGammaCorrection() { use_gamma_correction_ = true; }
    void DisableGammaCorrection() { use_gamma_correction_ = false; }
    void IncrementBrightness() { if(brightness_ < kBrightnessMaxValue) { brightness_++; }}
    void DecrementBrightness() { if(brightness_ > 0u) { brightness_--; }}
        
    /* A fade scales whatever the setting, the gamma correction and the automatic arrived
       at, rather than replacing it: what fades out is the display as it was configured,
       and the automatic keeps following the room while it does. */
    byte CalcBrightness() const {
        byte value = CalcConfiguredBrightness();

        if(night_level_ != kFadeLevelFull) { value = ScaleByLevel(value, night_level_); }
        if(fade_level_ != kFadeLevelFull) { value = ScaleByLevel(value, fade_level_); }

        return value;
    }

    /* How dark a fade has made the display, 255 being not at all. Kept here rather than in
       the animation because ApplyBrightness() runs on every task and would otherwise undo
       what the animation wrote on the previous one. */
    byte GetFadeLevel() const { return fade_level_; }
    void SetFadeLevel(byte level) { fade_level_ = level; }
    void ClearFade() { fade_level_ = kFadeLevelFull; }

    /* How dark the night switch has made the display, 255 being not at all. Scaling rather
       than replacing for the same reason a fade does: what is dimmed is the display as it
       was configured, so the brightness the owner set is still what daylight returns to and
       is still what Persistence stores. */
    byte GetNightLevel() const { return night_level_; }
    void SetNightLevel(byte level) { night_level_ = level; }
    void ClearNight() { night_level_ = kFadeLevelFull; }
        
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
