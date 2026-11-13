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

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_PIXELS_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_PIXELS_H_

#include "Pixel.h"
#include "StandardTypes.h"

/*! \brief The number of LEDs of the strip. */
#define PIXELS_NUMBER_OF_LEDS 110
/*! \brief STD_ON: the strip has a master brightness, which Display can dim the display with. */
#define PIXELS_SUPPORT_DIMMING STD_ON
/*! \brief STD_ON: Display holds a reference to the one Pixels, Pixels::GetInstance(). */
#define PIXELS_IS_SINGLETON STD_ON

/*! \brief The number of rows of the buffer. */
#define PIXELS_DISPLAY_NUMBER_OF_ROWS 10u
/*! \brief The number of columns of the buffer. */
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS 11u
/*! \brief The number of pixels of the buffer, rows times columns. */
#define PIXELS_NUMBER_OF_PIXELS (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

/*!
 * \brief The LED strip as the core sees it: a buffer of pixels, the master brightness they are
 *        dimmed by on the way out, and the gate that keeps every frame off the strip while its
 *        supply is off.
 *
 * A pixel is addressed by its index on the strip, counted from 0. The writers write the buffer and
 * mark it to be sent, and send nothing: the application sends a marked buffer on its next tick.
 * The master brightness dims a pixel on its way out, as GetOutputPixel() reads it, so the buffer
 * keeps the colours as they were written.
 *
 * Two forms of the reader, and the argument list says which: the one taking a reference answers
 * whether the index is on the strip, the one taking none answers the pixel and leaves an unlit one
 * where the index is not. Both check. Writers have a single form, which answers, and a caller that
 * has nothing to do with the answer ignores it.
 *
 * The simulator has no strip. The application's tick has PixelsFrame render a marked buffer into
 * the window instead of sending it, and clear the mark with ClearDirty().
 */
class Pixels {
 public:
  /*! \brief A pixel of the strip. */
  using PixelType = Pixel;
  /*! \brief The index of a pixel on the strip: a byte, or a uint16_t for 255 pixels and more. */
#if (PIXELS_NUMBER_OF_PIXELS < 255u)
  using IndexType = byte;
#else
  using IndexType = uint16_t;
#endif

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

