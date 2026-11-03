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
 *      \brief      ESP32 WS2812 driver, driven by the RMT peripheral
 *
 *      \details    Mirrors the public API of the simulator's Pixels so the shared
 *                  firmware core builds unchanged. The buffer layout is the same too,
 *                  and it already matches the strip: Pixel stores its colours in the
 *                  WS2812 wire order (see PIXEL_COLOR_OFFSET_GREEN in Pixel.h), so
 *                  going from buffer to frame costs a brightness scale and no
 *                  reordering.
 *
 *                  The RMT handles and the timing live in the source, not here, so
 *                  the firmware core does not pull the ESP-IDF driver headers in
 *                  through every module that displays something.
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
    PixelType pixel_buffer_[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
    byte pin_{0};
    byte brightness_{255};
    /* Whether the firmware touched buffer or brightness since the last render. Same
       flag as the simulator's, and it earns its keep twice here: it saves the pass over
       all 110 cells on the ticks where nothing happened, and it is what makes Render()
       the only place that transmits - see the reset-gap note in the source. Starts true
       so the first render establishes the strip's state. */
    bool dirty_{true};
    /* Whether anything may go out on the wire at all. Not the same as a brightness of zero,
       which still transmits a frame of black pixels: this is for the state in which the strip
       has no supply, and a frame arriving then pushes current into a dead rail through the
       LED's own protection diode. */
    bool output_suspended_{false};

    // functions
    static constexpr byte ToRow(byte index) { return index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
    static constexpr byte ToColumn(byte index) { return index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }

    StdReturnType TransmitFrame();
    void FillFrameBuffer() const;

    Pixels() { }
    ~Pixels() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Pixels& GetInstance() {
        static Pixels singleton_instance;
        return singleton_instance;
    }

    // get methods
    /* Two forms of the reader, and the argument list says which: the one taking a reference
       answers whether the index is on the strip, the one taking none answers the pixel and
       leaves an unlit one where the index is not. Both check. Writers have a single form,
       which answers, and a caller that has nothing to do with the answer ignores it. */
    byte GetBrightness() const { return brightness_; }
    StdReturnType GetPixel(byte, PixelType&) const;
    PixelType GetPixel(byte) const;
    StdReturnType GetOutputPixel(byte index, PixelType& pixel) const {
        if(GetPixel(index, pixel) == kENotOk) { return kENotOk; }

        pixel.DimmPixel(brightness_);
        return kEOk;
    }
    PixelType GetOutputPixel(byte index) const {
        PixelType pixel = GetPixel(index);
        pixel.DimmPixel(brightness_);
        return pixel;
    }

    // set methods
    void SetPixels(PixelType);
    void SetBrightness(byte, bool = false);
    StdReturnType SetPixel(byte, PixelType);
    StdReturnType SetPixel(byte, byte, byte, byte);
    StdReturnType ClearPixel(byte index) { return SetPixel(index, 0, 0, 0); }

    // methods
    static constexpr bool IsIndexValid(IndexType index) { return index < PIXELS_NUMBER_OF_PIXELS; }
    void Init(byte pin);
    void EnablePixels() { SetBrightness(255); }
    void DisablePixels() { SetBrightness(0); }
    void ClearPixels() { SetPixels(PixelType(0u, 0u, 0u)); }
    /* Only asks for a render, exactly as on the simulator. What reaches the strip is
       decided by Render() on the next tick; see the source for why the transmission
       must not happen from here. */
    StdReturnType Show() {
        dirty_ = true;
        return kEOk;
    }

    // render interface, for the application's tick
    bool IsDirty() const { return dirty_; }
    StdReturnType Render();

    // output gate, for the supply switch
    /* Whether the last frame is still going out. Asked before the supply is cut: the RMT
       peripheral reads the frame buffer by DMA long after the call that queued it returned,
       so a port dropped on that tick lands in the middle of a frame. */
    bool IsFrameOnTheWire() const;
    bool IsOutputSuspended() const { return output_suspended_; }
    void SuspendOutput() { output_suspended_ = true; }
    /* Marks the buffer, because the LEDs lose their registers along with their supply: what
       was up there is gone, so coming back is a redraw rather than a resume. */
    void ResumeOutput() { output_suspended_ = false; dirty_ = true; }
};

#endif // _PIXELS_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
