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
    byte Brightness{255u};
    bool UseAutomatic{false};
    bool UseGammaCorrection{false};
    GammaCorrection GCorrection;

    static constexpr float AutomaticCorrectionFactor{1.0f};
    static constexpr byte BrightnessMaxValue{255};
    /* Floor of the automatic. Without it a dark room scales the brightness down to
       zero and the clock disappears completely, which is never what is wanted from a
       clock; the automatic only dims, it does not switch off. Switching off is what a
       brightness of zero is for. */
    static constexpr byte BrightnessAutomaticMinValue{8u};
        
    // function
    byte calcBrightnessAutomatic(byte sBrightness) const {
        IlluminanceType Illuminance = Illuminance::getInstance().getIlluminance();
        IlluminanceType IlluminanceMax = Illuminance::getInstance().getCalibrationValuesMaxValue();

        /* A maximum of zero would divide by zero, and a reading above it would scale the
           brightness past its own value and overflow the cast. Both are reachable after
           a calibration run, so the sensor cannot brighten beyond what was asked for. */
        if(IlluminanceMax == 0u || Illuminance >= IlluminanceMax) { return sBrightness; }

        float IlluminanceFactor = static_cast<float>(Illuminance) / IlluminanceMax;
        byte automaticBrightness = static_cast<byte>(sBrightness * IlluminanceFactor * AutomaticCorrectionFactor);

        /* never below the floor, but never above what was asked for either: a brightness
           of zero has to stay off */
        if(sBrightness < BrightnessAutomaticMinValue) { return sBrightness; }
        if(automaticBrightness < BrightnessAutomaticMinValue) { return BrightnessAutomaticMinValue; }

        return automaticBrightness;
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
    bool getUseAutomatic() const { return UseAutomatic; }
    bool getUseGammaCorrection() const { return UseGammaCorrection; }

	// set methods
    void setBrightness(byte sBrightness) { Brightness = sBrightness; }
    void setUseAutomatic(bool sUseAutomatic) { UseAutomatic = sUseAutomatic; }
    void setUseGammaCorrection(bool sUseGammaCorrection) { UseGammaCorrection = sUseGammaCorrection; }

	// methods
    void enableAutomatic() { UseAutomatic = true; }
    void disableAutomatic() { UseAutomatic = false; }
    void enableGammaCorrection() { UseGammaCorrection = true; }
    void disableGammaCorrection() { UseGammaCorrection = false; }
    void incrementBrightness() { if(Brightness < BrightnessMaxValue) { Brightness++; }}
    void decrementBrightness() { if(Brightness > 0u) { Brightness--; }}
        
    byte calcBrightness() const {
        if(UseGammaCorrection) {
            if(UseAutomatic) { return calcBrightnessAutomaticCorrected(); }
            else { return calcBrightnessCorrected(); }
        } else {
            if(UseAutomatic) { return calcBrightnessAutomatic(); }
            else { return Brightness; }
        }
    }
        
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
