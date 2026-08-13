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
/**     \file       WordclockSerial.h
 *      \brief      The port Serial is bound to: the UART plus an injected line source
 *
 *      \details    Communication reads its commands one character at a time out of
 *                  Serial, and every answer goes back out through the same object. A
 *                  second front end - a web socket - therefore does not need a protocol
 *                  of its own: it needs to reach this port. That is the same trick the
 *                  simulator plays, where Serial is bound to a shim that writes into two
 *                  text controls.
 *
 *                  Reading takes the UART first and the injected characters second, so a
 *                  console on the wire is never starved by a busy network client.
 *
 *                  Writing goes to the UART as it happens and is collected into a line at
 *                  the same time. A whole line is what a web socket wants - one frame per
 *                  answer rather than one per character - so the line sink is called from
 *                  println(), not from print().
 *
 *                  This header deliberately includes no Arduino header. Arduino.h next to
 *                  it includes the core's and then binds Serial to this class, so
 *                  including one from here would close that circle.
 *
******************************************************************************************************************************************************/
#ifndef _WORDCLOCK_SERIAL_H_
#define _WORDCLOCK_SERIAL_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

#include <atomic>
#include <stddef.h>
#include <stdint.h>

/* Only ever used as a pointer, so the declaration is enough - and it has to be made here
   rather than taken from a header, see the note about the include circle above. The core
   declares it the same way. */
class __FlashStringHelper;

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WordclockSerial configuration parameter */
/* Room for one answer. Longer output is flushed early rather than truncated, which costs
   a web socket client a split line and keeps the UART's output complete. */
#define WORDCLOCK_SERIAL_LINE_LENGTH                    160u

/* Room for a command in flight. A command is short and the firmware drains this every
   scheduler tick, so this only has to bridge one tick's worth of typing. Must be a power
   of two, which is what lets the indices wrap by masking. */
#define WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE             256u

/******************************************************************************************************************************************************
 *  C L A S S   W O R D C L O C K   S E R I A L
******************************************************************************************************************************************************/
class WordclockSerial
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* A plain function pointer rather than std::function: this is called from the
       firmware's task on every finished line, and nothing here allocates. */
    using LineSinkType = void (*)(const char*);

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr size_t LineLength{WORDCLOCK_SERIAL_LINE_LENGTH};
    static constexpr size_t InjectBufferSize{WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE};

    static_assert((InjectBufferSize & (InjectBufferSize - 1u)) == 0u,
                  "WordclockSerial: the inject buffer size has to be a power of two");

    char LineBuffer[LineLength]{};
    size_t LineUsed{0u};

    char InjectBuffer[InjectBufferSize]{};
    /* One producer - the web server's task - and one consumer, the firmware's tick. That
       is what makes two atomic indices enough and a lock unnecessary: each side writes
       only its own index and reads only the other's. esp_http_server serves its handlers
       from a single task, so the one-producer premise holds; a second injecting task
       would break it and would need a mutex here. */
    std::atomic<size_t> InjectHead{0u};
    std::atomic<size_t> InjectTail{0u};

    LineSinkType LineSink{nullptr};

    // functions
    static constexpr size_t nextIndex(size_t Index) { return (Index + 1u) & (InjectBufferSize - 1u); }

    bool isInjectEmpty() const { return InjectTail.load(std::memory_order_acquire) == InjectHead.load(std::memory_order_relaxed); }
    char readInjected();
    void appendToLine(char);
    void flushLine();

    WordclockSerial() { }
    ~WordclockSerial() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WordclockSerial& getInstance() {
        static WordclockSerial SingletonInstance;
        return SingletonInstance;
    }

    // methods
    void begin(unsigned long);

    /* Whoever wants the answers registers here. Nothing is registered by default, so the
       firmware runs with the UART alone until a front end asks for them. */
    void setLineSink(LineSinkType sLineSink) { LineSink = sLineSink; }

    /* Called from another task, one character stream at a time. Returns E_NOT_OK when the
       buffer is full, which drops the rest of that command rather than a random middle
       part of it - a half command is worse than a missing one, and the parser would
       answer with an error either way. */
    StdReturnType inject(const char*, size_t);

    // the interface Communication uses
    int available() const;
    int read();

    void println();
    void print(const char*);
    void print(int);
    void println(const char*);
    void println(int);
    void print(char);
    /* What F() produces on the target. The simulator's F() is a no-op, so the same
       firmware call lands on print(const char*) there. */
    void print(const __FlashStringHelper*);
    void println(const __FlashStringHelper*);
};

#endif // _WORDCLOCK_SERIAL_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
