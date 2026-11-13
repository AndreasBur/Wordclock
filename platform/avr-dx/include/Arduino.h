// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The core runtime this backend offers, on bare avr-libc.
 *
 * Named Arduino.h because that is the name the firmware core includes, but there is no Arduino core
 * behind it. The seam asks for a handful of types, the flash-string accessors and a Serial that can
 * print and be read; all of that is a few dozen lines on avr-libc, and pulling in a whole Arduino
 * core for it would cost far more than it saves.
 *
 * It also avoids the one thing that made the xmega build need a GCC plugin. The Arduino core
 * reaches Serial through Print and Stream, which are virtual, and avr-gcc puts every vtable in RAM
 * - avr-flash-vtbl exists to move them back to flash. SerialPort below declares no virtual function
 * at all, so there is no vtable to move and no plugin to keep in step with a compiler version.
 *
 * Serial is a macro rather than an object, as on the simulator, so that the singleton is
 * constructed on first use and no static initialisation order can get in front of it.
 */

#ifndef WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_ARDUINO_H_
#define WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_ARDUINO_H_

/*
 * Re-exported, every one of them: this header is the backend's Arduino API, and what includes it
 * wants - `byte` above all - is defined in them rather than here. Without the pragma an
 * include-cleaner credits those symbols to the inner header, finds nothing of this one used
 * directly, and offers to drop the include; taking that offer leaves the file resting on whatever
 * neighbour happens to include this header too.
 */
// IWYU pragma: begin_exports
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// IWYU pragma: end_exports

/*!
 * \brief The USART of the console, USART1.
 *
 * USART1 on PC0/PC1, because USART0 drives the LED strip - see WS2812.h.
 */
#define ARDUINO_SERIAL_USART USART1
/*! \brief The port of the console's pins. */
#define ARDUINO_SERIAL_PORT PORTC
#define ARDUINO_SERIAL_PIN_TXD 0u /*!< The pin of TXD, PC0. */

/*!
 * \brief The size of the transmit queue in characters, a power of two; it holds one less.
 *
 * What one answer may be while the wire drains. Answers are printed from a task and must not stall
 * it, so they are queued rather than waited on; the longest the command set produces is the help
 * text, and this is comfortably above it.
 */
#define ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE 256u
/*!
 * \brief The size of the receive queue in characters, a power of two; it holds one less.
 *
 * One command line, with room for the next to arrive while it is being parsed. The console is
 * polled every tenth tick, which at 115200 baud is far more than the hardware's own two bytes of
 * headroom - so the receiver has to buffer or it drops what was typed.
 */
#define ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE 128u

/*!
 * \brief The type of a string in flash, as Arduino's.
 *
 * Only declared, never defined: it exists so that a flash string and a RAM string are different
 * types and pick different print() overloads.
 */
class __FlashStringHelper;  // NOLINT(bugprone-reserved-identifier): the Arduino core's name

/*!
 * \brief Keeps a string literal in flash, as Arduino's F().
 *
 * `PSTR()` puts the literal into flash, and the result is a `const __FlashStringHelper*`, which
 * SerialPort::print() reads from there.
 *
 * \param[in] String The string literal.
 */
#define F(String) (reinterpret_cast<const __FlashStringHelper*>(PSTR(String)))

#ifndef bitRead
/*!
 * \brief Returns bit `bit` of `value`, 0 or 1, as Arduino's bitRead() does.
 *
 * \param[in] value The value.
 * \param[in] bit The number of the bit, counted from 0 at the least significant one.
 */
#define bitRead(value, bit) (((value) >> (bit)) & 0x01u)  // NOLINT(readability-identifier-naming)
#endif

/*!
 * \brief The serial port, as Arduino's Serial: the one SerialPort, on USART1.
 *
 * On Arduino, Serial is an object of the core; here it is a macro for SerialPort::GetInstance().
 */
#define Serial SerialPort::GetInstance()  // NOLINT(readability-identifier-naming)