  static constexpr byte ToRow(byte index) { return index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
  static constexpr byte ToColumn(byte index) { return index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }

  Pixels() {}
  ~Pixels() = default;

 public:
  /*! \brief Returns the one strip. */
  static Pixels& GetInstance() {
    static Pixels singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns the master brightness, which dims the pixels on the way out; 255 at first. */
  byte GetBrightness() const { return brightness_; }

  /*!
   * \brief Gives the pixel at `index`, as it was written.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \param[out] pixel The pixel.
   * \return kEOk, or kENotOk if `index` is not on the strip; `pixel` is then left as it was.
   */
  StdReturnType GetPixel(byte index, PixelType& pixel) const;

  /*!
   * \brief Returns the pixel at `index`, as it was written.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \return The pixel, or an unlit one if `index` is not on the strip.
   */
  PixelType GetPixel(byte index) const;

  /*!
   * \brief Gives the pixel at `index` as it goes out: dimmed by the master brightness.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \param[out] pixel The pixel, dimmed.
   * \return kEOk, or kENotOk if `index` is not on the strip; `pixel` is then left as it was.
   */
  StdReturnType GetOutputPixel(byte index, PixelType& pixel) const {
    if (GetPixel(index, pixel) == kENotOk) {
      return kENotOk;
    }

    pixel.DimmPixel(brightness_);
    return kEOk;
  }

  /*!
   * \brief Returns the pixel at `index` as it goes out: dimmed by the master brightness.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \return The pixel, dimmed, or an unlit one if `index` is not on the strip.
   */
  PixelType GetOutputPixel(byte index) const {
    PixelType pixel = GetPixel(index);
    pixel.DimmPixel(brightness_);
    return pixel;
  }

  /*!
   * \brief Writes `pixel` to every pixel of the strip, and marks the buffer to be sent.
   *
   * \param[in] pixel The pixel.
   */
  void SetPixels(PixelType pixel);

  /*!
   * \brief Sets the master brightness the pixels are dimmed by on the way out, and marks the buffer
   *        to be sent.
   *
   * The buffer keeps its colours, so a brightness raised again shows them as they were.
   *
   * \param[in] brightness The master brightness: 255 keeps a pixel as it is, 0 turns it off.
   * \param[in] gamma_correction Not used.
   */
  void SetBrightness(byte brightness, bool gamma_correction = false);

  /*!
   * \brief Writes `pixel` at `index`, and marks the buffer to be sent.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \param[in] pixel The pixel.
   * \return kEOk, or kENotOk if `index` is not on the strip; the buffer is then left as it was.
   */
  StdReturnType SetPixel(byte index, PixelType pixel);

  /*!
   * \brief Writes the pixel of `red`, `green` and `blue` at `index`, and marks the buffer to be
   *        sent.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \param[in] red The red value.
   * \param[in] green The green value.
   * \param[in] blue The blue value.
   * \return kEOk, or kENotOk if `index` is not on the strip; the buffer is then left as it was.
   */
  StdReturnType SetPixel(byte index, byte red, byte green, byte blue);

  /*!
   * \brief Turns the pixel at `index` off, and marks the buffer to be sent.
   *
   * \param[in] index The index of the pixel, counted from 0.
   * \return kEOk, or kENotOk if `index` is not on the strip; the buffer is then left as it was.
   */
  StdReturnType ClearPixel(byte index) { return SetPixel(index, 0, 0, 0); }

  /*!
   * \brief Returns whether `index` is on the strip.
   *
   * \param[in] index The index.
   * \return True if `index` lies below PIXELS_NUMBER_OF_PIXELS.
   */
  static constexpr bool IsIndexValid(IndexType index) { return index < PIXELS_NUMBER_OF_PIXELS; }

  /*!
   * \brief Sets the strip up on the data line `pin`.
   *
   * There is no data line in the simulator: it only keeps `pin`.
   *
   * \param[in] pin The pin of the strip's data line.
   */
  void Init(byte pin) { pin_ = pin; }

  /*! \brief Sets the master brightness to 255, and marks the buffer to be sent. */
  void EnablePixels() { SetBrightness(255); }

  /*! \brief Sets the master brightness to 0, and marks the buffer to be sent. */
  void DisablePixels() { SetBrightness(0); }

  /*! \brief Turns every pixel off, and marks the buffer to be sent. */
  void ClearPixels() { SetPixels(PixelType(0u, 0u, 0u)); }

  /*!
   * \brief Marks the buffer to be sent.
   *
   * It sends nothing itself: the application sends a marked buffer on its next tick, once,
   * however many calls came before. Here it only asks for a render, which PixelsFrame picks up
   * on the next tick.
   *
   * \return kEOk on every backend.
   */
  StdReturnType Show() {
    dirty_ = true;
    return kEOk;
  }

  /*!
   * \brief Returns whether the buffer is marked to be sent: from a write, Show() or ResumeOutput(),
   *        until the application's tick has sent it.
   *
   * In the simulator, until PixelsFrame has rendered it and called ClearDirty(). It starts
   * marked, so the first render establishes the window's colours.
   *
   * \return True while the buffer is marked.
   */
  bool IsDirty() const { return dirty_; }

  /*!
   * \brief Clears the mark on the buffer, once it has been rendered.
   *
   * Only in the simulator, for PixelsFrame and the tests. PixelsFrame leaves the mark while the
   * output is suspended, so the buffer is rendered once the output is resumed.
   */
  void ClearDirty() { dirty_ = false; }

  /*!
   * \brief Returns whether a frame is still going out on the strip's data line; always false in the
   *        simulator.
   *
   * Nothing is on a wire here, so a frame is never still going out. The hardware backends ask
   * their peripheral, and answering false is what lets the sequence that waits for a frame be
   * tested without one.
   *
   * \return False.
   */
  static constexpr bool IsFrameOnTheWire() { return false; }

  /*! \brief Returns whether the output to the strip is suspended; false at first. */
  bool IsOutputSuspended() const { return output_suspended_; }

  /*!
   * \brief Stops every frame from going out to the strip, until ResumeOutput().
   *
   * For the supply switch. The buffer stays marked, so a frame owed is sent once the output is
   * resumed.
   */
  void SuspendOutput() { output_suspended_ = true; }

  /*!
   * \brief Lets frames go out to the strip again, and marks the buffer to be sent.
   *
   * For the supply switch. Marks the buffer, so what comes back is a redraw and not the
   * window's last state: the LEDs lose their registers along with their supply, so on hardware
   * there is nothing left up there to keep showing.
   */
  void ResumeOutput() {
    output_suspended_ = false;
    dirty_ = true;
  }
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_PIXELS_H_
