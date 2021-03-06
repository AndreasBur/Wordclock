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
/**     \file       WS2812.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
 *****************************************************************************************************************************************************/
#define _WS2812_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
 *****************************************************************************************************************************************************/
#include "WS2812.h"
#include <util/atomic.h>

/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* fixed cycles by inline assembler in send data function */
#define WS2812_ASM_FIXED_CYCLES_LOW         3
#define WS2812_ASM_FIXED_CYCLES_HIGH        6
#define WS2812_ASM_FIXED_CYCLES_TOTAL       10

/* calculate cycles to match the timing, if possible */
#define WS2812_ZERO_CYCLES                  (((F_CPU / 1000) * WS2812_ZERO_PULSE_DURATION_NS) / 1000000)
#define WS2812_ONE_CYCLES                   (((F_CPU / 1000) * WS2812_ONE_PULSE_DURATION_NS)  / 1000000) /* +500000 überdenken */
#define WS2812_TOTAL_CYCLES                 (((F_CPU / 1000) * WS2812_PERIOD_DURATION_NS)     / 1000000) /* +500000 überdenken */

/* W1 NOPs between start (rising edge) and falling edge low */
#define WS2812_ASM_W1_SIGNED                (WS2812_ZERO_CYCLES - WS2812_ASM_FIXED_CYCLES_LOW)
/* W2 NOPs between falling edge low and falling edge high */
#define WS2812_ASM_W2_SIGNED                (WS2812_ONE_CYCLES - WS2812_ASM_FIXED_CYCLES_HIGH - WS2812_ASM_W1_SIGNED)
/* W3 NOPs between falling edge high and end to complete loop */
#define WS2812_ASM_W3_SIGNED                (WS2812_TOTAL_CYCLES - WS2812_ASM_FIXED_CYCLES_TOTAL - WS2812_ASM_W1_SIGNED - WS2812_ASM_W2_SIGNED)

/* simple way to delete negative wait values */
#if (WS2812_ASM_W1_SIGNED > 0)
    #define WS2812_ASM_W1_NOPs              WS2812_ASM_W1_SIGNED
#else
    #define WS2812_ASM_W1_NOPs              0
#endif

#if (WS2812_ASM_W2_SIGNED > 0)
    #define WS2812_ASM_W2_NOPs              WS2812_ASM_W2_SIGNED
#else
    #define WS2812_ASM_W2_NOPs              0
#endif

#if (WS2812_ASM_W3_SIGNED > 0)
    #define WS2812_ASM_W3_NOPs              WS2812_ASM_W3_SIGNED
#else
    #define WS2812_ASM_W3_NOPs              0
#endif

/* the only critical timing parameter is the minimum pulse length of zero "0" 
   warn or throw error if this timing can not be met with current F_CPU settings. */
#define WS2812_ZERO_PULSE_DURATION_NS_CALC (((WS2812_ASM_W1_NOPs + WS2812_ASM_FIXED_CYCLES_LOW) * 1000000) / (F_CPU / 1000))


#if (WS2812_ZERO_PULSE_DURATION_NS_CALC > 550)
    #error "WS2812: sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif (WS2812_ZERO_PULSE_DURATION_NS_CALC > 450)
    #warning "WS2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
    #warning "Please consider a higher clockspeed, if possible"
#endif


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
const WS2812::Gamma7TableElementType Gamma7Table[] PROGMEM {133, 139, 145, 151, 158, 165, 172, 180, 188, 196, 205, 214, 224, 234, 244, 255};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of WS2812
******************************************************************************************************************************************************/
/*! \brief          WS2812 Constructor
 *  \details        Instantiation of the WS2812 library
 *
 *  \return         -
 *****************************************************************************************************************************************************/
WS2812::WS2812()
{
    Brightness = 255;
#if (WS2812_RESET_TIMER == STD_ON)
    ResetTimer = 0;
#endif
    clearAllPixels();
#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
    setColorOrder(COLOR_ORDER_BRG);
#endif
} /* WS2812 */


/******************************************************************************************************************************************************
  Destructor of WS2812
******************************************************************************************************************************************************/
WS2812::~WS2812()
{

} /* ~WS2812 */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::init(byte Pin)
{
    setPin(Pin);
    show();
} /* init */


