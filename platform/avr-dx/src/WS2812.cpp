// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Pulse-shaping WS2812 transmitter for the AVR Dx, see WS2812.h.
 */

#include "WS2812.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "Arduino.h"
#include "Bit.h"

// Local to this file, so Doxygen leaves it out.
/*! \cond */
// The synchronous baud rate register: the host clock is the peripheral clock divided by
// twice this, in units of a sixty-fourth.
#define WS2812_BAUD_REGISTER static_cast<uint16_t>((64uL * F_CPU) / (2uL * WS2812_BIT_RATE_HZ))

// Peripheral clock cycles per pulse. Rounded to nearest, because at 24 MHz a cycle is
// already 42 ns and truncating both widths would move them a full step towards zero.
#define WS2812_CYCLES_PER_NS(Nanoseconds) \
  static_cast<uint16_t>(((F_CPU / 1000000uL) * (Nanoseconds) + 500uL) / 1000uL)

#define WS2812_ZERO_PULSE_CYCLES WS2812_CYCLES_PER_NS(WS2812_ZERO_PULSE_DURATION_NS)
#define WS2812_ONE_PULSE_CYCLES WS2812_CYCLES_PER_NS(WS2812_ONE_PULSE_DURATION_NS)

// out = IN0 ? IN2 : IN1, with IN0 the data line and IN1/IN2 the two one-shots. Written out
// rather than left as a bare 0xE4 so the table can be checked against the wiring below.
#define WS2812_LUT_TRUTH_TABLE 0xE4u

// The truth table is what turns two timers into one data line, so a wrong width is worth
// catching here rather than on a strip that only shows the wrong colour.
static_assert(WS2812_ZERO_PULSE_CYCLES > 0u, "WS2812: F_CPU is too low to time a zero pulse");
static_assert(WS2812_ONE_PULSE_CYCLES > WS2812_ZERO_PULSE_CYCLES,
              "WS2812: the two pulse widths collapsed into one");
static_assert(WS2812_CYCLES_PER_NS(WS2812_ZERO_PULSE_MAX_DURATION_NS) > WS2812_ZERO_PULSE_CYCLES,
              "WS2812: the zero pulse would be long enough to be read as a one");

namespace {
// What the interrupt is working through. Only ever set while the transmitter is idle, so
// the handler needs no guard against the frame changing under it.
byte const* volatile frame_data{nullptr};
uint16_t volatile frame_length{0u};
uint16_t volatile frame_position{0u};

}  // namespace

/*
 * Feeds the next pixel byte to the strip. Raised to the elevated interrupt level in Init(), so it
 * runs ahead of the console's handlers. That is what keeps the gap between two bytes inside the
 * strip's reset time: the pulse widths themselves come from hardware and do not care when this
 * runs, but a byte held back longer than the reset time makes the strip latch half a frame.
 */
ISR(USART0_DRE_vect) {
  if (frame_position >= frame_length) {
    // Disarmed rather than left to fire on an empty frame; the transmitter counts as
    // busy until the shift register has drained, which STATUS reports.
    Bit<uint8_t>::ClearBits(USART0.CTRLA, USART_DREIE_bm);
    return;
  }

  USART0.TXDATAL = frame_data[frame_position];
  frame_position++;
}
/*! \endcond */

/*
 * Puts USART0 into SPI host mode at the strip's bit rate. Master SPI mode rather than a plain
 * USART because the strip needs a clock edge per bit: that edge is what starts the one-shots, and
 * it is the reason the pulse widths need no software timing at all.
 *
 * Most significant bit first, because that is the order the WS2812 expects its colour bytes in.
 */
void WS2812::InitUsart() {
  // TXD and XCK are outputs; the data line the strip sees is neither of them, it is the
  // look-up table's own pin. TXD only has to reach the table, XCK the event system.
  PORTA.DIRSET = PIN0_bm | PIN2_bm;

  USART0.BAUD = WS2812_BAUD_REGISTER;
  USART0.CTRLC = USART_CMODE_MSPI_gc;
  USART0.CTRLB = USART_TXEN_bm;
}

