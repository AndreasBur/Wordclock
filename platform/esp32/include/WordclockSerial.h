// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The port Serial is bound to: the UART plus an injected line source.
 *
 * Communication reads its commands one character at a time out of Serial, and every answer goes
 * back out through the same object. A second front end - a web socket - therefore does not need a
 * protocol of its own: it needs to reach this port. That is the same trick the simulator plays,
 * where Serial is bound to a shim that writes into two text controls.
 *
 * Reading takes the UART first and the injected characters second, so a console on the wire is
 * never starved by a busy network client.
 *
 * Writing goes to the UART as it happens and is collected into a line at the same time. A whole
 * line is what a web socket wants - one frame per answer rather than one per character - so the
 * line sink is called from println(), not from print().
 *
 * This header deliberately includes no Arduino header. Arduino.h next to it includes the core's and
 * then binds Serial to this class, so including one from here would close that circle.
 */

#ifndef _WORDCLOCK_SERIAL_H_
#define _WORDCLOCK_SERIAL_H_

#include "StandardTypes.h"

#include <atomic>
#include <stddef.h>
#include <stdint.h>

/*
 * Only ever used as a pointer, so the declaration is enough - and it has to be made here
 * rather than taken from a header, see the note about the include circle above. The core
 * declares it the same way.
 */
class __FlashStringHelper;

/* WordclockSerial configuration parameter */
// Room for one answer. Longer output is flushed early rather than truncated, which costs
// a web socket client a split line and keeps the UART's output complete.
#define WORDCLOCK_SERIAL_LINE_LENGTH                    160u

/*
 * Room for a command in flight. A command is short and the firmware drains this every
 * scheduler tick, so this only has to bridge one tick's worth of typing. Must be a power
 * of two, which is what lets the indices wrap by masking.
 */
#define WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE             256u

class WordclockSerial
{
  public:
    // A plain function pointer rather than std::function: this is called from the
    // firmware's task on every finished line, and nothing here allocates.
    using LineSinkType = void (*)(const char*);

  private:
    static constexpr size_t kLineLength{WORDCLOCK_SERIAL_LINE_LENGTH};
    static constexpr size_t kInjectBufferSize{WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE};

    static_assert((kInjectBufferSize & (kInjectBufferSize - 1u)) == 0u,
                  "WordclockSerial: the inject buffer size has to be a power of two");

    char line_buffer_[kLineLength]{};
    size_t line_used_{0u};

    char inject_buffer_[kInjectBufferSize]{};
    /*
     * One producer - the web server's task - and one consumer, the firmware's tick. That
     * is what makes two atomic indices enough and a lock unnecessary: each side writes
     * only its own index and reads only the other's. esp_http_server serves its handlers
     * from a single task, so the one-producer premise holds; a second injecting task
     * would break it and would need a mutex here.
     */
    std::atomic<size_t> inject_head_{0u};
    std::atomic<size_t> inject_tail_{0u};

    LineSinkType line_sink_{nullptr};

    // functions
    static constexpr size_t NextIndex(size_t index) { return (index + 1u) & (kInjectBufferSize - 1u); }

    bool IsInjectEmpty() const { return inject_tail_.load(std::memory_order_acquire) == inject_head_.load(std::memory_order_relaxed); }
    char ReadInjected();
    void AppendToLine(char);
    void FlushLine();

    WordclockSerial() { }
    ~WordclockSerial() { }

  public:
    static WordclockSerial& GetInstance() {
        static WordclockSerial singleton_instance;
        return singleton_instance;
    }

    // methods
    // Here and below, a NOLINT keeps the name Arduino's Serial gives it: the firmware core is
    // written against that API, and this class stands in for it.
    void begin(unsigned long); // NOLINT(readability-identifier-naming)

    // Whoever wants the answers registers here. Nothing is registered by default, so the
    // firmware runs with the UART alone until a front end asks for them.
    void SetLineSink(LineSinkType line_sink) { line_sink_ = line_sink; }

    /*
     * Called from another task, one character stream at a time. Returns kENotOk when the
     * buffer is full, which drops the rest of that command rather than a random middle
     * part of it - a half command is worse than a missing one, and the parser would
     * answer with an error either way.
     */
    StdReturnType Inject(const char*, size_t);

    // the interface Communication uses
    int available() const; // NOLINT(readability-identifier-naming)
    int read(); // NOLINT(readability-identifier-naming)

    void println(); // NOLINT(readability-identifier-naming)
    void print(const char*); // NOLINT(readability-identifier-naming)
    void print(int); // NOLINT(readability-identifier-naming)
    void println(const char*); // NOLINT(readability-identifier-naming)
    void println(int); // NOLINT(readability-identifier-naming)
    void print(char); // NOLINT(readability-identifier-naming)
    // What F() produces on the target. The simulator's F() is a no-op, so the same
    // firmware call lands on print(const char*) there.
    void print(const __FlashStringHelper*); // NOLINT(readability-identifier-naming)
    void println(const __FlashStringHelper*); // NOLINT(readability-identifier-naming)

    /*
     * Waits for the UART to have sent what it was given. Only one caller so far, and it is
     * the one that matters: the restart, which would otherwise cut its own answer in half.
     * Says nothing about the web socket, whose frames leave from the server's task.
     */
    void flush(); // NOLINT(readability-identifier-naming)
};

#endif // _WORDCLOCK_SERIAL_H_
