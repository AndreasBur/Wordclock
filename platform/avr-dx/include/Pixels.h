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
 *      \brief      The LED matrix as the firmware core sees it
 *
 *      \details    The buffer the core writes into, and the brightness it is dimmed by. What
 *                  reaches the strip is decided in render(), which the application calls once
 *                  per tick - show() only marks the buffer as changed.
 *
 *                  Same split as the ESP32 backend, for the same reason: several modules call
 *                  show() within one tick, and transmitting from each of them would put
 *                  frames back to back with no reset gap between them.
 *
******************************************************************************************************************************************************/
#ifndef _PIXELS_H_
#define _PIXELS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Pixel.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Pixels configuration parameter */
#define PIXELS_NUMBER_OF_LEDS                           110
#define PIXELS_SUPPORT_DIMMING                          STD_ON
#define PIXELS_IS_SINGLETON                             STD_ON

#define PIXELS_DISPLAY_NUMBER_OF_ROWS                   10u
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS                11u
#define PIXELS_NUMBER_OF_PIXELS                         (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

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
    /* Zero-initialised, so the strip starts dark rather than at whatever the memory held -
       the first render then puts a defined frame on the wire. */
    PixelType PixelBuffer[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
    byte Pin{0};
    byte Brightness{255};
    /* Whether the firmware touched buffer or brightness since the last render. Starts true
       so the first render establishes the strip's state. */
    bool Dirty{true};
    bool Initialised{false};

    // functions
    static constexpr byte toRow(byte Index) { return Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
    static constexpr byte toColumn(byte Index) { return Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
    StdReturnType transmitFrame();
    void fillFrameBuffer() const;

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
    void init(byte sPin);
    void enablePixels() { setBrightness(255); }
    void disablePixels() { setBrightness(0); }
    void clearPixels() { setPixels(PixelType(0u, 0u, 0u)); }

    /* Only asks for a render; what reaches the strip is decided by render() on the next
       tick. */
    StdReturnType show() {
        Dirty = true;
        return E_OK;
    }

    // render interface, for the application's tick
    bool isDirty() const { return Dirty; }
    StdReturnType render();
};

#endif // _PIXELS_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
