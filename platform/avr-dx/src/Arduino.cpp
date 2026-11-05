// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Time base and console for the AVR Dx backend.
 *
 * Both halves of the console are interrupt driven, and that is not a refinement. Printing waits on
 * the wire only when the queue is full, so an answer does not stall the tick it was produced in;
 * and receiving cannot poll, because the console is read every tenth tick while the hardware holds
 * two bytes - at 115200 baud everything else typed would be lost.
 *
 * Both interrupt handlers are deliberately short, and the reason is the LED strip rather than the
 * console: the strip's own handler has to be served within the reset gap, and every handler that
 * can delay it counts against that budget. See WS2812.cpp for the timing this has to stay inside.
 */

#include <avr/interrupt.h>
#include <avr/io.h>

#include "Arduino.h"
#include "Bit.h"

// Local to this file, so Doxygen leaves it out.
/*! \cond */
/*
 * The asynchronous baud rate register, from the data sheet: the register counts in sixty-fourths,
 * over the sixteen samples per bit the receiver takes. Rounded rather than truncated, which keeps
 * the error inside the receiver's tolerance at the rates the console is used at.
 */
#define ARDUINO_BAUD_REGISTER(Baudrate)     static_cast<uint16_t>(((64uL * F_CPU) + (8uL * (Baudrate))) / (16uL * (Baudrate)))

// One interrupt per millisecond. At 24 MHz the reload stays inside the counter's 16 bits,
// so the time base needs no prescaler and keeps its full resolution.
#define ARDUINO_TIMEBASE_COMPARE_VALUE      static_cast<uint16_t>((F_CPU / 1000uL) - 1uL)

namespace {
// Power of two, so the wrap is a mask rather than a division - these run inside the
// interrupt handlers the strip's timing budget is measured against.
static_assert((ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE & (ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE - 1u)) == 0u,
              "Arduino: the transmit buffer size has to be a power of two");
static_assert((ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE & (ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE - 1u)) == 0u,
              "Arduino: the receive buffer size has to be a power of two");

constexpr uint16_t kTransmitBufferMask{ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE - 1u};
constexpr uint16_t kReceiveBufferMask{ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE - 1u};

volatile char transmit_buffer[ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE]{};
volatile uint16_t transmit_head{0u};
volatile uint16_t transmit_tail{0u};
// Whether a character has been handed to the transmitter, so that flush() has one to wait for.
volatile bool transmit_started{false};

volatile char receive_buffer[ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE]{};
volatile uint16_t receive_head{0u};
volatile uint16_t receive_tail{0u};

volatile uint32_t milliseconds{0u};

} // namespace

// The time base. Nothing but a counter, so that it can never be the handler that pushes
// the strip past its reset gap.
ISR(TCB0_INT_vect)
{
    TCB0.INTFLAGS = TCB_CAPT_bm;
    milliseconds++;
}

// One character out of the queue per empty data register. Disarms itself when the queue
// runs dry, rather than firing on into an empty buffer.
ISR(USART1_DRE_vect)
{
    if(transmit_head == transmit_tail) {
        Bit<uint8_t>::ClearBits(ARDUINO_SERIAL_USART.CTRLA, USART_DREIE_bm);
        return;
    }

    // Cleared before every character, so that TXCIF says the last one has left the shift
    // register rather than that some character once did; flush() waits on it.
    ARDUINO_SERIAL_USART.STATUS = USART_TXCIF_bm;
    ARDUINO_SERIAL_USART.TXDATAL = static_cast<uint8_t>(transmit_buffer[transmit_tail]);
    transmit_tail = (transmit_tail + 1u) & kTransmitBufferMask;
    transmit_started = true;
}

/*
 * A full receive buffer drops the newest character rather than overwriting the oldest: what
 * is already queued is the beginning of a command, and keeping that lets the parser reject
 * a truncated line, while overwriting would hand it a line spliced from two.
 */
ISR(USART1_RXC_vect)
{
    const char character = static_cast<char>(ARDUINO_SERIAL_USART.RXDATAL);
    const uint16_t next_head = (receive_head + 1u) & kReceiveBufferMask;

    if(next_head != receive_tail) {
        receive_buffer[receive_head] = character;
        receive_head = next_head;
    }
}
/*! \endcond */

void InitRuntime()
{
    _PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc);
    // No prescaler, so the peripheral clock is the oscillator and F_CPU means what the
    // pulse widths assume it means.
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0u);

    TCB0.CCMP = ARDUINO_TIMEBASE_COMPARE_VALUE;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;

    sei();
}

uint32_t millis()
{
    uint32_t value;

    // Four bytes read against a one-byte-wide interrupt: without the guard a tick landing
    // mid-read returns a number that was never on the clock.
    const uint8_t stored_status = SREG;
    cli();
    value = milliseconds;
    SREG = stored_status;

    return value;
}

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void SerialPort::begin(uint32_t baudrate)
{
    ARDUINO_SERIAL_PORT.DIRSET = static_cast<uint8_t>(1u << ARDUINO_SERIAL_PIN_TXD);

    ARDUINO_SERIAL_USART.BAUD = ARDUINO_BAUD_REGISTER(baudrate);
    ARDUINO_SERIAL_USART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc;
    ARDUINO_SERIAL_USART.CTRLA = USART_RXCIE_bm;
    ARDUINO_SERIAL_USART.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

/*
 * Queues one character for the wire. Waits only when the queue is full, and then only for as long
 * as one character takes. Printing from a task must not otherwise block: the tick it runs in also
 * feeds the LED strip.
 */
void SerialPort::write(char character)
{
    const uint16_t next_head = (transmit_head + 1u) & kTransmitBufferMask;

    while(next_head == transmit_tail) {
        // The handler that drains the queue may be the one being waited for, so it has to
        // be able to run - a spin with interrupts disabled here would never end.
    }

    transmit_buffer[transmit_head] = character;
    transmit_head = next_head;

    Bit<uint8_t>::SetBits(ARDUINO_SERIAL_USART.CTRLA, USART_DREIE_bm);
}

void SerialPort::print(const char* text)
{
    if(text == nullptr) { return; }

    while(*text != '\0') {
        write(*text);
        text++;
    }
}

void SerialPort::print(const __FlashStringHelper* text)
{
    if(text == nullptr) { return; }

    const char* address = reinterpret_cast<const char*>(text);

    for(char character = static_cast<char>(pgm_read_byte(address)); character != '\0';
        character = static_cast<char>(pgm_read_byte(++address))) {
        write(character);
    }
}

void SerialPort::PrintNumber(int32_t number)
{
    char number_string[kNumberStringLength];

    ltoa(number, number_string, 10);
    print(number_string);
}

void SerialPort::PrintUnsignedNumber(uint32_t number)
{
    char number_string[kNumberStringLength];

    ultoa(number, number_string, 10);
    print(number_string);
}

bool SerialPort::available() const
{
    return receive_head != receive_tail;
}

char SerialPort::read()
{
    if(receive_head == receive_tail) { return '\0'; }

    const char character = receive_buffer[receive_tail];
    receive_tail = (receive_tail + 1u) & kReceiveBufferMask;

    return character;
}

void SerialPort::flush()
{
    while(transmit_head != transmit_tail) {
        /* Waiting for the handler to empty the queue. */
    }
    // Nothing sent yet, nothing to wait for: TXCIF is 0 after reset and would never come.
    if(!transmit_started) { return; }

    while((ARDUINO_SERIAL_USART.STATUS & USART_TXCIF_bm) == 0u) {
        /* And for the last character to have left the shift register. */
    }
}
// NOLINTEND(readability-convert-member-functions-to-static)
