// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief ESP32 WS2812 driver, driven by the RMT peripheral.
 *
 * Mirrors the public API of the simulator's Pixels so the shared firmware core builds unchanged.
 * The buffer layout is the same too, and it already matches the strip: Pixel stores its colours in
 * the WS2812 wire order (see PIXEL_COLOR_OFFSET_GREEN in Pixel.h), so going from buffer to frame
 * costs a brightness scale and no reordering.
 *
 * The RMT handles and the timing live in the source, not here, so the firmware core does not pull
 * the ESP-IDF driver headers in through every module that displays something.
 */

#ifndef WORDCLOCK_PLATFORM_ESP32_INCLUDE_PIXELS_H_
#define WORDCLOCK_PLATFORM_ESP32_INCLUDE_PIXELS_H_

#include "StandardTypes.h"
#include "Pixel.h"

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
 * On the ESP32 the RMT peripheral sends the frame, from Render(), which the application calls once
 * per tick. Render() copies the buffer, dimmed and in the strip's colour order, into a frame of its
 * own and hands that to the RMT driver, which goes on reading it from its interrupt after the call
 * has returned.
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
  // held - the first render then puts a defined frame on the wire.
  PixelType pixel_buffer_[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]{};
  byte pin_{0};
  byte brightness_{255};
  /*
   * Whether the firmware touched buffer or brightness since the last render. Same
   * flag as the simulator's, and it earns its keep twice here: it saves the pass over
   * all 110 cells on the ticks where nothing happened, and it is what makes Render()
   * the only place that transmits - see the reset-gap note at Render(). Starts true
   * so the first render establishes the strip's state.
   */
  bool dirty_{true};
  /*
   * Whether anything may go out on the wire at all. Not the same as a brightness of zero,
   * which still transmits a frame of black pixels: this is for the state in which the strip
   * has no supply, and a frame arriving then pushes current into a dead rail through the
   * LED's own protection diode.
   */
  bool output_suspended_{false};

  static constexpr byte ToRow(byte index) { return index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }
  static constexpr byte ToColumn(byte index) { return index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS; }

  StdReturnType TransmitFrame();
  void FillFrameBuffer() const;

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
   * It claims an RMT transmit channel on `pin`, with an encoder that turns each bit into the
   * WS2812's pulse, and enables the channel. A failure is printed on the serial line, and
   * Render() then answers kENotOk.
   *
   * Failures are reported and left behind rather than thrown: the contract's Init() returns
   * nothing, and a clock whose serial console still answers is more useful for finding a
   * miswired pin than one that reboots. Render() then does nothing until a channel exists.
   *
   * \param[in] pin The pin of the strip's data line.
   */
  void Init(byte pin);

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
   * however many calls came before. Here Render() sends it, and Render() says why the
   * transmission must not happen from here.
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
   * Render() clears it once it has tried to hand the frame to the RMT driver, even when the
   * transmission failed. Without an RMT channel, and while the output is suspended, Render()
   * leaves it marked. It starts marked, so the first render establishes the strip's state.
   *
   * \return True while the buffer is marked.
   */
  bool IsDirty() const { return dirty_; }

  /*!
   * \brief Sends the buffer to the strip, if it is marked.
   *
   * The application calls this once per tick, after the firmware's pass - never Show() itself.
   * Two reasons, and the second is the one that bites: several modules call Display::Show()
   * within a single tick (DisplayManager, Animations, Text, Clock and the command parsers all
   * do), so transmitting from Show() would put frames back to back on the wire. WS2812 needs the
   * line low for longer than 280 us between frames, and losing that gap does not blank the
   * display - it shifts the next frame by a pixel, which reads as a wiring fault. Rendering once
   * per 10 ms tick leaves milliseconds of idle line.
   *
   * It waits for the previous frame to leave the wire first, for at most 50 ms, because the
   * driver still reads the frame until then. It then copies the buffer, dimmed by the master
   * brightness, into that frame and queues it with the RMT driver, which sends it after the
   * call has returned.
   *
   * \return kEOk if a frame went out or none was due - also while the output is suspended, which
   *         leaves the mark - or kENotOk if Init() did not claim an RMT channel, which leaves the
   *         mark too, or if the previous frame did not finish in time or the driver did not take
   *         the frame, which clear it.
   * \warning Without an RMT channel the mark is never cleared, so Power, which waits for an
   *          unmarked buffer before it cuts the supply, never cuts it.
   */
  StdReturnType Render();

  /*!
   * \brief Returns whether a frame is still going out on the strip's data line.
   *
   * Asked before the supply is cut: the RMT driver goes on feeding the frame to the peripheral
   * from its interrupt long after the call that queued it returned, so a port dropped on that
   * tick lands in the middle of a frame.
   *
   * It asks `rmt_tx_wait_all_done()`, the call Render() waits on, with a timeout of 0 ms: this is
   * a question and not a wait, and it is asked from a task that has a clock to keep.
   * `ESP_ERR_TIMEOUT` is therefore the ordinary "still busy" answer rather than a fault, and
   * every answer other than `ESP_OK` counts as a frame still going out.
   *
   * Without an RMT channel it answers false. A channel that was never opened has nothing going
   * out, which is not the same as what the driver would answer for a null handle - and
   * reporting a frame that cannot exist would stall the switch-off for good.
   *
   * \return True while a frame is still going out.
   */
  bool IsFrameOnTheWire() const;

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
   * For the supply switch. The LEDs lose their registers along with their supply, so what comes
   * back is a redraw.
   */
  void ResumeOutput() {
    output_suspended_ = false;
    dirty_ = true;
  }
};

#endif  // WORDCLOCK_PLATFORM_ESP32_INCLUDE_PIXELS_H_
