// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief WS2812 waveform out of a PIO state machine, fed by DMA.
 *
 * The counterpart of the ESP32 backend's RMT channel and of the AVR-Dx backend's timer-and-CCL
 * arrangement: the pulse widths come out of hardware, and the processor only hands over a frame.
 *
 * Split out of Pixels for the same reason the AVR-Dx backend splits its WS2812 out - the buffer
 * logic above is identical on all three platforms, and keeping the waveform beside it would hide
 * that.
 *
 * A frame is handed to DMA and the call returns. Nothing here waits for the strip: 110 LEDs take
 * 3.3 ms, a third of the scheduler's tick, and blocking for that would push every other module's
 * timing around.
 */

#ifndef _WS2812_PIO_H_
#define _WS2812_PIO_H_

#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief The PIO block the strip's state machine runs in: 0 selects `pio0`, any other value `pio1`.
 *
 * One state machine of one block. The RP2350 has twelve of them across three blocks, so claiming
 * one costs nothing that anything else here wants.
 */
#define WS2812_PIO_BLOCK_INDEX                          0u
/*!
 * \brief The state machine of that block which sends the frames.
 *
 * WS2812Pio::Init() claims it from the SDK, so a library that asks the SDK for an unused state
 * machine of the block is handed another one, and one that claims this one first stops Init().
 */
#define WS2812_PIO_STATE_MACHINE_INDEX                  0u

/*!
 * \brief The strip's transmitter: a PIO state machine that makes the WS2812 waveform, fed a frame
 *        by DMA.
 *
 * There is one instance, GetInstance(). Init() loads the program and starts the state machine,
 * Transmit() starts a frame, and IsBusy() tells whether DMA is still feeding the last one. Pixels
 * is what calls it.
 *
 * A bit is 25 cycles of the state machine at 20 MHz, 1250 ns: high for 400 ns for a zero and for
 * 800 ns for a one.
 */
class WS2812Pio
{
  public:
    /*!
     * \brief A word of a frame: one LED.
     *
     * One LED per word rather than one colour per byte, because that is the shape the state
     * machine reads: the three bytes sit in bits 31..8 and are shifted out from the top, which is
     * the WS2812's most-significant-first order.
     */
    using WordType = uint32_t;

  private:
    /*
     * The three phases of one bit, in state machine cycles. A zero is high for Phase1 and
     * low for the other two; a one is high for Phase1 + Phase2 and low for Phase3. At the
     * clock below that is 400 ns and 800 ns of high time against a 1250 ns period - the
     * datasheet's nominal values, not the Pico SDK example's 375/750, or its 250/875 before
     * 2024. Those also work, but they sit away from the middle of the tolerance, and it is the
     * far end of a long strip with a slow edge that eats into what is left.
     */
    static constexpr uint32_t kPhase1Cycles{8u};
    static constexpr uint32_t kPhase2Cycles{8u};
    static constexpr uint32_t kPhase3Cycles{9u};
    static constexpr uint32_t kCyclesPerBit{kPhase1Cycles + kPhase2Cycles + kPhase3Cycles};

    // 50 ns a cycle, which makes all three phases whole numbers and keeps every delay
    // inside the four bits an instruction has for it once one bit is spent on side-set.
    static constexpr uint32_t kStateMachineClockHz{20u * 1000u * 1000u};

    static constexpr uint32_t kBitsPerPixel{24u};

    static_assert(kCyclesPerBit * (1000000000u / kStateMachineClockHz) == 1250u,
                  "WS2812Pio: the three phases do not add up to the WS2812's 1.25 us bit period");
    static_assert((kPhase1Cycles <= 16u) && (kPhase2Cycles <= 16u) && (kPhase3Cycles <= 16u),
                  "WS2812Pio: a delay of more than 16 cycles does not fit an instruction's four delay bits");

    bool is_ready_{false};
    int dma_channel_{-1};

  public:
    /*! \brief Returns the one transmitter. */
    static WS2812Pio& GetInstance() {
        static WS2812Pio singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Sets the transmitter up on the strip's data line `pin`.
     *
     * It claims state machine WS2812_PIO_STATE_MACHINE_INDEX of PIO block WS2812_PIO_BLOCK_INDEX,
     * loads the program into the block wherever the loader finds room, claims an unused DMA
     * channel, and starts the state machine with
     * `pin` as its side-set pin, made an output. The state machine shifts each word out to the
     * left, most significant bit first, pulls the next word after 24 bits, and has both FIFOs
     * joined for transmitting. Its clock is divided down from the system clock to 20 MHz.
     *
     * If the state machine is claimed already, the program does not fit into the block or no DMA
     * channel is free, it prints that on the serial line, gives back what it had claimed and
     * loaded, and gives up; IsReady() stays false. It reports and gives up rather than
     * retrying: the contract's Init() returns nothing, and a clock whose console still answers is
     * more useful for finding a miswired pin than one that reboots. Once it has succeeded, a
     * further call does nothing.
     *
     * \param[in] pin The GPIO of the strip's data line.
     */
    void Init(uint8_t pin);

    /*! \brief Returns whether Init() has set the transmitter up; false at first. */
    bool IsReady() const { return is_ready_; }

    /*!
     * \brief Returns whether DMA is still feeding the previous frame to the state machine.
     *
     * The caller has to check this before handing over the next one - two frames back to back
     * would lose the low gap the strip latches on. While the transmitter is not set up it answers
     * false.
     *
     * \return True while the DMA channel is still busy with a frame.
     * \note Not busy does not mean the frame has left the line: the DMA channel is done once it has
     *       written the last word into the state machine's FIFO, and the words in the FIFO still go
     *       out after that. A caller that sends at most one frame per tick, as Pixels does, keeps
     *       the frames apart.
     */
    bool IsBusy() const;

    /*!
     * \brief Starts sending one frame to the strip.
     *
     * Hands the frame to DMA and returns. DMA writes it into the state machine's FIFO a word at a
     * time, as fast as the state machine takes the words.
     *
     * It refuses rather than waits while DMA is still feeding the previous frame. The caller ticks
     * every 10 ms and a frame takes 3.3 ms, so a refusal here means something is wrong rather
     * than that the strip is busy - and waiting would spend a third of a tick doing nothing.
     *
     * \param[in] frame The words, one per LED, each with its three colour bytes in bits 31..8, in
     *                  the order the strip takes them.
     * \param[in] count The number of words.
     * \return kEOk, or kENotOk if the transmitter is not set up, `frame` is `nullptr`, `count` is 0
     *         or IsBusy() is true; nothing is then sent, and a frame on the wire goes on.
     * \pre `frame` outlives the transfer, which is why Pixels keeps a frame of its own rather than
     *      passing its pixel buffer.
     */
    StdReturnType Transmit(const WordType* frame, size_t count);
};

#endif // _WS2812_PIO_H_
