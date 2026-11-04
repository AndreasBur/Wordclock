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
/**     \file       Arduino.cpp
 *      \brief      Time base and console for the AVR Dx backend
 *
 *      \details    Both halves of the console are interrupt driven, and that is not a
 *                  refinement. Printing waits on the wire only when the queue is full, so an
 *                  answer does not stall the tick it was produced in; and receiving cannot
 *                  poll, because the console is read every tenth tick while the hardware
 *                  holds two bytes - at 115200 baud everything else typed would be lost.
 *
 *                  Both interrupt handlers are deliberately short, and the reason is the LED
 *                  strip rather than the console: the strip's own handler has to be served
 *                  within the reset gap, and every handler that can delay it counts against
 *                  that budget. See WS2812.cpp for the timing this has to stay inside.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <avr/interrupt.h>
#include <avr/io.h>

#include "Arduino.h"
#include "Bit.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/*
 * The asynchronous baud rate register, from the data sheet: 64 samples per bit over the
 * sixteen the receiver takes. Rounded rather than truncated, which keeps the error inside
 * the receiver's tolerance at the rates the console is used at.
 */
#define ARDUINO_BAUD_REGISTER(Baudrate)     static_cast<uint16_t>(((64uL * F_CPU) + (8uL * (Baudrate))) / (16uL * (Baudrate)))

// One interrupt per millisecond. At 24 MHz the reload stays inside the counter's 16 bits,
// so the time base needs no prescaler and keeps its full resolution.
#define ARDUINO_TIMEBASE_COMPARE_VALUE      static_cast<uint16_t>((F_CPU / 1000uL) - 1uL)

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
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

volatile char receive_buffer[ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE]{};
volatile uint16_t receive_head{0u};
volatile uint16_t receive_tail{0u};

volatile uint32_t milliseconds{0u};

} // namespace

/******************************************************************************************************************************************************
 *  I N T E R R U P T   H A N D L E R S
******************************************************************************************************************************************************/
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

    ARDUINO_SERIAL_USART.TXDATAL = static_cast<uint8_t>(transmit_buffer[transmit_tail]);
    transmit_tail = (transmit_tail + 1u) & kTransmitBufferMask;
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

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  InitRuntime()
******************************************************************************************************************************************************/
/*! \brief          Brings the clock and the millisecond time base up
 *  \details        The oscillator is set explicitly rather than left at its reset value:
 *                  the device comes up at 4 MHz, and every timing constant in this backend -
 *                  the baud rates, the time base, and above all the strip's pulse widths -
 *                  is computed from F_CPU. A mismatch between the two would not fail to
 *                  build, it would put wrong pulses on the wire.
******************************************************************************************************************************************************/
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
} /* InitRuntime */


/******************************************************************************************************************************************************
  millis()
******************************************************************************************************************************************************/
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
} /* millis */


/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  SerialPort::begin()
******************************************************************************************************************************************************/
void SerialPort::begin(uint32_t baudrate)
{
    ARDUINO_SERIAL_PORT.DIRSET = static_cast<uint8_t>(1u << ARDUINO_SERIAL_PIN_TXD);

    ARDUINO_SERIAL_USART.BAUD = ARDUINO_BAUD_REGISTER(baudrate);
    ARDUINO_SERIAL_USART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc | USART_SBMODE_1BIT_gc;
    ARDUINO_SERIAL_USART.CTRLA = USART_RXCIE_bm;
    ARDUINO_SERIAL_USART.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
} /* Begin */


/******************************************************************************************************************************************************
  SerialPort::write()
******************************************************************************************************************************************************/
/*! \brief          Queues one character for the wire
 *  \details        Waits only when the queue is full, and then only for as long as one
 *                  character takes. Printing from a task must not otherwise block: the tick
 *                  it runs in also feeds the LED strip.
******************************************************************************************************************************************************/
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
} /* Write */


/******************************************************************************************************************************************************
  SerialPort::print()
******************************************************************************************************************************************************/
void SerialPort::print(const char* text)
{
    if(text == nullptr) { return; }

    while(*text != '\0') {
        write(*text);
        text++;
    }
} /* print */


/******************************************************************************************************************************************************
  SerialPort::print()
******************************************************************************************************************************************************/
/*! \brief          Prints a string that lives in flash
 *  \details        The whole point of F(): read byte by byte out of program memory, so the
 *                  literal never occupies RAM. Every fixed word the command answers are
 *                  built from goes through here.
******************************************************************************************************************************************************/
void SerialPort::print(const __FlashStringHelper* text)
{
    if(text == nullptr) { return; }

    const char* address = reinterpret_cast<const char*>(text);

    for(char character = static_cast<char>(pgm_read_byte(address)); character != '\0';
        character = static_cast<char>(pgm_read_byte(++address))) {
        write(character);
    }
} /* print */


/******************************************************************************************************************************************************
  SerialPort::PrintNumber()
******************************************************************************************************************************************************/
void SerialPort::PrintNumber(int32_t number)
{
    char number_string[kNumberStringLength];

    ltoa(number, number_string, 10);
    print(number_string);
} /* PrintNumber */


/******************************************************************************************************************************************************
  SerialPort::PrintUnsignedNumber()
******************************************************************************************************************************************************/
void SerialPort::PrintUnsignedNumber(uint32_t number)
{
    char number_string[kNumberStringLength];

    ultoa(number, number_string, 10);
    print(number_string);
} /* PrintUnsignedNumber */


/******************************************************************************************************************************************************
  SerialPort::available()
******************************************************************************************************************************************************/
bool SerialPort::available() const
{
    return receive_head != receive_tail;
} /* available */


/******************************************************************************************************************************************************
  SerialPort::read()
******************************************************************************************************************************************************/
/*! \brief          Takes the next character out of the receive queue
 *  \details        Answers a null character when there is nothing queued, which is what the
 *                  core's reader treats as "nothing there" - it asks available() first, so
 *                  this only has to be defined, not meaningful.
******************************************************************************************************************************************************/
char SerialPort::read()
{
    if(receive_head == receive_tail) { return '\0'; }

    const char character = receive_buffer[receive_tail];
    receive_tail = (receive_tail + 1u) & kReceiveBufferMask;

    return character;
} /* Read */


/******************************************************************************************************************************************************
  SerialPort::flush()
******************************************************************************************************************************************************/
void SerialPort::flush()
{
    while(transmit_head != transmit_tail) {
        /* Waiting for the handler to empty the queue. */
    }

    while((ARDUINO_SERIAL_USART.STATUS & USART_TXCIF_bm) == 0u) {
        /* And for the last character to have left the shift register. */
    }
} /* Flush */
// NOLINTEND(readability-convert-member-functions-to-static)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
