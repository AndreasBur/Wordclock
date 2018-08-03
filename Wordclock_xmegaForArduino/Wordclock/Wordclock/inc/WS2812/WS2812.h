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

#define WS2812_GAMMA7_TABLE_NUMBER_OF_VALUES        16


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/
#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    #define WS2812_POS_ABS_RED(Red)             (Red + OffsetRed)
    #define WS2812_POS_ABS_GREEN(Green)         (Green + OffsetGreen)
    #define WS2812_POS_ABS_BLUE(Blue)           (Blue + OffsetBlue)
#else
    #define WS2812_POS_ABS_RED(Red)             (Red + WS2812_COLOR_OFFSET_RED)
    #define WS2812_POS_ABS_GREEN(Green)         (Green + WS2812_COLOR_OFFSET_GREEN)
    #define WS2812_POS_ABS_BLUE(Blue)           (Blue + WS2812_COLOR_OFFSET_BLUE)
#endif


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

    using Gamma7TableElementType = byte;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    byte PinMask;
    volatile byte* PortOutputRegister;
    byte Pixels[WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS];
    static const Gamma7TableElementType Gamma7Table[];

#if (WS2812_SUPPORT_DIMMING == STD_ON)
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
    byte calcGamma7CorrectionValue(byte);
    void sendData(const byte*, uint16_t);

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void dimmPixels(byte*, uint16_t);
    void dimmPixel(PixelType*, PixelType);
    void dimmPixel(PixelType*, byte, byte, byte);
    void dimmColor(byte* ColorDimmed, byte Color) const { *ColorDimmed = (Color * Brightness) >> 8; }
#endif

    Gamma7TableElementType getGamma7TableElement(byte Index) const {
        Gamma7TableElementType Gamma7TableElement;
        memcpy_P(&Gamma7TableElement, &Gamma7Table[Index], sizeof(Gamma7TableElementType));
        return Gamma7TableElement;
    }

#if (WS2812_RESET_TIMER == STD_ON)
    boolean isResetTimeElapsed() { return ((micros() - ResetTimer) > (WS2812_RESET_DURATION_NS / 1000) || ResetTimer == 0); }
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
    void setBrightness(byte sBrightness, boolean = false);
    stdReturnType setPin(byte);
    stdReturnType setPixel(byte, PixelType);
    stdReturnType setPixel(byte, byte, byte, byte);
    void setPixelFast(byte, PixelType);
    void setPixelFast(byte, byte, byte, byte);

    // methods
    void init(byte);
    void clearAllPixels() { memset(Pixels, 0, sizeof(Pixels)); }
    void setAllPixels(PixelType);
    void setAllPixels(byte, byte, byte);
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