/******************************************************************************************************************************************************
  setAllPixels()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setAllPixels(PixelType Pixel)
{
    for(byte Index = 0; Index < WS2812_NUMBER_OF_LEDS; Index++) {
        Pixels[WS2812_POS_ABS_RED(Index)] = Pixel.Red;
        Pixels[WS2812_POS_ABS_GREEN(Index)] = Pixel.Green;
        Pixels[WS2812_POS_ABS_BLUE(Index)] = Pixel.Blue;
    }
}


/******************************************************************************************************************************************************
  setAllPixels()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setAllPixels(byte Red, byte Green, byte Blue)
{
    for(byte Index = 0; Index < WS2812_NUMBER_OF_LEDS; Index++) {
        Pixels[WS2812_POS_ABS_RED(Index)] = Red;
        Pixels[WS2812_POS_ABS_GREEN(Index)] = Green;
        Pixels[WS2812_POS_ABS_BLUE(Index)] = Blue;
    }
}


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
#if (WS2812_RESET_TIMER == STD_ON)
StdReturnType WS2812::show()
#elif (WS2812_RESET_TIMER == STD_OFF)
void WS2812::show()
#endif
{
#if (WS2812_RESET_TIMER == STD_ON)
    if((micros() - ResetTimer) > (WS2812_RESET_DURATION_NS / 1000) || ResetTimer == 0) {
#endif
        if(Brightness != 255) {
            byte PixelsDimmed[WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS];
            dimmPixels(PixelsDimmed, WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS);
            sendData(PixelsDimmed, WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS);
        } else {
            sendData(Pixels, WS2812_NUMBER_OF_LEDS * WS2812_NUMBER_OF_COLORS);
        }
#if (WS2812_RESET_TIMER == STD_ON)
        ResetTimer = micros();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
#endif
} /* show */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::getPixel(byte Index, PixelType* Pixel) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Pixel->Red = Pixels[WS2812_POS_ABS_RED(Index)];
        Pixel->Green = Pixels[WS2812_POS_ABS_GREEN(Index)];
        Pixel->Blue = Pixels[WS2812_POS_ABS_BLUE(Index)];
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
WS2812::PixelType WS2812::getPixelFast(byte Index) const
{
    PixelType Pixel;
    
    Pixel.Red = Pixels[WS2812_POS_ABS_RED(Index)];
    Pixel.Green = Pixels[WS2812_POS_ABS_GREEN(Index)];
    Pixel.Blue = Pixels[WS2812_POS_ABS_BLUE(Index)];
    
    return Pixel;
} /* getPixelFast */


/******************************************************************************************************************************************************
  getPixelDimmed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::getPixelDimmed(byte Index, PixelType* Pixel) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        if(Brightness != 255) {
            dimmColor(&Pixel->Red, Pixels[WS2812_POS_ABS_RED(Index)]);
            dimmColor(&Pixel->Green, Pixels[WS2812_POS_ABS_GREEN(Index)]);
            dimmColor(&Pixel->Blue, Pixels[WS2812_POS_ABS_BLUE(Index)]);
        } else {
            getPixel(Index, Pixel);
        }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixelDimmed */


/******************************************************************************************************************************************************
  getPixelDimmedFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
WS2812::PixelType WS2812::getPixelDimmedFast(byte Index) const
{
    PixelType Pixel;

    if(Brightness == 255) {
        return getPixelFast(Index);
    } else {
        dimmColor(&Pixel.Red, Pixels[WS2812_POS_ABS_RED(Index)]);
        dimmColor(&Pixel.Green, Pixels[WS2812_POS_ABS_GREEN(Index)]);
        dimmColor(&Pixel.Blue, Pixels[WS2812_POS_ABS_BLUE(Index)]);
    }
    return Pixel;
} /* getPixelDimmedFast */


/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setBrightness(byte sBrightness, bool GammaCorrection)
{
    if(GammaCorrection) {
        Brightness = calcGamma7CorrectionValue(sBrightness / 2);
    } else {
        Brightness = sBrightness;
    }

} /* setBrightness */


