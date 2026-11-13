// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The port Serial is bound to: the core's serial port plus an injected line source.
 *
 * Communication reads its commands one character at a time out of Serial, and every answer goes
 * back out through the same object. A second front end - a web socket - therefore does not need a
 * protocol of its own: it needs to reach this port. That is the same trick the simulator plays,
 * where Serial is bound to a shim that writes into two text controls.
 *
 * Reading takes the UART first and the injected characters second, so a console on the wire is
 * never starved by a busy network client.
 *
 * Writing goes to the core's port as it happens and is collected into a line at the same time. A
 * whole line is what a web socket wants - one frame per answer rather than one per character - so
 * the line sink is called when a line ends, from println(), or when it fills the buffer, not for
 * every character.
 *
 * This header deliberately includes no Arduino header. Arduino.h next to it includes the core's and
 * then binds Serial to this class, so including one from here would close that circle.
 */

#ifndef WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKSERIAL_H_
#define WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKSERIAL_H_

#include "StandardTypes.h"

#include <atomic>
#include <stddef.h>
#include <stdint.h>

/*
 * Only ever used as a pointer, so the declaration is enough - and it has to be made here
 * rather than taken from a header, see the note about the include circle above. In the
 * core's namespace and brought out with the core's own using-declaration, because this core
 * is ArduinoCore-API: a class of the same name declared at global scope is a second class
 * that conflicts with that using-declaration. GCC let it pass; clang refuses it.
 */
// NOLINTBEGIN(bugprone-reserved-identifier): the core's name
namespace arduino { class __FlashStringHelper; }
using arduino::__FlashStringHelper;
// NOLINTEND(bugprone-reserved-identifier)

/*!
 * \brief The room for one answer line.
 *
 * A line holds at most WORDCLOCK_SERIAL_LINE_LENGTH - 1 characters and its terminator. Longer
 * output is flushed early rather than truncated, which costs a web socket client a split line and
 * keeps the port's output complete. WebFrontend sizes the buffer it widens a line into by it.
 */
#define WORDCLOCK_SERIAL_LINE_LENGTH                    160u

/*!
 * \brief The room for injected characters not read yet; a power of two, and it holds one less.
 *
 * Room for a command in flight. A command is short, and Communication drains this every
 * COMMUNICATION_TASK_CYCLE ticks, up to the end of one command per call, so this only has to
 * bridge what arrives between two of those calls. Must be a power of two, which is what lets the
 * indices wrap by masking.
 */
#define WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE             256u

/*!
 * \brief The port the web front end reaches by this name.
 *
 * WebFrontend hands a browser's command to it with Inject(), as though it had been typed. The
 * backend's server installs the sink every finished answer line goes to with SetLineSink().
 *
 * On the RP2350 it is a class of its own, which `Serial` is bound to by this platform's Arduino.h,
 * with a lock-free buffer the web socket's event handler injects into. It stands in for the core's
 * own `Serial`, wordclock_platform::HardwarePort(): it reads that port first and the injected
 * characters second, and writes to that port as it goes while it collects each line for the sink.
 *
 * The writers keep one line buffer and take no lock, so they are for one context at a time:
 * loop()'s. WebInterface's callbacks, which run from lwIP's background interrupt, print nothing
 * themselves; a refused update is printed from the next tick.
 */
class WordclockSerial
{
  public:
    /*!
     * \brief A function a finished output line is handed to, terminated and without its line end.
     *
     * A plain function pointer rather than std::function: this is called on every finished line,
     * and nothing here allocates.
     */
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
     * One producer - the web socket's event handler - and one consumer, the firmware's tick.
     * That is what makes two atomic indices enough and a lock unnecessary: each side writes
     * only its own index and reads only the other's. The handler runs from lwIP's callbacks,
     * which arduino-pico runs in one context, the low-priority interrupt of the CYW43's async
     * context, so the one-producer premise holds; a second injecting context would break it
     * and would need a lock here.
     */
    std::atomic<size_t> inject_head_{0u};
    std::atomic<size_t> inject_tail_{0u};

    LineSinkType line_sink_{nullptr};

    static constexpr size_t NextIndex(size_t index) { return (index + 1u) & (kInjectBufferSize - 1u); }

    bool IsInjectEmpty() const { return inject_tail_.load(std::memory_order_acquire) == inject_head_.load(std::memory_order_relaxed); }
    char ReadInjected();
    void AppendToLine(char character);
    void FlushLine();

    WordclockSerial() { }
    ~WordclockSerial() = default;

