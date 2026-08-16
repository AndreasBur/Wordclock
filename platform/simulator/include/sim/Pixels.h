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
 *      \brief      Simulator pixel strip (replaces the WS2812 driver)
 *
 *      \details    Mirrors the public API of the hardware pixel driver - get/set/clear
 *                  pixel, brightness, show - over a plain buffer, and nothing else.
 *
 *                  Deliberately free of wxWidgets: the window that renders this buffer
 *                  is PixelsFrame. Nothing on the way from the firmware to a pixel
 *                  constructs a window any more, which is what lets the tests in tests/
 *                  drive Display and DisplayManager without a display.
 *
 *                  Serial still reaches wxWidgets through the Arduino shim, but
 *                  SerialShim holds null controls until it is attached and checks for
 *                  them, so printing headless is a no-op rather than a crash.
 *
******************************************************************************************************************************************************/
#ifndef PIXELS_H
#define PIXELS_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Pixel.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
#define PIXELS_NUMBER_OF_LEDS                           110
#define PIXELS_SUPPORT_DIMMING                          STD_ON
#define PIXELS_IS_SINGLETON                             STD_ON

#define PIXELS_DISPLAY_NUMBER_OF_ROWS                   10u
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS                11u
#define PIXELS_NUMBER_OF_PIXELS                         (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

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
    using PixelType = Pixel;
# if (PIXELS_NUMBER_OF_PIXELS < 255u)
    using IndexType = byte;
# else
    using IndexType = uint16_t;
# endif

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Zero-initialised, so the strip starts dark rather than at whatever the memory
       held - the hardware driver starts from a defined state too. */
    PixelType PixelBuffer[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
    byte Pin{0};
    byte Brightness{255};
    /* Whether the firmware touched buffer or brightness since the last render. It saves
       the window a pass over all 110 cells on the ticks where nothing happened, and it
       is what lets a test see whether a redraw was attempted at all - which is exactly
       what DisplayManager's word-set latch promises not to do. Starts true so the first
       pass establishes the window's colours. */
    bool Dirty{true};

    // functions
    static constexpr byte toRow(byte Index) { return Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
    static constexpr byte toColumn(byte Index) { return Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }

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
    byte getBrightness() const { return Brightness; }
    StdReturnType getPixel(byte, PixelType&) const;
    PixelType getPixelFast(byte) const;
    StdReturnType getOutputPixel(byte Index, PixelType& Pixel) const {
        if(getPixel(Index, Pixel) == E_NOT_OK) { return E_NOT_OK; }

        Pixel.dimmPixel(Brightness);
        return E_OK;
    }
    PixelType getOutputPixelFast(byte Index) const {
        PixelType Pixel = getPixelFast(Index);
        Pixel.dimmPixel(Brightness);
        return Pixel;
    }

    // set methods
    void setPixels(PixelType);
    void setBrightness(byte, bool = false);
    StdReturnType setPixel(byte, PixelType);
    StdReturnType setPixel(byte, byte, byte, byte);
    StdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
    void setPixelFast(byte, PixelType);
    void setPixelFast(byte, byte, byte, byte);
    void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

    // methods
    bool isIndexValid(IndexType Index) const { return Index < PIXELS_NUMBER_OF_PIXELS; }
    void init(byte sPin) { Pin = sPin; }
    void enablePixels() { setBrightness(255); }
    void disablePixels() { setBrightness(0); }
    void clearPixels() { setPixels(PixelType(0u, 0u, 0u)); }
    /* On the hardware this hands the buffer to the strip. Here it only asks for a
       render, which PixelsFrame picks up on the next tick. */
    StdReturnType show() {
        Dirty = true;
        return E_OK;
    }

    // render interface, for PixelsFrame and the tests
    bool isDirty() const { return Dirty; }
    void clearDirty() { Dirty = false; }
};

#endif // PIXELS_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
