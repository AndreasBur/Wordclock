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
/**     \file       Arduino.h
 *      \brief      The core runtime this backend offers, on bare avr-libc
 *
 *      \details    Named Arduino.h because that is the name the firmware core includes, but
 *                  there is no Arduino core behind it. The seam asks for a handful of types,
 *                  the flash-string accessors and a Serial that can print and be read; all of
 *                  that is a few dozen lines on avr-libc, and pulling in a whole Arduino core
 *                  for it would cost far more than it saves.
 *
 *                  It also avoids the one thing that made the xmega build need a GCC plugin.
 *                  The Arduino core reaches Serial through Print and Stream, which are
 *                  virtual, and avr-gcc puts every vtable in RAM - avr-flash-vtbl exists to
 *                  move them back to flash. SerialPort below declares no virtual function at
 *                  all, so there is no vtable to move and no plugin to keep in step with a
 *                  compiler version.
 *
 *                  Serial is a macro rather than an object, as on the simulator, so that the
 *                  singleton is constructed on first use and no static initialisation order
 *                  can get in front of it.
 *
******************************************************************************************************************************************************/
#ifndef _ARDUINO_H_
#define _ARDUINO_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Arduino configuration parameter */
/* USART1 on PC0/PC1, because USART0 drives the LED strip - see WS2812.h. */
#define ARDUINO_SERIAL_USART                            USART1
#define ARDUINO_SERIAL_PORT                             PORTC
#define ARDUINO_SERIAL_PIN_TXD                          0u

/* What one answer may be while the wire drains. Answers are printed from a task and must
   not stall it, so they are queued rather than waited on; the longest the command set
   produces is the help text, and this is comfortably above it. */
#define ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE             256u
/* One command line, with room for the next to arrive while it is being parsed. The console
   is polled every tenth tick, which at 115200 baud is far more than the hardware's own two
   bytes of headroom - so the receiver has to buffer or it drops what was typed. */
#define ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE              128u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/
/* Only declared, never defined: it exists so that a flash string and a RAM string are
   different types and pick different print() overloads. */
class __FlashStringHelper;

#define F(String)               (reinterpret_cast<const __FlashStringHelper*>(PSTR(String)))

#ifndef bitRead
#define bitRead(value, bit)     (((value) >> (bit)) & 0x01u) // NOLINT(readability-identifier-naming)
#endif

#define Serial                  SerialPort::GetInstance() // NOLINT(readability-identifier-naming)

/******************************************************************************************************************************************************
 *  G L O B A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
/* Here and below, a NOLINT keeps a name the Arduino core gives it: the firmware is written
   against that API, and these are its stand-ins. */
using byte = uint8_t; // NOLINT(readability-identifier-naming)
using boolean = bool; // NOLINT(readability-identifier-naming)

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N S
******************************************************************************************************************************************************/
/* Starts the millisecond time base and enables interrupts. Has to run before anything
   below is used, which is why main() calls it first. */
void InitRuntime();

/* Milliseconds since initRuntime(). Wraps after 49 days; every reader here takes a
   difference, which stays correct across the wrap as long as it is read more often than
   that. */
uint32_t millis(); // NOLINT(readability-identifier-naming)

/******************************************************************************************************************************************************
 *  C L A S S   S E R I A L   P O R T
******************************************************************************************************************************************************/
class SerialPort
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr uint16_t kTransmitBufferSize{ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE};
    static constexpr uint16_t kReceiveBufferSize{ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE};

    /* Long enough for the widest number any overload below prints, plus sign and
       terminator. */
    static constexpr byte kNumberStringLength{12u};

    SerialPort() { }
    ~SerialPort() { }

    /* Reads no member, and is still not static: made static it cost 72 bytes of flash, as
       GCC then compiles it and the print() overloads built on it differently. The check is
       answered where it is defined, in Arduino.cpp. */
    void write(char); // NOLINT(readability-identifier-naming)
    void PrintNumber(int32_t);
    void PrintUnsignedNumber(uint32_t);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static SerialPort& GetInstance() {
        static SerialPort singleton_instance;
        return singleton_instance;
    }

    void begin(uint32_t baudrate); // NOLINT(readability-identifier-naming)

    /* The overload set is the simulator's, plus the flash-string one that F() needs.
       byte and the smaller integers reach printNumber() through the int overload by
       integral promotion, which is what makes a byte print as a number here and on the
       simulator alike rather than as a character. */
    void print(const __FlashStringHelper*); // NOLINT(readability-identifier-naming)
    void print(const char*); // NOLINT(readability-identifier-naming)
    void print(char character) { write(character); } // NOLINT(readability-identifier-naming)
    void print(int number) { PrintNumber(number); } // NOLINT(readability-identifier-naming)
    void print(unsigned int number) { PrintUnsignedNumber(number); } // NOLINT(readability-identifier-naming)
    void print(long number) { PrintNumber(number); } // NOLINT(readability-identifier-naming)
    void print(unsigned long number) { PrintUnsignedNumber(number); } // NOLINT(readability-identifier-naming)

    void println() { write('\r'); write('\n'); } // NOLINT(readability-identifier-naming)
    void println(const __FlashStringHelper* text) { print(text); println(); } // NOLINT(readability-identifier-naming)
    void println(const char* text) { print(text); println(); } // NOLINT(readability-identifier-naming)
    void println(int number) { print(number); println(); } // NOLINT(readability-identifier-naming)

    bool available() const; // NOLINT(readability-identifier-naming)
    char read(); // NOLINT(readability-identifier-naming)

    /* Waits until the queue has reached the wire. Only for the paths that are about to
       stop the processor - a restart otherwise cuts its own answer off mid-sentence. */
    void flush(); // NOLINT(readability-identifier-naming)
};

#endif // _ARDUINO_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
