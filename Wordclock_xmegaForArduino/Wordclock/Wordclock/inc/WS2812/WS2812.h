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
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _WS2812_H_
#define _WS2812_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "NeoPixel.h"
#include "GammaCorrection.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WS2812 configuration parameter */
#define WS2812_RESET_TIMER                          STD_ON
#define WS2812_SUPPORT_DIMMING                      STD_ON
#define WS2812_NUMBER_OF_LEDS                       110


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
        PORT_C,
        PORT_D,
    };

    enum PinType {
        PIN_0,
        PIN_4
    };

    enum StateType {
        STATE_UNINIT,
        STATE_BUSY,
        STATE_IDLE,
    };

    using PixelType = NeoPixel::NeoPixelType;
#if (WS2812_NUMBER_OF_LEDS < 255)
    using IndexType = byte;
#else
    using IndexType = uint16_t;
#endif

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  private:
    using PixelsType = std::array<NeoPixel, WS2812_NUMBER_OF_LEDS>;
    using pPixelsType = PixelsType*;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
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
    void copyCurrentFrameToNextFrame() { memcpy(pNextFrame, pCurrentFrame, sizeof(PixelsType)); }
    void copyNextFrameToCurrentFrame() { memcpy(pCurrentFrame, pNextFrame, sizeof(PixelsType)); }
    void showNextFrame();
    void showNextFrameDimmed();
    void startDmaTransfer(pPixelsType);
    boolean isDmaBusy() { return EDMA.STATUS & (EDMA_CH0BUSY_bm | EDMA_CH0PEND_bm); }

#if (WS2812_RESET_TIMER == STD_ON)
    void startResetTimer() { ResetTimer = micros(); }
    boolean isResetTimeElapsed();
#endif

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void dimmPixels(pPixelsType pPixels) const;
    PixelType dimmPixel(PixelType Pixel) const { return dimmPixel(Pixel.Red, Pixel.Green, Pixel.Blue); }
    PixelType dimmPixel(byte, byte, byte) const;
    byte dimmColor(byte Color) const { return (Color * Brightness) >> 8; }
    void copyNextFrameToCurrentFrameDimmed();
#endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
	static WS2812& getInstance();

	// get methods
    StateType getState() const { return State; }

    stdReturnType getPixel(IndexType, PixelType&) const;
    stdReturnType getPixelRed(IndexType, byte&) const;
    stdReturnType getPixelGreen(IndexType, byte&) const;
    stdReturnType getPixelBlue(IndexType, byte&) const;

    PixelType getPixelFast(IndexType Index) const { return (*pNextFrame)[Index].getPixel(); }
    byte getPixelRedFast(IndexType Index) const { return (*pNextFrame)[Index].getRed(); }
    byte getPixelGreenFast(IndexType Index) const { return (*pNextFrame)[Index].getGreen(); }
    byte getPixelBlueFast(IndexType Index) const { return (*pNextFrame)[Index].getBlue(); }

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    byte getBrightness() const { return Brightness; }
    stdReturnType getPixelDimmed(IndexType, PixelType&) const;
    PixelType getPixelDimmedFast(IndexType) const;
#endif

	// set methods
    stdReturnType setPixel(IndexType, PixelType);
    stdReturnType setPixel(IndexType, byte, byte, byte);
    stdReturnType setPixelRed(IndexType, byte);
    stdReturnType setPixelGreen(IndexType, byte);
    stdReturnType setPixelBlue(IndexType, byte);

    void setPixelFast(IndexType Index, PixelType Pixel) { (*pNextFrame)[Index].setPixel(Pixel); }
    void setPixelFast(IndexType Index, byte Red, byte Green, byte Blue) { (*pNextFrame)[Index].setPixel(Red, Green, Blue); }
    void setPixelRedFast(IndexType Index, byte Red) { (*pNextFrame)[Index].setRed(Red); }
    void setPixelGreenFast(IndexType Index, byte Green) { (*pNextFrame)[Index].setGreen(Green); }
    void setPixelBlueFast(IndexType Index, byte Blue) { (*pNextFrame)[Index].setGreen(Blue); }

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    void setBrightness(byte sBrightness, boolean = false);
#endif

	// methods
    void init(PortType, PinType);
    stdReturnType show();
    void setPixels(PixelType Pixel) { setPixels(Pixel.Red, Pixel.Green, Pixel.Blue); }
    void setPixels(byte, byte, byte);
    stdReturnType clearPixel(IndexType Index);
    void clearPixelFast(IndexType Index) { (*pNextFrame)[Index].clearPixel(); }
    void clearPixels() { for(IndexType Index = 0; Index < WS2812_NUMBER_OF_LEDS; Index++) clearPixelFast(Index); }

    // friend functions
    friend void dmaIsr();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
