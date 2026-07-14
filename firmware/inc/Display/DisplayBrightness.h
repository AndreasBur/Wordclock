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
        
    // function
    byte calcBrightnessAutomatic(byte sBrightness) const { 
        IlluminanceType Illuminance = Illuminance::getInstance().getIlluminance();
        IlluminanceType IlluminanceMax = Illuminance::getInstance().getCalibrationValuesMaxValue();
        float IlluminanceFactor = static_cast<float>(Illuminance) / IlluminanceMax;
        return static_cast<byte>(sBrightness * IlluminanceFactor * AutomaticCorrectionFactor);
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
    bool getUseGammaCorrection() const { return UseAutomatic; }

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