/******************************************************************************************************************************************************
  setPin()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::setPin(byte Pin)
{
    if(Pin < NUM_DIGITAL_PINS) {
        PinMask = digitalPinToBitMask(Pin);
        PortOutputRegister = portOutputRegister(digitalPinToPort(Pin));
        pinMode(Pin, OUTPUT);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPin */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::setPixel(byte Index, PixelType Pixel)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Pixels[WS2812_POS_ABS_RED(Index)] = Pixel.Red;
        Pixels[WS2812_POS_ABS_GREEN(Index)] = Pixel.Green;
        Pixels[WS2812_POS_ABS_BLUE(Index)] = Pixel.Blue;
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setPixelFast(byte Index, PixelType Pixel)
{
    Pixels[WS2812_POS_ABS_RED(Index)] = Pixel.Red;
    Pixels[WS2812_POS_ABS_GREEN(Index)] = Pixel.Green;
    Pixels[WS2812_POS_ABS_BLUE(Index)] = Pixel.Blue;
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Pixels[WS2812_POS_ABS_RED(Index)] = Red;
        Pixels[WS2812_POS_ABS_GREEN(Index)] = Green;
        Pixels[WS2812_POS_ABS_BLUE(Index)] = Blue;
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    Pixels[WS2812_POS_ABS_RED(Index)] = Red;
    Pixels[WS2812_POS_ABS_GREEN(Index)] = Green;
    Pixels[WS2812_POS_ABS_BLUE(Index)] = Blue;
} /* setPixelFast */


/******************************************************************************************************************************************************
  setColorOrder()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
#if (WS2812_RGB_ORDER_ON_RUNTIME == STD_ON)
void WS2812::setColorOrder(ColorOrderType ColorOrder)
{
    if(ColorOrder == COLOR_ORDER_BRG) {
        OffsetBlue = 0;
        OffsetRed = 1;
        OffsetGreen = 2;
    }
    if(ColorOrder == COLOR_ORDER_GBR) {
        OffsetGreen = 0;
        OffsetRed = 1;
        OffsetBlue = 2;
    }
    if(ColorOrder == COLOR_ORDER_RGB) {
        OffsetRed = 0;
        OffsetGreen = 1;
        OffsetBlue = 2;
    }
} /* setColorOrder */
#endif


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  calcGammaCorrectionValue()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        calculates the gamma correction value with the help of a small table
 *                  and very efficient shift calculation
 *  param[in]       ValueLinear    7 bit unsigned (0..127)
 *  \return         exponential value (approx. 1.0443^x) (1..255)
 *****************************************************************************************************************************************************/
uint8_t WS2812::calcGamma7CorrectionValue(byte ValueLinear)
{
    Gamma7TableElementType Exponent = getGamma7TableElement(ValueLinear % WS2812_GAMMA7_TABLE_NUMBER_OF_VALUES);
    return Exponent >> (7 - ValueLinear / WS2812_GAMMA7_TABLE_NUMBER_OF_VALUES);
} /* calcGamma7CorrectionValue */


/******************************************************************************************************************************************************
  dimmPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
inline void WS2812::dimmPixel(PixelType* PixelDimmed, PixelType Pixel)
{
    dimmColor(&PixelDimmed->Red, Pixel.Red);
    dimmColor(&PixelDimmed->Green, Pixel.Green);
    dimmColor(&PixelDimmed->Blue, Pixel.Blue);
} /* dimmPixel */


/******************************************************************************************************************************************************
  dimmPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
inline void WS2812::dimmPixel(PixelType* PixelDimmed, byte Red, byte Green, byte Blue)
{
    dimmColor(&PixelDimmed->Red, Red);
    dimmColor(&PixelDimmed->Green, Green);
    dimmColor(&PixelDimmed->Blue, Blue);
} /* dimmPixel */


