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
 *      \brief      RP2350 WS2812 driver, driven by a PIO state machine
 *
 *      \details    Mirrors the public API of the simulator's Pixels so the shared
 *                  firmware core builds unchanged. The buffer layout is the same too,
 *                  and it already matches the strip: Pixel stores its colours in the
 *                  WS2812 wire order (see PIXEL_COLOR_OFFSET_GREEN in Pixel.h), so
 *                  going from buffer to frame costs a brightness scale and no
 *                  reordering.
 *
 *                  The waveform lives in WS2812Pio, not here, so the firmware core does
 *                  not pull the Pico SDK's PIO and DMA headers in through every module
 *                  that displays something.
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
       held - the first render then puts a defined frame on the wire. */
    PixelType PixelBuffer[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
    byte Pin{0};
    byte Brightness{255};
    /* Whether the firmware touched buffer or brightness since the last render. Same
       flag as the simulator's, and it earns its keep twice here: it saves the pass over
       all 110 cells on the ticks where nothing happened, and it is what makes render()
       the only place that transmits - see the reset-gap note in the source. Starts true
       so the first render establishes the strip's state. */
    bool Dirty{true};
    /* Whether anything may go out on the wire at all. Not the same as a brightness of zero,
       which still transmits a frame of black pixels: this is for the state in which the strip
       has no supply, and a frame arriving then pushes current into a dead rail through the
       LED's own protection diode. */
    bool OutputSuspended{false};

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
    /* Two forms of the reader, and the argument list says which: the one taking a reference
       answers whether the index is on the strip, the one taking none answers the pixel and
       leaves an unlit one where the index is not. Both check. Writers have a single form,
       which answers, and a caller that has nothing to do with the answer ignores it. */
    byte getBrightness() const { return Brightness; }
    StdReturnType getPixel(byte, PixelType&) const;
    PixelType getPixel(byte) const;
    StdReturnType getOutputPixel(byte Index, PixelType& Pixel) const {
        if(getPixel(Index, Pixel) == E_NOT_OK) { return E_NOT_OK; }

        Pixel.dimmPixel(Brightness);
        return E_OK;
    }
    PixelType getOutputPixel(byte Index) const {
        PixelType Pixel = getPixel(Index);
        Pixel.dimmPixel(Brightness);
        return Pixel;
    }

    // set methods
    void setPixels(PixelType);
    void setBrightness(byte, bool = false);
    StdReturnType setPixel(byte, PixelType);
    StdReturnType setPixel(byte, byte, byte, byte);
    StdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }

    // methods
    bool isIndexValid(IndexType Index) const { return Index < PIXELS_NUMBER_OF_PIXELS; }
    void init(byte sPin);
    void enablePixels() { setBrightness(255); }
    void disablePixels() { setBrightness(0); }
    void clearPixels() { setPixels(PixelType(0u, 0u, 0u)); }
    /* Only asks for a render, exactly as on the simulator. What reaches the strip is
       decided by render() on the next tick; see the source for why the transmission
       must not happen from here. */
    StdReturnType show() {
        Dirty = true;
        return E_OK;
    }

    // render interface, for the application's tick
    bool isDirty() const { return Dirty; }
    StdReturnType render();

    // output gate, for the supply switch
    /* Whether the last frame is still going out. Asked before the supply is cut: DMA reads the
       frame buffer long after the call that started it returned, so a port dropped on that
       tick lands in the middle of a frame. */
    bool isFrameOnTheWire() const;
    bool isOutputSuspended() const { return OutputSuspended; }
    void suspendOutput() { OutputSuspended = true; }
    /* Marks the buffer, because the LEDs lose their registers along with their supply: what
       was up there is gone, so coming back is a redraw rather than a resume. */
    void resumeOutput() { OutputSuspended = false; Dirty = true; }
};

#endif // _PIXELS_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
