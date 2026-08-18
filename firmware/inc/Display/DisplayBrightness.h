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
    /* The night switch's own scaling, kept apart from FadeLevel although it does the same
       arithmetic. A fade belongs to a running animation, which clears it when it ends -
       and an animation ending in the small hours would take the night dimming with it. */
    byte NightLevel{FadeLevelFull};
    bool UseAutomatic{UseAutomaticInitValue};
    bool UseGammaCorrection{UseGammaCorrectionInitValue};
    GammaCorrection GCorrection;

    static constexpr float AutomaticCorrectionFactor{1.0f};
    static constexpr byte BrightnessMaxValue{255};
    static constexpr byte BrightnessAutomaticMinValue{8u};
        
    // function
    /* The room's brightness, spread between the two calibrated bounds rather than divided
       by the upper one alone. Dividing left the lower bound with no effect at all - the
       procedure that calibrates it answered, Persistence stored it, and nothing ever read
       it - and it put a living room at 100 lx a sixth of a percent up a scale that ends at
       the sensor's 65535, so the automatic held every indoor clock at its floor. */
    byte calcBrightnessAutomatic(byte sBrightness) const {
        const Illuminance& Sensor = Illuminance::getInstance();

        if(sBrightness < BrightnessAutomaticMinValue) { return sBrightness; }

        return toAutomaticLevel(sBrightness, toIlluminanceFactor(Sensor.getIlluminance(),
                                                                Sensor.getCalibrationValuesMinValue(),
                                                                Sensor.getCalibrationValuesMaxValue()));
    }
    /* Where the reading sits between the bounds: 0 at the dark one, 1 at the bright one.
       Bounds that are not in order are what a calibration in the wrong order stores, and
       nothing rejects it - so they are answered with 1 here, which is the display the owner
       set and the same thing the automatic did before it had a lower bound at all. */
    static constexpr float toIlluminanceFactor(IlluminanceType Measured, IlluminanceType Dark, IlluminanceType Bright) {
        if(Bright <= Dark) { return 1.0f; }
        if(Measured <= Dark) { return 0.0f; }
        if(Measured >= Bright) { return 1.0f; }

        return static_cast<float>(Measured - Dark) / static_cast<float>(Bright - Dark);
    }
    /* Never darker than BrightnessAutomaticMinValue: a display the automatic took all the
       way down reads as a clock that has failed, not as a dark room. */
    static constexpr byte toAutomaticLevel(byte sBrightness, float IlluminanceFactor) {
        const byte Level = static_cast<byte>(sBrightness * IlluminanceFactor * AutomaticCorrectionFactor);

        return (Level < BrightnessAutomaticMinValue) ? BrightnessAutomaticMinValue : Level;
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
        NightLevel = FadeLevelFull;
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
        byte Value = calcConfiguredBrightness();

        if(NightLevel != FadeLevelFull) { Value = scaleByLevel(Value, NightLevel); }
        if(FadeLevel != FadeLevelFull) { Value = scaleByLevel(Value, FadeLevel); }

        return Value;
    }

    /* How dark a fade has made the display, 255 being not at all. Kept here rather than in
       the animation because applyBrightness() runs on every task and would otherwise undo
       what the animation wrote on the previous one. */
    byte getFadeLevel() const { return FadeLevel; }
    void setFadeLevel(byte Level) { FadeLevel = Level; }
    void clearFade() { FadeLevel = FadeLevelFull; }

    /* How dark the night switch has made the display, 255 being not at all. Scaling rather
       than replacing for the same reason a fade does: what is dimmed is the display as it
       was configured, so the brightness the owner set is still what daylight returns to and
       is still what Persistence stores. */
    byte getNightLevel() const { return NightLevel; }
    void setNightLevel(byte Level) { NightLevel = Level; }
    void clearNight() { NightLevel = FadeLevelFull; }
        
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