/******************************************************************************************************************************************************
  dimmPixels()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
inline void WS2812::dimmPixels(byte* PixelsDimmed, uint16_t DataLength)
{
    for(uint16_t i = 0; i < DataLength; i = i + WS2812_NUMBER_OF_COLORS)
    {
        dimmColor(&PixelsDimmed[WS2812_POS_ABS_RED(i)], Pixels[WS2812_POS_ABS_RED(i)]);
        dimmColor(&PixelsDimmed[WS2812_POS_ABS_GREEN(i)], Pixels[WS2812_POS_ABS_GREEN(i)]);
        dimmColor(&PixelsDimmed[WS2812_POS_ABS_BLUE(i)], Pixels[WS2812_POS_ABS_BLUE(i)]);
    }
} /* dimmPixels */
 

/******************************************************************************************************************************************************
  sendData()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void  WS2812::sendData(const byte* Data, uint16_t DataLength)
{
    uint8_t BitCounter;
    uint8_t PortMaskLow;
    uint8_t PortMaskHigh;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PortMaskLow = ~PinMask & *PortOutputRegister;
        PortMaskHigh = PinMask | *PortOutputRegister;

        for(uint16_t ByteCounter = 0; ByteCounter < DataLength; ByteCounter++)
        {
            asm volatile(

                "ldi   %[BitCounter], 8             \n\t"
                "loop%=:                            \n\t"
                "st    X, %[PortMaskHigh]           \n\t"    //  '1' [02] '0' [02] - re

#if IS_BIT_SET(WS2812_ASM_W1_NOPs, 0)
                STD_NOP_1
#endif
#if IS_BIT_SET(WS2812_ASM_W1_NOPs, 1)
                STD_NOP_2
#endif
#if IS_BIT_SET(WS2812_ASM_W1_NOPs, 2)
                STD_NOP_4
#endif
#if IS_BIT_SET(WS2812_ASM_W1_NOPs, 3)
                STD_NOP_8
#endif
#if IS_BIT_SET(WS2812_ASM_W1_NOPs, 4)
                STD_NOP_16
#endif

                "sbrs  %[CurrentByte],  7           \n\t"    //  '1' [04] '0' [03]
                "st    X, %[PortMaskLow]            \n\t"    //  '1' [--] '0' [05] - fe-low
                "lsl   %[CurrentByte]               \n\t"    //  '1' [05] '0' [06]

#if IS_BIT_SET(WS2812_ASM_W2_NOPs, 0)
                STD_NOP_1
#endif
#if IS_BIT_SET(WS2812_ASM_W2_NOPs, 1)
                STD_NOP_2
#endif
#if IS_BIT_SET(WS2812_ASM_W2_NOPs, 2)
                STD_NOP_4
#endif
#if IS_BIT_SET(WS2812_ASM_W2_NOPs, 3)
                STD_NOP_8
#endif
#if IS_BIT_SET(WS2812_ASM_W2_NOPs, 4)
                STD_NOP_16
#endif

                "brcc skipone%=                     \n\t"    //  '1' [+1] '0' [+2] -
                "st   X, %[PortMaskLow]             \n\t"    //  '1' [+3] '0' [--] - fe-high
                "skipone%=:                         \n\t"    //  '1' [+3] '0' [+2] -

#if IS_BIT_SET(WS2812_ASM_W3_NOPs, 0)
                STD_NOP_1
#endif
#if IS_BIT_SET(WS2812_ASM_W3_NOPs, 1)
                STD_NOP_2
#endif
#if IS_BIT_SET(WS2812_ASM_W3_NOPs, 2)
                STD_NOP_4
#endif
#if IS_BIT_SET(WS2812_ASM_W3_NOPs, 3)
                STD_NOP_8
#endif
#if IS_BIT_SET(WS2812_ASM_W3_NOPs, 4)
                STD_NOP_16
#endif

                "dec   %[BitCounter]                \n\t"    //  '1' [+4] '0' [+3]
                "brne  loop%=                       \n\t"    //  '1' [+5] '0' [+4]
                : [BitCounter]      "=&d" (BitCounter)
                : [CurrentByte]     "r"   (Data[ByteCounter]),
                  [Port]            "x"   (PortOutputRegister),
                  [PortMaskHigh]    "r"   (PortMaskHigh),
                  [PortMaskLow]     "r"   (PortMaskLow)
            );
        }
    }
} /* sendData */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/
 