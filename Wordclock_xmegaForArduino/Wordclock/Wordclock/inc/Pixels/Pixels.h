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
/**     \file       Pixels.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _PIXELS_H_
#define _PIXELS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "WS2812.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Pixels configuration parameter */
#define PIXELS_SUPPORT_DIMMING              WS2812_SUPPORT_DIMMING
#define PIXELS_RESET_TIMER                  WS2812_RESET_TIMER
#define PIXELS_SUPPORT_DIMMING              WS2812_SUPPORT_DIMMING
#define PIXELS_NUMBER_OF_LEDS               WS2812_NUMBER_OF_LEDS

/* Pixels parameter */
#define PIXELS_IS_SINGLETON                 STD_ON

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   P I X E L S
******************************************************************************************************************************************************/
class Pixels
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using StateType = WS2812::StateType;
    using IndexType = WS2812::IndexType;
    using PortType = WS2812::PortType;
    using PortPinType = WS2812::PortPinType;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    WS2812& NeoPixels{WS2812::getInstance()};
    
    // functions
    Pixels() { }
    ~Pixels() { }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Pixels& getInstance() {
        static Pixels SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    StateType getState() const { return NeoPixels.getState(); }

    StdReturnType getPixel(IndexType Index, Pixel& sPixel) const { return NeoPixels.getPixel(Index, sPixel); }
    StdReturnType getPixelRed(IndexType Index, byte& Red) const { return NeoPixels.getPixelRed(Index, Red); }
    StdReturnType getPixelGreen(IndexType Index, byte& Green) const { return NeoPixels.getPixelGreen(Index, Green); }
    StdReturnType getPixelBlue(IndexType Index, byte& Blue) const { return NeoPixels.getPixelBlue(Index, Blue); }

    Pixel getPixelFast(IndexType Index) const { return NeoPixels.getPixelFast(Index); }
    byte getPixelRedFast(IndexType Index) const { return NeoPixels.getPixelRedFast(Index); }
    byte getPixelGreenFast(IndexType Index) const { return NeoPixels.getPixelGreenFast(Index); }
    byte getPixelBlueFast(IndexType Index) const { return NeoPixels.getPixelBlueFast(Index); }

#if (PIXELS_SUPPORT_DIMMING == STD_ON)
    byte getBrightness() const { return NeoPixels.getBrightness(); }
    StdReturnType getPixelDimmed(IndexType Index, Pixel& sPixel) const { return NeoPixels.getPixelDimmed(Index, sPixel); }
    Pixel getPixelDimmedFast(IndexType) const { return NeoPixels.getPixelDimmedFast(); }
#endif

    // set methods
    StdReturnType setPixel(IndexType Index, Pixel sPixel) { return NeoPixels.setPixel(Index, sPixel); }
    StdReturnType setPixel(IndexType Index, byte Red, byte Green, byte Blue) { return NeoPixels.setPixel(Index, Red, Green, Blue); }
    StdReturnType setPixelRed(IndexType Index, byte Red) { return NeoPixels.setPixelRed(Index, Red); }
    StdReturnType setPixelGreen(IndexType Index, byte Green) { return NeoPixels.setPixelGreen(Index, Green); }
    StdReturnType setPixelBlue(IndexType Index, byte Blue) { return NeoPixels.setPixelBlue(Index, Blue); }

    void setPixelFast(IndexType Index, Pixel sPixel) { NeoPixels.setPixelFast(Index, sPixel); }
    void setPixelFast(IndexType Index, byte Red, byte Green, byte Blue) { NeoPixels.setPixelFast(Index, Red, Green, Blue); }
    void setPixelRedFast(IndexType Index, byte Red) { NeoPixels.setPixelRed(Index, Red); }
    void setPixelGreenFast(IndexType Index, byte Green) { NeoPixels.setPixelRed(Index, Green); }
    void setPixelBlueFast(IndexType Index, byte Blue) { NeoPixels.setPixelRed(Index, Blue); }

#if (PIXELS_SUPPORT_DIMMING == STD_ON)
    void setBrightness(byte Brightness, bool GammaCorrection = false) { NeoPixels.setBrightness(Brightness, GammaCorrection); }
#endif

    // methods
    bool init(byte Pin) { return NeoPixels.init(Pin); }
    void init(PortType Port, PortPinType PortPin) { NeoPixels.init(Port, PortPin); }
    StdReturnType show() { return NeoPixels.show(); }
    bool isIndexValid(IndexType Index) { return NeoPixels.isIndexValid(Index); }
    void setPixels(Pixel sPixel) { NeoPixels.setPixels(sPixel); }
    void setPixels(byte Red, byte Green, byte Blue) { NeoPixels.setPixels(Red, Green, Blue); }
    StdReturnType clearPixel(IndexType Index) { return NeoPixels.clearPixel(Index); }
    void clearPixelFast(IndexType Index) { NeoPixels.clearPixelFast(Index); }
    void clearPixels() { NeoPixels.clearPixels(); }
    void enablePixels() { NeoPixels.enablePixels(); }
    void disablePixels() { NeoPixels.disablePixels(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
