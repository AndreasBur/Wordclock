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
/**     \file       WS2812.h
 *      \brief      Main header file of WS2812 library
 *
 *      \details    Arduino library to control WS2812 based LEDs
 *                  
 *****************************************************************************************************************************************************/
#ifndef _WS2812_H_
#define _WS2812_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "GammaCorrection.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* WS2812 configuration parameter */
#define WS2812_RESET_TIMER                          STD_OFF
#define WS2812_RGB_ORDER_ON_RUNTIME                 STD_OFF
#define WS2812_SUPPORT_DIMMING                      STD_ON
#define WS2812_NUMBER_OF_LEDS                       110

/* WS2812 parameter */
#define WS2812_NUMBER_OF_COLORS                     3
#define WS2812_COLOR_OFFSET_GREEN                   0
#define WS2812_COLOR_OFFSET_RED                     1
#define WS2812_COLOR_OFFSET_BLUE                    2

#define WS2812_NUMBER_OF_BITS_PER_COLOR             8
#define WS2812_NUMBER_OF_BITS_PER_LED               (WS2812_NUMBER_OF_COLORS * WS2812_NUMBER_OF_BITS_PER_COLOR)

/* Timing of WS2812 protocol */
#define WS2812_ZERO_PULSE_DURATION_NS               350
#define WS2812_ONE_PULSE_DURATION_NS                900
#define WS2812_PERIOD_DURATION_NS                   1250
#define WS2812_RESET_DURATION_NS                    50000


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *   C L A S S   W S 2 8 1 2
******************************************************************************************************************************************************/
class WS2812
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    /* type which describes the color order of the protocol */
    enum ColorOrderType {
        COLOR_ORDER_RGB,
        COLOR_ORDER_BRG,
        COLOR_ORDER_GBR
    };
#endif

    /* type which describes the structure of a pixel */
    struct PixelType {
        byte Red;
        byte Green;
        byte Blue;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    byte PinMask;
    volatile byte* PortOutputRegister;
    byte Pixels[WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS];


#if (WS2812_SUPPORT_DIMMING == STD_ON)
    GammaCorrection GCorrection;
    byte Brightness;
#endif

#if (WS2812_RESET_TIMER == STD_ON)
    unsigned long ResetTimer;
#endif

#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    byte OffsetRed;
    byte OffsetGreen;
    byte OffsetBlue;
#endif
    // functions
    void sendData(const byte*, uint16_t);

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void dimmPixels(byte*, uint16_t) const;
    void dimmPixel(PixelType*, PixelType) const;
    void dimmPixel(PixelType*, byte, byte, byte) const;
    void dimmColor(byte* ColorDimmed, byte Color) const { *ColorDimmed = (Color * Brightness) >> 8; }
#endif

#if (WS2812_RESET_TIMER == STD_ON)
    boolean isResetTimeElapsed();
#endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    WS2812();
    ~WS2812();

    // get methods
    stdReturnType getPixel(byte, PixelType*) const;
    PixelType getPixelFast(byte) const;
    
#if (WS2812_SUPPORT_DIMMING == STD_ON)
    byte getBrightness() const { return Brightness; }
    stdReturnType getPixelDimmed(byte, PixelType*) const;
    PixelType getPixelDimmedFast(byte) const;
#endif

    // set methods
    stdReturnType setPin(byte);
    stdReturnType setPixel(byte, PixelType);
    stdReturnType setPixel(byte, byte, byte, byte);
    void setPixelFast(byte, PixelType);
    void setPixelFast(byte, byte, byte, byte);
    
#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void setBrightness(byte sBrightness, boolean = false);
#endif

    // methods
    void init(byte);
    void clearPixels() { memset(Pixels, 0, sizeof(Pixels)); }
    void setPixels(PixelType Pixel) { setPixels(Pixel.Red, Pixel.Green, Pixel.Blue); }
    void setPixels(byte, byte, byte);
    stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
    void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

#if (WS2812_RESET_TIMER == STD_ON)
    stdReturnType show();
#elif (WS2812_RESET_TIMER == STD_OFF)
    void show();
#endif

#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    void setColorOrder(ColorOrderType);
#endif
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