/*
 * Routes the SPI clock to both one-shots. One channel for both timers, so the two pulses start on
 * the very same edge. Started apart they would drift against each other and the look-up table
 * would mux between two pulses that no longer share a rising edge.
 */
void WS2812::InitEventSystem() {
  EVSYS.CHANNEL0 = EVSYS_CHANNEL0_PORTA_PIN2_gc;
  EVSYS.USERTCB1CAPT = EVSYS_USER_CHANNEL0_gc;
  EVSYS.USERTCB2CAPT = EVSYS_USER_CHANNEL0_gc;
}

/*
 * Arms the two pulse widths. Single-shot mode: the event starts the counter, the output stays high
 * for the compare value and then falls. One timer per width is what replaces the E5's single XCL,
 * whose one-shot could carry both.
 */
void WS2812::InitOneShotTimers() {
  TCB1.CCMP = WS2812_ZERO_PULSE_CYCLES;
  TCB1.EVCTRL = TCB_CAPTEI_bm;
  TCB1.CTRLB = TCB_CNTMODE_SINGLE_gc;
  TCB1.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;

  TCB2.CCMP = WS2812_ONE_PULSE_CYCLES;
  TCB2.EVCTRL = TCB_CAPTEI_bm;
  TCB2.CTRLB = TCB_CNTMODE_SINGLE_gc;
  TCB2.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;
}

/*
 * Muxes the two pulses onto the data pin. The three inputs are peripherals rather than pins, which
 * is why the port around them stays free; the input positions are fixed by the device, and a
 * look-up table can only reach the USART and timers that carry its own index or the neighbouring
 * one.
 */
void WS2812::InitLookUpTable() {
  CCL.LUT0CTRLB = CCL_INSEL0_USART0_gc | CCL_INSEL1_TCB1_gc;
  CCL.LUT0CTRLC = CCL_INSEL2_TCB2_gc;
  CCL.TRUTH0 = WS2812_LUT_TRUTH_TABLE;
  CCL.LUT0CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;

  CCL.CTRLA = CCL_ENABLE_bm;
}

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 * The ones it would make const drive the hardware, which a const would deny.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
// NOLINTBEGIN(readability-make-member-function-const)
void WS2812::Init() {
  InitUsart();
  InitEventSystem();
  InitOneShotTimers();
  InitLookUpTable();

  /*
   * The one vector allowed to interrupt the others. The console's handlers are short, but
   * "short" is measured against the strip's reset time here, and this removes the question
   * rather than answering it per handler.
   */
  CPUINT.LVL1VEC = USART0_DRE_vect_num;
}

bool WS2812::IsBusy() const {
  if (frame_position < frame_length) {
    return true;
  }
  /*
   * Before the first frame there is no transmission to wait for. TXCIF is 0 after reset and
   * only a finished transmission sets it, so asked from the start it held every Send() off,
   * and the strip never got a frame.
   */
  if (frame_length == 0u) {
    return false;
  }

  return (USART0.STATUS & USART_TXCIF_bm) == 0u;
}

StdReturnType WS2812::Send(byte const* frame, uint16_t length) {
  if ((frame == nullptr) || (length == 0u)) {
    return kENotOk;
  }
  if (IsBusy()) {
    return kENotOk;
  }

  frame_data = frame;
  frame_length = length;
  frame_position = 0u;

  // Cleared by hand, because it is what IsBusy() reads to tell a drained shift register
  // from one that never started.
  USART0.STATUS = USART_TXCIF_bm;
  Bit<uint8_t>::SetBits(USART0.CTRLA, USART_DREIE_bm);

  return kEOk;
}
// NOLINTEND(readability-make-member-function-const)
// NOLINTEND(readability-convert-member-functions-to-static)
