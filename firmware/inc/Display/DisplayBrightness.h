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
    static constexpr byte BrightnessInitValue{255u};
    static constexpr bool UseAutomaticInitValue{false};
    static constexpr bool UseGammaCorrectionInitValue{false};
    /* What a fade scales the result by. Full means there is no fade, which is the state
       everything but a running AnimationFade is in. */
    static constexpr byte FadeLevelFull{255u};

    byte Brightness{BrightnessInitValue};
    byte FadeLevel{FadeLevelFull};
    bool UseAutomatic{UseAutomaticInitValue};
    bool UseGammaCorrection{UseGammaCorrectionInitValue};
    GammaCorrection GCorrection;

    static constexpr float AutomaticCorrectionFactor{1.0f};
    static constexpr byte BrightnessMaxValue{255};
    static constexpr byte BrightnessAutomaticMinValue{8u};
        
    // function
    byte calcBrightnessAutomatic(byte sBrightness) const {
        IlluminanceType Illuminance = Illuminance::getInstance().getIlluminance();
        IlluminanceType IlluminanceMax = Illuminance::getInstance().getCalibrationValuesMaxValue();

        if(IlluminanceMax == 0u || Illuminance >= IlluminanceMax) { return sBrightness; }

        float IlluminanceFactor = static_cast<float>(Illuminance) / IlluminanceMax;
        byte automaticBrightness = static_cast<byte>(sBrightness * IlluminanceFactor * AutomaticCorrectionFactor);

        if(sBrightness < BrightnessAutomaticMinValue) { return sBrightness; }
        if(automaticBrightness < BrightnessAutomaticMinValue) { return BrightnessAutomaticMinValue; }

        return automaticBrightness;
    }
    /* The same shape as Pixel's own dimming: a level of 255 leaves the value alone, and
       one less than that scales it down without ever reaching zero by rounding. */
    static constexpr byte scaleByLevel(byte Value, byte Level) {
        return static_cast<byte>((static_cast<uint16_t>(Value) * (Level + 1u)) >> 8u);
    }

    byte calcBrightnessCorrected() const { return GCorrection.calcCorrectedValue(Brightness); }
    byte calcBrightnessAutomatic() const { return calcBrightnessAutomatic(Brightness); }
    byte calcBrightnessAutomaticCorrected() const { return calcBrightnessAutomatic(GCorrection.calcCorrectedValue(Brightness)); }
        
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DisplayBrightness() { }
    ~DisplayBrightness() { }

	// get methods
    byte getBrightness() const { return Brightness; }
    byte calcConfiguredBrightness() const {
        if(UseGammaCorrection) {
            if(UseAutomatic) { return calcBrightnessAutomaticCorrected(); }
            else { return calcBrightnessCorrected(); }
        } else {
            if(UseAutomatic) { return calcBrightnessAutomatic(); }
            else { return Brightness; }
        }
    }
    bool getUseAutomatic() const { return UseAutomatic; }
    bool getUseGammaCorrection() const { return UseGammaCorrection; }

	// set methods
    void setBrightness(byte sBrightness) { Brightness = sBrightness; }
    void setUseAutomatic(bool sUseAutomatic) { UseAutomatic = sUseAutomatic; }
    void setUseGammaCorrection(bool sUseGammaCorrection) { UseGammaCorrection = sUseGammaCorrection; }

	// methods
    void resetToDefaults() {
        Brightness = BrightnessInitValue;
        UseAutomatic = UseAutomaticInitValue;
        UseGammaCorrection = UseGammaCorrectionInitValue;
    }

    void enableAutomatic() { UseAutomatic = true; }
    void disableAutomatic() { UseAutomatic = false; }
    void enableGammaCorrection() { UseGammaCorrection = true; }
    void disableGammaCorrection() { UseGammaCorrection = false; }
    void incrementBrightness() { if(Brightness < BrightnessMaxValue) { Brightness++; }}
    void decrementBrightness() { if(Brightness > 0u) { Brightness--; }}
        
    /* A fade scales whatever the setting, the gamma correction and the automatic arrived
       at, rather than replacing it: what fades out is the display as it was configured,
       and the automatic keeps following the room while it does. */
    byte calcBrightness() const {
        if(FadeLevel == FadeLevelFull) { return calcConfiguredBrightness(); }

        return scaleByLevel(calcConfiguredBrightness(), FadeLevel);
    }

    /* How dark a fade has made the display, 255 being not at all. Kept here rather than in
       the animation because applyBrightness() runs on every task and would otherwise undo
       what the animation wrote on the previous one. */
    byte getFadeLevel() const { return FadeLevel; }
    void setFadeLevel(byte Level) { FadeLevel = Level; }
    void clearFade() { FadeLevel = FadeLevelFull; }
        
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
