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
#define bitRead(Value, Bit)     (((Value) >> (Bit)) & 0x01u)
#endif

#define Serial                  SerialPort::getInstance()

/******************************************************************************************************************************************************
 *  G L O B A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
using byte = uint8_t;
using boolean = bool;

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N S
******************************************************************************************************************************************************/
/* Starts the millisecond time base and enables interrupts. Has to run before anything
   below is used, which is why main() calls it first. */
void initRuntime();

/* Milliseconds since initRuntime(). Wraps after 49 days; every reader here takes a
   difference, which stays correct across the wrap as long as it is read more often than
   that. */
uint32_t millis();

/******************************************************************************************************************************************************
 *  C L A S S   S E R I A L   P O R T
******************************************************************************************************************************************************/
class SerialPort
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr uint16_t TransmitBufferSize{ARDUINO_SERIAL_TRANSMIT_BUFFER_SIZE};
    static constexpr uint16_t ReceiveBufferSize{ARDUINO_SERIAL_RECEIVE_BUFFER_SIZE};

    /* Long enough for the widest number any overload below prints, plus sign and
       terminator. */
    static constexpr byte NumberStringLength{12u};

    SerialPort() { }
    ~SerialPort() { }

    void write(char);
    void printNumber(int32_t);
    void printUnsignedNumber(uint32_t);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static SerialPort& getInstance() {
        static SerialPort SingletonInstance;
        return SingletonInstance;
    }

    void begin(uint32_t Baudrate);

    /* The overload set is the simulator's, plus the flash-string one that F() needs.
       byte and the smaller integers reach printNumber() through the int overload by
       integral promotion, which is what makes a byte print as a number here and on the
       simulator alike rather than as a character. */
    void print(const __FlashStringHelper*);
    void print(const char*);
    void print(char Character) { write(Character); }
    void print(int Number) { printNumber(Number); }
    void print(unsigned int Number) { printUnsignedNumber(Number); }
    void print(long Number) { printNumber(Number); }
    void print(unsigned long Number) { printUnsignedNumber(Number); }

    void println() { write('\r'); write('\n'); }
    void println(const __FlashStringHelper* Text) { print(Text); println(); }
    void println(const char* Text) { print(Text); println(); }
    void println(int Number) { print(Number); println(); }

    bool available() const;
    char read();

    /* Waits until the queue has reached the wire. Only for the paths that are about to
       stop the processor - a restart otherwise cuts its own answer off mid-sentence. */
    void flush();
};

#endif // _ARDUINO_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
