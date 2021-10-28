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
/**     \file       DisplayColor.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_COLOR_H_
#define _DISPLAY_COLOR_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixel.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DisplayColor configuration parameter */
#define DISPLAY_COLOR_SUPPORT_DIMMING                   STD_ON

/* DisplayColor parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y   C O L O R
******************************************************************************************************************************************************/
class DisplayColor
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    Pixel Color{255, 255, 255};

#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    Pixel ColorDimmed{Color};
#endif

    // functions
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    byte dimmColor(byte Color, byte Brightness) const {
        byte dimmedColor = (Color * Brightness) >> 8u;
        if(dimmedColor == 0u) { return 1u; }
        else { return dimmedColor; }
    }
#endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    DisplayColor() { }
    ~DisplayColor() { }

	// get methods
    Pixel getColor() const { return Color; }
    byte getColorRed() const { return Color.getRed(); }
    byte getColorGreen() const { return Color.getGreen(); }
    byte getColorBlue() const { return Color.getBlue(); }
        
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    Pixel getColorDimmed() { return ColorDimmed; }
    byte getColorRedDimmed() { return ColorDimmed.getRed(); }
    byte getColorGreenDimmed() { return ColorDimmed.getGreen(); }
    byte getColorBlueDimmed() { return ColorDimmed.getBlue(); }
#endif

	// set methods
    void setColor(Pixel sColor) { Color = sColor; }
    void setColorRed(byte Red) { Color.setRed(Red); }
    void setColorGreen(byte Green) { Color.setGreen(Green); }
    void setColorBlue(byte Blue) { Color.setBlue(Blue); }

	// methods
    void incrementColorRed() { Color.incrementRed(); }
    void incrementColorGreen() { Color.incrementGreen(); }
    void incrementColorBlue() { Color.incrementBlue(); }
    
    void decrementColorRed() { Color.decrementRed(); }
    void decrementColorGreen() { Color.decrementGreen(); }
    void decrementColorBlue() { Color.decrementBlue(); }    
    
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
    void dimmColorRed(byte Brightness) { ColorDimmed.setRed(dimmColor(Color.getRed(), Brightness)); }
    void dimmColorGreen(byte Brightness) { ColorDimmed.setGreen(dimmColor(Color.getGreen(), Brightness)); }
    void dimmColorBlue(byte Brightness) { ColorDimmed.setBlue(dimmColor(Color.getBlue(), Brightness)); }
    
    void dimmColors(byte Brightness) {
        dimmColorRed(Brightness);
        dimmColorGreen(Brightness);
        dimmColorBlue(Brightness);
    }
#endif
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