  public:
    /*! \brief Returns the one port, which Serial names. */
    static WordclockSerial& GetInstance() {
        static WordclockSerial singleton_instance;
        return singleton_instance;
    }

    // Here and below, a NOLINT keeps the name Arduino's Serial gives it: the firmware core is
    // written against that API, and this class stands in for it.
    /*!
     * \brief Opens the core's own port at `baud_rate`, as Arduino's begin().
     *
     * \param[in] baud_rate The baud rate.
     */
    void begin(unsigned long baud_rate); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Installs the function every finished output line goes to.
     *
     * Whoever wants the answers registers here. Nothing is registered by default, so the firmware
     * runs with the core's port alone until a front end asks for them; WebInterface::Begin()
     * installs its own once its server runs. The sink is called from the context that printed the
     * line.
     *
     * \param[in] line_sink The function, or nullptr for none.
     */
    void SetLineSink(LineSinkType line_sink) { line_sink_ = line_sink; }

    /*!
     * \brief Hands `length` characters to the port's input, as though they had been typed.
     *
     * For the web socket's event handler, the one producer the buffer is built for; see the note
     * on the indices. A full buffer drops the remainder rather than overwriting what has not been
     * read yet: the reader is mid-command, and overwriting would splice two commands into one that
     * parses as neither.
     *
     * \param[in] characters The characters; they need not be terminated.
     * \param[in] length The number of characters.
     * \return kEOk, or kENotOk for a null `characters`, a `length` of 0 or a full buffer, which
     *         drops the rest of the command and keeps what went in.
     * \note What went in before the buffer filled stays without the end of its command, so the
     *       parser reads it together with what is injected next.
     */
    StdReturnType Inject(const char* characters, size_t length);

    // the interface Communication uses
    /*!
     * \brief Returns the characters waiting on the core's port, or 1 while only injected ones wait.
     */
    int available() const; // NOLINT(readability-identifier-naming)

    /*!
     * \brief Takes the next character, from the core's port before the injected ones.
     *
     * The core's port is served first so that a console on the wire keeps working while a network
     * client is busy - the wire is where one debugs a clock whose network is the problem.
     *
     * \return The character, as an unsigned char, or -1 when there is none.
     */
    int read(); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Ends the line: writes a carriage return and a line feed, and hands the line to the
     *        sink.
     *
     * An empty line is not handed over, because println() is also used to terminate output that
     * print() already ended with a newline - passing that on would send a blank frame after every
     * answer.
     */
    void println(); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Writes `text`, character by character, as print(char) does.
     *
     * \param[in] text The text, terminated, or nullptr, which writes nothing.
     */
    void print(const char* text); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Writes `number` in decimal, with a minus sign if it is negative.
     *
     * \param[in] number The number.
     */
    void print(int number); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Writes `text` and ends the line, as print() and println() do.
     *
     * \param[in] text The text, terminated, or nullptr, which ends the line alone.
     */
    void println(const char* text); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Writes `number` in decimal and ends the line, as print() and println() do.
     *
     * \param[in] number The number.
     */
    void println(int number); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Writes one character, to the core's port and into the line being collected.
     *
     * A newline is treated as the end of the line rather than as content, the same way the
     * simulator's shim does it - the firmware ends some of its output with print('\\n') instead of
     * println(), and a line sink that did not notice would run two answers together. A line that
     * outgrows the buffer is handed to the sink early rather than cut.
     *
     * \param[in] character The character.
     */
    void print(char character); // NOLINT(readability-identifier-naming)

    /*!
     * \copydoc print(const char*)
     *
     * The overload for what F() makes on the target. Flash is read through the same address space
     * here, so what F() wrapped is read as an ordinary string. The simulator's F() is a no-op, so
     * the same firmware call lands on print(const char*) there.
     */
    void print(const __FlashStringHelper* text); // NOLINT(readability-identifier-naming)

    /*!
     * \copydoc println(const char*)
     *
     * The overload for what F() makes on the target, which is read as an ordinary string.
     */
    void println(const __FlashStringHelper* text); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Flushes the core's port, as Arduino's flush().
     *
     * Only one caller so far, and it is the one that matters: the restart, which would otherwise
     * cut its own answer in half. Says nothing about the web socket, whose frames lwIP sends.
     *
     * \note What it waits for is the core's port's own: on the USB CDC port, arduino-pico's flush()
     *       hands what is buffered to the USB stack and returns, without waiting for the host to
     *       take it.
     */
    void flush(); // NOLINT(readability-identifier-naming)
};

#endif  // WORDCLOCK_PLATFORM_RP2350_INCLUDE_WORDCLOCKSERIAL_H_
