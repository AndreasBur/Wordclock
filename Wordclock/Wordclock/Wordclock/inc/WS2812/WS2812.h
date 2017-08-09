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
 *****************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
 *****************************************************************************************************************************************************/
/* WS2812 configuration parameter */
#define WS2812_RESET_TIMER                  STD_ON
#define WS2812_RGB_ORDER_ON_RUNTIME         STD_ON
#define WS2812_NUMBER_OF_LEDS               110

/* WS2812 parameter */
#define WS2812_NUMBER_OF_COLORS             3
#define WS2812_COLOR_OFFSET_GREEN           0
#define WS2812_COLOR_OFFSET_RED             1
#define WS2812_COLOR_OFFSET_BLUE            2

#define WS2812_NUMBER_OF_BITS_PER_COLOR     8
#define WS2812_NUMBER_OF_BITS_PER_LED       (WS2812_NUMBER_OF_COLORS * WS2812_NUMBER_OF_BITS_PER_COLOR)

/* Timing of WS2812 protocol */
#define WS2812_ZERO_PULSE_DURATION_NS       350UL
#define WS2812_ONE_PULSE_DURATION_NS        900UL
#define WS2812_PERIOD_DURATION_NS           1250UL
#define WS2812_RESET_DURATION_NS            50000UL


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
 *****************************************************************************************************************************************************/
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
 *****************************************************************************************************************************************************/
/* type which describes the color order of the protocol */
#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
typedef enum {
    WS2812_COLOR_ORDER_RGB,
    WS2812_COLOR_ORDER_BRG,
    WS2812_COLOR_ORDER_GBR
} WS2812ColorOrderType;
#endif

/* type which describes the structure of a pixel */
typedef struct {
    byte Red;
    byte Green;
    byte Blue;
} WS2812PixelType;


/******************************************************************************************************************************************************
 *  CLASS  WS2812
 *****************************************************************************************************************************************************/
class WS2812
{
  private:
    byte PinMask;
    const volatile byte* PortOutputRegister;
    //volatile byte* PortModeRegister; /* wird im Original DDR enabled, wirklich  notwendig? */
    byte Pixels[WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS];
    byte Brightness;

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
    void dimmPixels(byte*, uint16_t);
    void dimmPixel(WS2812PixelType*, WS2812PixelType);
    void dimmPixel(WS2812PixelType*, byte, byte, byte);
    void dimmColor(byte* ColorDimmed, byte Color) { *ColorDimmed = (Color * Brightness) >> 8; }

  public:
    WS2812(byte);
    ~WS2812();

    // get methods
    byte getBrightness() { return Brightness; }
    stdReturnType getPixel(byte, WS2812PixelType*);
    stdReturnType getPixelDimmed(byte, WS2812PixelType*);
	WS2812PixelType getPixelFast(byte);
	WS2812PixelType getPixelDimmedFast(byte);

    // set methods
    void etBrightness(byte sBrightness) { Brightness = sBrightness; }
    stdReturnType setPin(byte);
    stdReturnType setPixel(byte, WS2812PixelType);
    stdReturnType setPixel(byte, byte, byte, byte);
	void setPixelFast(byte, WS2812PixelType);
	void setPixelFast(byte, byte, byte, byte);

    // methods
    void init();
    void clearAllPixels() { memset(Pixels, 0, sizeof(Pixels)); }
    stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
	void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

#if (WS2812_RESET_TIMER == STD_ON)
    stdReturnType show();
#elif (WS2812_RESET_TIMER == STD_OFF)
    void show();
#endif

#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    void setColorOrder(WS2812ColorOrderType);
#endif
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
