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
******************************************************************************************************************************************************/
#ifndef _WS2812_H_
#define _WS2812_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixel.h"
#include "GammaCorrection.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WS2812 configuration parameter */
#define WS2812_RESET_TIMER                          STD_OFF
#define WS2812_SUPPORT_DIMMING                      STD_OFF
#define WS2812_NUMBER_OF_LEDS                       110u


/* WS2812 parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   W S 2 8 1 2
******************************************************************************************************************************************************/
class WS2812
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum PortType {
        PORT_C = 3u,
        PORT_D = 4u,
    };

    enum PortPinType {
        PORT_PIN_0 = 0u,
        PORT_PIN_4 = 4u
    };

    enum StateType {
        STATE_UNINIT,
        STATE_BUSY,
        STATE_IDLE,
    };

    //using PixelType = Pixel::PixelType;
#if (WS2812_NUMBER_OF_LEDS < 255u)
    using IndexType = byte;
#else
    using IndexType = uint16_t;
#endif

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  private:
    using PixelsType = std::array<Pixel, WS2812_NUMBER_OF_LEDS>;
    using pPixelsType = PixelsType*;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    WS2812();
    ~WS2812();

    volatile StateType State;
    PixelsType PixelsBuffer1;
    PixelsType PixelsBuffer2;
    pPixelsType pCurrentFrame;
    pPixelsType pNextFrame;

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    GammaCorrection GCorrection;
    byte Brightness;
#endif

#if (WS2812_RESET_TIMER == STD_ON)
    unsigned long ResetTimer;
#endif

    // functions
    void initDma(USART_t*, byte);
    void initEventSystem(byte, byte);
    void initUsart(USART_t*);
    void initXcl(byte, byte);
    void initPort(PORT_t*);
  
    void switchPixelsBufferPointer();
    void copyCurrentFrameToNextFrame() { *pNextFrame = *pCurrentFrame; }
    void copyNextFrameToCurrentFrame() { *pCurrentFrame = *pNextFrame; }
    void showNextFrame();
    void showNextFrameDimmed();
    void startDmaTransfer(pPixelsType);
    bool isDmaBusy() { return EDMA.STATUS & (EDMA_CH0BUSY_bm | EDMA_CH0PEND_bm); }

#if (WS2812_RESET_TIMER == STD_ON)
    void startResetTimer() { ResetTimer = micros(); }
    bool isResetTimeElapsed();
#endif

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void dimmPixels(pPixelsType pPixels) const;
    Pixel dimmPixel(Pixel sPixel) const { return dimmPixel(Pixel.getRed(), Pixel.getGreen(), Pixel.getBlue()); }
    Pixel dimmPixel(byte, byte, byte) const;
    void copyNextFrameToCurrentFrameDimmed();
    
    byte dimmColor(byte Color) const {
        byte dimmedColor = (Color * Brightness) >> 8u;
        if(dimmedColor == 0u) return 1u;
        else return dimmedColor;
    }
#endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WS2812& getInstance() {
        static WS2812 SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    StateType getState() const { return State; }

    StdReturnType getPixel(IndexType, Pixel&) const;
    StdReturnType getPixelRed(IndexType, byte&) const;
    StdReturnType getPixelGreen(IndexType, byte&) const;
    StdReturnType getPixelBlue(IndexType, byte&) const;

    Pixel getPixelFast(IndexType Index) const { return (*pNextFrame)[Index]; }
    byte getPixelRedFast(IndexType Index) const { return (*pNextFrame)[Index].getRed(); }
    byte getPixelGreenFast(IndexType Index) const { return (*pNextFrame)[Index].getGreen(); }
    byte getPixelBlueFast(IndexType Index) const { return (*pNextFrame)[Index].getBlue(); }

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    byte getBrightness() const { return Brightness; }
    StdReturnType getPixelDimmed(IndexType, Pixel&) const;
    Pixel getPixelDimmedFast(IndexType) const;
#endif

    // set methods
    StdReturnType setPixel(IndexType, Pixel);
    StdReturnType setPixel(IndexType, byte, byte, byte);
    StdReturnType setPixelRed(IndexType, byte);
    StdReturnType setPixelGreen(IndexType, byte);
    StdReturnType setPixelBlue(IndexType, byte);

    void setPixelFast(IndexType Index, Pixel sPixel) { (*pNextFrame)[Index] = sPixel; }
    void setPixelFast(IndexType Index, byte Red, byte Green, byte Blue) { (*pNextFrame)[Index].setPixel(Red, Green, Blue); }
    void setPixelRedFast(IndexType Index, byte Red) { (*pNextFrame)[Index].setRed(Red); }
    void setPixelGreenFast(IndexType Index, byte Green) { (*pNextFrame)[Index].setGreen(Green); }
    void setPixelBlueFast(IndexType Index, byte Blue) { (*pNextFrame)[Index].setGreen(Blue); }

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void setBrightness(byte, bool = false);
#endif

    // methods
    bool init(byte);
    void init(PortType, PortPinType);
    StdReturnType show();
    static bool isIndexValid(IndexType Index) { return Index < WS2812_NUMBER_OF_LEDS; }
    void setPixels(Pixel Pixel) { setPixels(Pixel.getRed(), Pixel.getGreen(), Pixel.getBlue()); }
    void setPixels(byte, byte, byte);
    StdReturnType clearPixel(IndexType Index);
    void clearPixelFast(IndexType Index) { (*pNextFrame)[Index].clearPixel(); }
    void clearPixels() { for(IndexType Index = 0u; Index < WS2812_NUMBER_OF_LEDS; Index++) clearPixelFast(Index); }
    void enablePixels() { switchPixelsBufferPointer(); show(); }
    void disablePixels() { switchPixelsBufferPointer(); clearPixels(); startDmaTransfer(pNextFrame); }

    // friend functions
    friend void dmaIsr();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