// Here and below, a NOLINT keeps a name the Arduino core gives it: the firmware is written
// against that API, and these are its stand-ins.
/*! \brief An unsigned 8-bit number, as Arduino's byte. */
using byte = uint8_t;  // NOLINT(readability-identifier-naming)
/*! \brief A truth value, as Arduino's boolean. */
using boolean = bool;  // NOLINT(readability-identifier-naming)

/*!
 * \brief Brings the clock and the millisecond time base up, and enables interrupts.
 *
 * It sets the oscillator to F_CPU with no prescaler, so the peripheral clock is the oscillator,
 * and starts TCB0 as a periodic interrupt that counts millis() up once per millisecond. Has to run
 * before anything below is used, which is why main() calls it first. Arduino has no such function:
 * its core does this itself before the sketch runs.
 *
 * The oscillator is set explicitly rather than left at its reset value: the device comes up at
 * 4 MHz, and every timing constant in this backend - the baud rates, the time base, and above all
 * the strip's pulse widths - is computed from F_CPU. A mismatch between the two would not fail to
 * build, it would put wrong pulses on the wire. So the setting is derived from F_CPU, and an
 * `AVR_F_CPU` the oscillator does not offer - it offers 1, 2, 3, 4, 8, 12, 16, 20 and 24 MHz -
 * fails to build. Offering it is not enough, though: Timer.h wants a multiple of 2 MHz and
 * WS2812.cpp wants two pulse widths that still differ, which rules out 1, 2 and 3 MHz as well. 4
 * MHz is the lowest that builds.
 */
void InitRuntime();

/*!
 * \brief Returns the milliseconds since InitRuntime(), as Arduino's millis().
 *
 * Wraps after 49 days; every reader here takes a difference, which stays correct across the wrap
 * as long as it is read more often than that. The counter is read with interrupts off, which are
 * then restored to as they were.
 */
uint32_t millis();  // NOLINT(readability-identifier-naming)

/*!
 * \brief The serial console on USART1, with the functions of Arduino's Serial that the firmware
 *        calls.
 *
 * Both directions are interrupt driven. print() queues its characters and returns, and waits only
 * when the transmit queue is full, for as long as one character takes. What arrives waits in the
 * receive queue for read(); when that queue is full, the newest character is dropped.
 *
 * The overload set is the simulator's, plus the flash-string one that F() needs. byte and the
 * smaller integers reach PrintNumber() through the int overload by integral promotion, which is
 * what makes a byte print as a number here and on the simulator alike rather than as a character.
 *
 * Where it differs from Arduino's Serial: there is no Print or Stream behind it and no virtual
 * function, numbers are printed in decimal only, available() answers whether a character is
 * waiting rather than how many, and read() answers a null character rather than -1 when none is.
 *
 * \warning print() from an interrupt handler, or with interrupts off, never returns once the
 *          transmit queue is full: the handler that empties it cannot run.
 */
class SerialPort {
 private:
  static constexpr uint16_t kTransmitBufferSize{ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE};
  static constexpr uint16_t kReceiveBufferSize{ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE};

  // Long enough for the widest number any overload below prints, plus sign and
  // terminator.
  static constexpr byte kNumberStringLength{12u};

  SerialPort() {}
  ~SerialPort() = default;

  /*
   * Reads no member, and is still not static: made static it cost 72 bytes of flash, as
   * GCC then compiles it and the print() overloads built on it differently. The check is
   * answered where it is defined, in Arduino.cpp.
   */
  void write(char character);  // NOLINT(readability-identifier-naming)
  void PrintNumber(int32_t number);
  void PrintUnsignedNumber(uint32_t number);

