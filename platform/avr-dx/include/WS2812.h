// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Pulse-shaping WS2812 transmitter for the AVR Dx.
 *
 * The successor of the xmega E5 driver, and the same idea: the pulse widths are made in hardware,
 * so no cycle-counted code has to hold the timing and an interrupt cannot corrupt a bit.
 *
 * The E5 built it from one XCL - a timer whose one-shot carried both widths in PERCAPTL and CMPL,
 * and a LUT that muxed them against the data line. The AVR Dx has the LUT again, as CCL, but its
 * TCBs carry one compare value each. So the one XCL timer becomes two TCBs, one per width, and the
 * LUT picks between them:
 *
 * - USART0 (MSPI host): clocks the pixel bytes out at 800 kbit/s.
 * - XCK edge: starts both one-shots through the event system.
 * - TCB1: the short pulse, a zero bit.
 * - TCB2: the long pulse, a one bit.
 * - CCL LUT0: out = TXD ? TCB2 : TCB1, truth table 0xE4.
 *
 * What the E5 had and this does not is the DMA: no AVR Dx has one. The bytes are therefore fed by
 * an interrupt, one per 10 us. That is affordable precisely because the pulse shaping is in
 * hardware - a late byte only stretches the gap between bytes, it does not deform a pulse. The gap
 * is what has to be watched instead, see the reset time below.
 *
 * \attention The register set-up below is derived from the data sheet and compiles against the
 * device headers, but it has not been on an oscilloscope. Before trusting it, check on hardware:
 * the two pulse widths, that the data bit is stable when the one-shots fire (UCPHA), and whether
 * the CCL sees the TCB waveforms without their pin outputs enabled.
 */

#ifndef _WS2812_H_
#define _WS2812_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief The pin of PORTA that carries the strip's data line, PA3.
 *
 * The data line, driven by the CCL rather than by a port register: LUT0's own output pin. PA3 also
 * carries USART0's XDIR, which stays undriven because RS485 mode is never enabled - the
 * alternative, PA6, belongs to USART0's alternative pin block and would be just as free.
 *
 * \note No code reads it: the pin is the one LUT0's output comes out on, not a number passed down.
 */
#define WS2812_DATA_PIN                                 3u

/*!
 * \brief The width of the pulse of a zero bit, in nanoseconds, from the data sheet.
 *
 * Only the short pulse is critical: a zero that grows past about 550 ns is read as a one.
 */
#define WS2812_ZERO_PULSE_DURATION_NS                   350u
/*! \brief The width of the pulse of a one bit, in nanoseconds, from the data sheet. */
#define WS2812_ONE_PULSE_DURATION_NS                    900u
/*! \brief The longest pulse the strip still reads as a zero, in nanoseconds. */
#define WS2812_ZERO_PULSE_MAX_DURATION_NS               550u

/*! \brief The bit rate of the strip, one bit per 1.25 us; the strip's rate, not a choice. */
#define WS2812_BIT_RATE_HZ                              800000uL

/*!
 * \brief How long the line has to rest before the strip latches what it was sent, in microseconds.
 *
 * The gap between two bytes has to stay below this, or the frame is torn in half - which is the one
 * budget every interrupt handler in this backend is measured against. No code reads it.
 */
#define WS2812_RESET_DURATION_US                        50u

/*!
 * \brief The strip's transmitter: USART0, two one-shot timers and a look-up table, fed byte by byte
 *        from an interrupt.
 *
 * There is one instance, GetInstance(). Init() builds the transmitter, Send() starts a frame, and
 * IsBusy() tells whether the last one is still on the wire.
 */
class WS2812
{
  private:
    WS2812() { }
    ~WS2812() { }

    static void InitUsart();
    static void InitEventSystem();
    static void InitOneShotTimers();
    static void InitLookUpTable();

  public:
    /*! \brief Returns the one transmitter. */
    static WS2812& GetInstance() {
        static WS2812 singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Builds the transmitter out of USART, event system, timers and look-up table.
     *
     * It puts USART0 into SPI host mode at WS2812_BIT_RATE_HZ, most significant bit first, with
     * TXD on PA0 and XCK on PA2 as outputs. Event channel 0 carries XCK to TCB1 and TCB2, which
     * are single-shot timers on the peripheral clock, armed with the zero and the one pulse. LUT0
     * muxes the two pulses with TXD onto its output pin. The order matters in one place only: the
     * look-up table is enabled last, so that it never drives the strip from inputs whose timers are
     * not yet running.
     *
     * Then it raises the interrupt that feeds the bytes, `USART0_DRE`, to the elevated level
     * `CPUINT.LVL1VEC`, so that it runs ahead of the console's handlers and of every other one.
     */
    void Init();

    /*!
     * \brief Returns whether a frame is still going out.
     *
     * A frame started while one is running would interleave two of them, which is why Send() asks
     * this first. Both halves are needed: the queue can be empty while the last byte is still in
     * the shift register, and starting a frame then would run the two together without the reset
     * gap between them. So it is busy while bytes of the frame are left to feed, and until the
     * USART's transmit complete flag `TXCIF`, which Send() clears, is set again. Before the first
     * frame it is not busy: `TXCIF` is clear after reset, and there is no transmission to wait for.
     *
     * \return True while a frame is going out.
     * \note Not busy does not mean the strip has latched the frame: nothing here waits
     *       WS2812_RESET_DURATION_US after the last byte. A caller that sends at most one frame per
     *       tick, as Pixels does, keeps the frames apart.
     */
    bool IsBusy() const;

    /*!
     * \brief Starts sending one frame to the strip.
     *
     * Hands the frame over and returns as soon as the first byte is on its way; the rest follows
     * from the interrupt, one byte per empty data register, which takes about 3.3 ms for a full
     * display. The caller is free in the meantime, and has to be - the tick this runs in also has
     * a clock to keep.
     *
     * \param[in] frame The bytes, in the order the strip takes them.
     * \param[in] length The number of bytes.
     * \return kEOk, or kENotOk if `frame` is `nullptr`, `length` is 0 or IsBusy() is true; nothing
     *         is then sent, and a frame on the wire goes on.
     * \pre `frame` outlives the transmission, which is why the caller keeps it rather than passing
     *      a temporary, and does not change until IsBusy() is false.
     */
    StdReturnType Send(const byte* frame, uint16_t length);
};

#endif // _WS2812_H_
