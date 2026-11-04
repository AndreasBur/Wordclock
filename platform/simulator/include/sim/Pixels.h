// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator pixel strip (replaces the WS2812 driver).
 *
 * Mirrors the public API of the hardware pixel driver - get/set/clear pixel, brightness, show -
 * over a plain buffer, and nothing else.
 *
 * Deliberately free of wxWidgets: the window that renders this buffer is PixelsFrame. Nothing on
 * the way from the firmware to a pixel constructs a window any more, which is what lets the tests
 * in tests/ drive Display and DisplayManager without a display.
 *
 * Serial still reaches wxWidgets through the Arduino shim, but SerialShim holds null controls until
 * it is attached and checks for them, so printing headless is a no-op rather than a crash.
 */

#ifndef PIXELS_H
#define PIXELS_H

#include "StandardTypes.h"
#include "Pixel.h"

#define PIXELS_NUMBER_OF_LEDS                           110
#define PIXELS_SUPPORT_DIMMING                          STD_ON
#define PIXELS_IS_SINGLETON                             STD_ON

#define PIXELS_DISPLAY_NUMBER_OF_ROWS                   10u
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS                11u
#define PIXELS_NUMBER_OF_PIXELS                         (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

class Pixels
{
  public:
    using PixelType = Pixel;
# if (PIXELS_NUMBER_OF_PIXELS < 255u)
    using IndexType = byte;
# else
    using IndexType = uint16_t;
# endif

  private:
    // Zero-initialised, so the strip starts dark rather than at whatever the memory
    // held - the hardware driver starts from a defined state too.
    PixelType pixel_buffer_[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
    byte pin_{0};
    byte brightness_{255};
    /*
     * Whether the firmware touched buffer or brightness since the last render. It saves
     * the window a pass over all 110 cells on the ticks where nothing happened, and it
     * is what lets a test see whether a redraw was attempted at all - which is exactly
     * what DisplayManager's word-set latch promises not to do. Starts true so the first
     * pass establishes the window's colours.
     */
    bool dirty_{true};
    /*
     * Whether anything may reach the strip at all. Separate from a brightness of zero,
     * which still sends a frame of black pixels: this is for the state in which the strip
     * has no supply, and a frame arriving then pushes current into a dead rail through the
     * LED's own protection diode. Starts false, because a strip that is powered is what
     * every backend comes up with.
     */
    bool output_suspended_{false};

    // functions
    static constexpr byte ToRow(byte index) { return index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
    static constexpr byte ToColumn(byte index) { return index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }

    Pixels() { }
    ~Pixels() { }

  public:
    static Pixels& GetInstance() {
        static Pixels singleton_instance;
        return singleton_instance;
    }

    // get methods
    /*
     * Two forms of the reader, and the argument list says which: the one taking a reference
     * answers whether the index is on the strip, the one taking none answers the pixel and
     * leaves an unlit one where the index is not. Both check. Writers have a single form,
     * which answers, and a caller that has nothing to do with the answer ignores it.
     */
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
    void Init(byte pin) { pin_ = pin; }
    void EnablePixels() { SetBrightness(255); }
    void DisablePixels() { SetBrightness(0); }
    void ClearPixels() { SetPixels(PixelType(0u, 0u, 0u)); }
    // On the hardware this hands the buffer to the strip. Here it only asks for a
    // render, which PixelsFrame picks up on the next tick.
    StdReturnType Show() {
        dirty_ = true;
        return kEOk;
    }

    // render interface, for PixelsFrame and the tests
    bool IsDirty() const { return dirty_; }
    void ClearDirty() { dirty_ = false; }

    // output gate, for the supply switch
    /*
     * Nothing is on a wire here, so a frame is never still going out. The hardware
     * backends ask their peripheral, and answering false is what lets the sequence that
     * waits for a frame be tested without one.
     */
    static constexpr bool IsFrameOnTheWire() { return false; }
    bool IsOutputSuspended() const { return output_suspended_; }
    void SuspendOutput() { output_suspended_ = true; }
    /*
     * Marks the buffer, so what comes back is a redraw and not the window's last state:
     * the LEDs lose their registers along with their supply, so on hardware there is
     * nothing left up there to keep showing.
     */
    void ResumeOutput() { output_suspended_ = false; dirty_ = true; }
};

#endif // PIXELS_H