 public:
  /*! \brief Returns the one serial port, which Serial names. */
  static SerialPort& GetInstance() {
    static SerialPort singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Opens the port at `baudrate`, with 8 data bits, no parity and 1 stop bit, as Arduino's
   *        begin().
   *
   * It makes TXD an output, sets the baud rate to the nearest step of the baud register, and
   * enables the transmitter, the receiver and the receive interrupt.
   *
   * \param[in] baudrate The baud rate. Nothing checks it: one too low for the 16 bits of the baud
   *                     register, below 1465 at 24 MHz, wraps.
   */
  void begin(uint32_t baudrate);  // NOLINT(readability-identifier-naming)

  /*!
   * \brief Queues a string that lives in flash, as F() makes one.
   *
   * The whole point of F(): read byte by byte out of program memory, so the literal never
   * occupies RAM. Every fixed word the command answers are built from goes through here.
   *
   * \param[in] text The string, or `nullptr`, which prints nothing.
   */
  void print(__FlashStringHelper const* text);  // NOLINT(readability-identifier-naming)
  /*!
   * \brief Queues a string in RAM, up to its terminator.
   *
   * \param[in] text The string, or `nullptr`, which prints nothing.
   */
  void print(char const* text);  // NOLINT(readability-identifier-naming)
  /*!
   * \brief Queues one character.
   *
   * \param[in] character The character.
   */
  void print(char character) { write(character); }  // NOLINT(readability-identifier-naming)
  /*!
   * \brief Queues `number` in decimal, with a minus sign if it is negative.
   *
   * \param[in] number The number.
   */
  void print(int number) { PrintNumber(number); }  // NOLINT(readability-identifier-naming)
  /*!
   * \brief Queues `number` in decimal.
   *
   * \param[in] number The number.
   */
  void print(unsigned int number) {  // NOLINT(readability-identifier-naming)
    PrintUnsignedNumber(number);
  }
  /*!
   * \brief Queues `number` in decimal, with a minus sign if it is negative.
   *
   * \param[in] number The number.
   */
  void print(long number) {  // NOLINT(readability-identifier-naming,runtime/int)
    PrintNumber(number);
  }
  /*!
   * \brief Queues `number` in decimal.
   *
   * \param[in] number The number.
   */
  void print(unsigned long number) {  // NOLINT(readability-identifier-naming,runtime/int)
    PrintUnsignedNumber(number);
  }

  /*! \brief Queues the end of a line: a carriage return and a line feed. */
  void println() {  // NOLINT(readability-identifier-naming)
    write('\r');
    write('\n');
  }
  /*!
   * \brief Queues a string that lives in flash, then the end of a line.
   *
   * \param[in] text The string, or `nullptr`, which prints the end of the line alone.
   */
  void println(__FlashStringHelper const* text) {  // NOLINT(readability-identifier-naming)
    print(text);
    println();
  }
  /*!
   * \brief Queues a string in RAM, then the end of a line.
   *
   * \param[in] text The string, or `nullptr`, which prints the end of the line alone.
   */
  void println(char const* text) {  // NOLINT(readability-identifier-naming)
    print(text);
    println();
  }
  /*!
   * \brief Queues `number` in decimal, then the end of a line.
   *
   * \param[in] number The number.
   */
  void println(int number) {  // NOLINT(readability-identifier-naming)
    print(number);
    println();
  }

  /*! \brief Returns whether a received character is waiting to be read. */
  bool available() const;  // NOLINT(readability-identifier-naming)

  /*!
   * \brief Takes the next character out of the receive queue.
   *
   * \return The character, or a null character when there is nothing queued. That is what the
   *         core's reader treats as "nothing there" - it asks available() first, so this only has
   *         to be defined, not meaningful.
   */
  char read();  // NOLINT(readability-identifier-naming)

  /*!
   * \brief Waits until the queue has reached the wire.
   *
   * Only for the paths that are about to stop the processor - a restart otherwise cuts its own
   * answer off mid-sentence. It waits for the queue to empty, then for the USART's transmit
   * complete flag `TXCIF`, which the transmit handler clears before every character, so that it
   * is set once the last one has left the shift register. With nothing sent yet, it returns at
   * once.
   *
   */
  void flush();  // NOLINT(readability-identifier-naming)
};

#endif  // WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_ARDUINO_H_
